#include <stdlib.h>
#include <XBee.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define XBEE_UART1
#define START_MSG 26
#define MAX_TIMEOUT 1500
#define  MAX_DATA_SIZE 96

// variables gÃ©nÃ©rales
XBee      xbee = XBee();
int       msg_id;
int       mode; // 0 = envoi; 1 = reception
int       cmp; // permet de detecter un timeout
char      data[MAX_DATA_SIZE];
char      voisin[6] = "3436E5";
uint32_t  voisin_add[3] = {0x4086D834, 0x4086D836, 0x406FB3E5};
char      src[2] = "22";
char      dst[2] = "E9"; // cible de l'envoi
char      msg[MAX_DATA_SIZE - START_MSG] = "hello from 22";

// variables pour l'affichage LCD
LiquidCrystal_I2C lcd(0x20, 16, 2);

// variables pour l'envoi
uint8_t spayload[MAX_DATA_SIZE];

// variables pour la rÃ©ception
uint8_t*    rpayload;
uint8_t     rpayload_len;
Rx16Response  rx16 = Rx16Response();
Rx64Response  rx64 = Rx64Response();

void  setup() {
  Serial.begin(38400);
  Serial.println("Setup Projet Delbouys & Gnebehi");
  mode = 0;
  msg_id = 0;
  
  lcd.begin();
  lcd.backlight();
  lcd.home();
  printLCD(0, "Setup Projet", true);
  printLCD(1, "Delbouys-Gnebehi", false);
  
  #ifdef XBEE_UART1 
    xbee.setSerial(Serial1);
    Serial1.begin(38400);
  #else
    xbee.begin(38400);
  #endif
}

void  loop() {
  if(mode == 0) {
    // ENVOI
    delay(5000);
    cmp = 0;
    createRREQ();
    Serial.print("Send RREQ... ");
    displayData();
    printLCD(0, "Send RREQ...", true);
    if(sendData(0x00000000, 0x0000FFFF) == 1) {
      mode = 1;
      msg_id++;
      if(msg_id > 9)
        msg_id = 0;
    }
  }
  else if(cmp > MAX_TIMEOUT) {
    // TIMEOUT
    mode = 0;
  }
  else {
    // RECEPTION
    delay(10);
    cmp++;
    xbee.readPacket();
    if(xbee.getResponse().isAvailable()) {
      //Serial.println("Something received...");
      if(xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
      }
      if(xbee.getResponse().getApiId() == RX_16_RESPONSE || xbee.getResponse().getApiId() == RX_64_RESPONSE) {
        if(xbee.getResponse().getApiId() == RX_16_RESPONSE) {
          xbee.getResponse().getRx16Response(rx16);
          rpayload = rx16.getData();
          rpayload_len = rx64.getDataLength();
        }
        else {
          xbee.getResponse().getRx64Response(rx64);
          rpayload = rx64.getData();
          rpayload_len = rx64.getDataLength();
        }
        
        // rpayload rempli, on peut commencer le traitement
        if(isWhitelisted()) {
          Serial.print("Message recu d'un voisin : ");
          displayRPayload();
          if((char)rpayload[0] == 'Q') {
            // message de type RREQ
            // si on est deja dans la route on ne fait rien
            if(alreadyInList() == false) {
              // si on est la cible, on renvoie un RREP
              if((char)rpayload[2] == src[0] && (char)rpayload[3] == src[1]) {
                createRREP();
                Serial.print("Send RREP... ");
                displayData();
                printLCD(0, "Send RREP...", true);
                sendToPrevious();
              }
              else if ((char)rpayload[4] != src[0] || (char)rpayload[5] != src[1]) {
                // sinon on s'ajoute dans la liste et on fait suivre si on a pas atteind la taille max de la route
                completeRREQ();
                if(data[START_MSG] == '\0') {
                  Serial.print("Forward RREQ... ");
                  displayData();
                  printLCD(0, "Forward RREQ...", true);
                  sendData(0x00000000, 0x0000FFFF);
                }
              }
            }
          }
          else if ((char)rpayload[0] == 'P') {
            // message de type RREP
            // si on est la source, on renvoie un DATA
            if((char)rpayload[4] == src[0] && (char)rpayload[5] == src[1]) {
              createDATA();
              Serial.print("Send DATA... ");
              displayData();
              printLCD(0, "Send DATA...", true);
              sendToNext();
              mode = 0;
            }
            else if(alreadyInList() == true){
              // sinon on continue a remonter la route
              copyRpayloadToData();
              Serial.print("Forward RREP...");
              displayData();
              printLCD(0, "Forward RREP...", true);
              sendToPrevious();
            }
          }
          else if ((char)rpayload[0] == 'M') {
            // message de type DATA
            // si on est la cible, on affiche DATA recu
            if((char)rpayload[2] == src[0] && (char)rpayload[3] == src[1]) {
              char  txtToDisplay[16];
              int   i_txtToDisplay = 0;
              Serial.print("Receive DATA... ");
              displayData();
              printLCD(0, "Receive DATA...", true);
              for(int i = START_MSG; i < rpayload_len; i++) {
                Serial.print((char)rpayload[i]);
                if(i_txtToDisplay < 15) {
                  txtToDisplay[i_txtToDisplay] = (char)rpayload[i];
                  i_txtToDisplay++;
                }
                else {
                  txtToDisplay[13] = '.';
                  txtToDisplay[14] = '.';
                  txtToDisplay[15] = '.';
                }
              }
              printLCD(1, txtToDisplay, false);
              Serial.println("");
            }
            else if(alreadyInList() == true){
              // sinon on continue a suivre la route
              copyRpayloadToData();
              Serial.print("Forward DATA...");
              displayData();
              printLCD(0, "Forward DATA...", true);
              sendToNext();
            }
          }
        }
      }
    }
  }
}

