/*
 * serial rotor DCU1 protocol to run @38k4/8N1
 * Port1: PC 38400 8N1  RX=0,  TX=1
 */
int packetSizeSlow=45;
int packetSizeFast=16;

unsigned int nb; 
char rxbuffer[16];    // rx buffer
unsigned int nb1;
char rx1buffer[16];    // rx1 buffer

typedef enum {RxStart, RxChar, RxEOM} RXstate;
RXstate rxState;

unsigned int snb=0; // write pointer 
unsigned int rnb=0; // read pointer
#define SMSG_LEN 128
char serialMsg[SMSG_LEN];
bool infoReq=0;
String infoToTx;
int connSetValue;

#define MAX_SMSG 5
class RxBuffer {
public:
  unsigned int sbi=0; // write pointer 
  unsigned int msgi=0; // msg pointer
  char serialMsg[MAX_SMSG][SMSG_LEN];
  char rxbuffer[16];    // rx buffer
  RXstate rxState=RxStart;
   
  bool putRxData(char data);
  bool putRxData(char *data, int len);
  char *getRxData();
};

bool RxBuffer::putRxData(char *dat, int len) {
int numr=0;  
   if (len<0 || len>SMSG_LEN) 
      return;
   for (int i=0;i<len;i++) {
       numr+=putRxData(dat[i])?1:0;
   }
   //Serial.print("RXB:");Serial.print(numr);Serial.print(",");Serial.print(len);
   return numr>0;
}
bool RxBuffer::putRxData(char dat) {
    if (0>=SMSG_LEN-sbi) {
      snb=0;
      rxState=RxStart; // no real ring buffer
    }
    if (dat==';') {
      if (rxState==RxChar) {
        rxState=RxEOM;
      }
    }
    else {
      rxState=RxChar;
    }
    if (msgi>=MAX_SMSG)
       return false;
    serialMsg[msgi][sbi]=dat;
    sbi++;
    if (rxState==RxEOM) {
      rxState=RxStart;
      serialMsg[msgi][sbi]=0;
      sbi=0;
      msgi++;
      return true;
    }
    return false;
}

char* RxBuffer::getRxData() {
   if (msgi<=0) 
     return NULL;
   char *m=  serialMsg[msgi-1];
   msgi--;
   return m;
}

RxBuffer pcCom;

void connect_setup() {
  // initialize serial ports:
  Serial.begin(38400, SERIAL_8N1);
  Serial.setTimeout(10);
  infoReq=0;
  rxState=RxStart;
  connMsgReceived=false;
  isConnected=false;
}

void serialEvent() {
  nb1=0;
  // get requests from PC 
  while  (Serial.available()) {
      nb1=Serial.readBytes(rx1buffer, packetSizeFast);
      if (pcCom.putRxData(rx1buffer, nb1)) {
        char*msg=pcCom.getRxData();
        if (msg!=NULL) {
          MsgExtract(msg);
          isConnected=true;
        }
      }
  }
}


bool connect_loop() {
  // only TX handled in here
  if (infoReq) {
    infoReq=false;
    Serial.write(infoToTx.c_str());
  }
  return connMsgReceived;
}

int connect_value() {
  connMsgReceived=false;
  return connSetValue;
}
void ConnMsgSend(char *msg, int value) {
  String sv=String(value);
  infoToTx=String(msg);
  infoToTx.concat(sv);
  infoToTx.concat(";");
  infoReq=true;
}

// mimics DCU1 protocol
void MsgExtract(char *msg) {
  if (msg[0]==';') {
    // stop moving now
    demo=false;
  }
  else if (strncmp(msg,"DG",2)==0) {
    sdebug=(msg[2]=='1');
  }
  else if (strncmp(msg,"AM;",3)==0) {
    // start, ignored now
  }
  else if (strncmp(msg,"AI1;",4)==0) {
    // get info 
    ConnMsgSend(";",currdeg);
  }
  else if (strncmp(msg,"AP",2)==0) {
     // set new value
     connSetValue=atoi((const char*)msg+2);
     connMsgReceived=true;
  }  
}