void  copyRpayloadToData() {
  int i = 0;
  
  while(i < rpayload_len) {
    data[i] = (char)rpayload[i];
    i++;
  }
}

void  displayData() {
  for(int i = 0; i < MAX_DATA_SIZE; i++) {
    Serial.print((char)data[i]);
  }
  Serial.println("");
}

void  displayRPayload() {
  for(int i = 0; i < rpayload_len; i++) {
    Serial.print((char)rpayload[i]);
  }
  Serial.println("");
}

// fonction d'affichage sur l'Ã©cran LCD
void  printLCD(int line, const char* txt, boolean clear) {
  if(clear)
    lcd.clear();
  lcd.setCursor(0, line);
  lcd.print(txt);
}

// rempli de '\0' la variable data dans le but de la vider
void  emptyData() {
  int i = 0;
  
  while(i < MAX_DATA_SIZE) {
    data[i] = '\0';
    i++;
  }
}

// verifie si le message reÃ§u vient bien d'un voisin
boolean isWhitelisted() {
  int       i;
  char      remoteAdd[10];
  XBeeAddress64 &add_from = rx64.getRemoteAddress64();
  
  sprintf(remoteAdd, "0x%08lx", add_from.getLsb());
  i = 2;
  while(i < 10) {
    if(remoteAdd[i] >= 'a' && remoteAdd[i] <= 'z')
      remoteAdd[i] = remoteAdd[i] - 32;
    i++;
  }
  i = 0;
  while(i < 6) {
    if(remoteAdd[8] == voisin[i] && remoteAdd[9] == voisin[i+1])
      return true;
    i = i + 2;
  }
  return false;
}

// verifie si on est deja present dans la route
boolean alreadyInList() {
  int i = 6;
  while(i < rpayload_len) {
    if(src[0] == (char)rpayload[i] && src[1] == (char)rpayload[i+1])
      return true;
    i = i + 2;
  }
  return false;
}

// creer un message de type RREQ
void  createRREQ() {
  emptyData();
  data[0] = 'Q';
  data[1] = '0' + msg_id;
  data[2] = dst[0];
  data[3] = dst[1];
  data[4] = src[0];
  data[5] = src[1];
}

// s'ajouter a la route du RREQ
void  completeRREQ() {
  int i = 0;
  
  emptyData();
  while(i < rpayload_len && (char)rpayload[i] != '\0') {
    data[i] = (char)rpayload[i];
    i++;
  }
  data[i] = src[0];
  data[i+1] = src[1];
}

// creer un message de type RREP
void  createRREP() {
  completeRREQ();
  data[0] = 'P';
}

// creer un message de type DATA
void  createDATA() {
  int i = 0;
  int j = 0;
  
  emptyData();
  while(i < rpayload_len) {
    data[i] = (char)rpayload[i];
    i++;
  }
  data[0] = 'M';
  i = START_MSG;
  while(msg[j] != '\0') {
    data[i] = msg[j];
    i++;
    j++;
  }
}

// envoi direct au precedent de la route
void  sendToPrevious() {
  int i = 6;

  while(src[0] != data[i] || src[1] != data[i+1])
    i = i + 2;
  directSend(data[i-2], data[i-1]);
}

// envoi direct au suivant de la route
void  sendToNext() {
  int i = 4;

  while(src[0] != data[i] || src[1] != data[i+1])
    i = i + 2;
  directSend(data[i+2], data[i+3]);
}

// envoie a un voisin direct
void  directSend(char a, char b) {
  if(a == voisin[0] && b == voisin[1])
    sendData(0x0013A200, voisin_add[0]);
  else if(a == voisin[2] && b == voisin[3])
    sendData(0x0013A200, voisin_add[1]);
  else if(a == voisin[4] && b == voisin[5])
    sendData(0x0013A200, voisin_add[2]);
}

// envoie data sur le broadcast ou sur une addresse precise
int   sendData(uint32_t sh, uint32_t sl) {
  XBeeAddress64   addr64 = XBeeAddress64(sh, sl);
  Tx64Request     tx;
  TxStatusResponse  txStatus;
  
  for(int i=0; i < sizeof(data); i++) {
    spayload[i] = (uint8_t)data[i];
    tx = Tx64Request(addr64, (uint8_t*)data, sizeof(data));
    xbee.send(tx);
    txStatus = TxStatusResponse();
    xbee.readPacket();
    if(xbee.getResponse().isAvailable()) {
      if(xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
        xbee.getResponse().getTxStatusResponse(txStatus);
        if(txStatus.isSuccess()) {
          Serial.println("Send successful !");
          printLCD(1, "SUCCESS !", false);
          return 1;
        }
        else {
          Serial.println("Send failed ...");
          printLCD(1, "FAIL ...", false);
          return 0;
        }
      }
    }
  }
  
  return 0;
}
