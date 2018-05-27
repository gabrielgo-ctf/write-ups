#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];

#define CAN0_INT 2
MCP_CAN CAN0(10);

#define CAN1_INT 4
MCP_CAN CAN1(9);

void setup()
{
  Serial.begin(115200);
  
  if(CAN1.begin(MCP_ANY, CAN_50KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("CAN1 Inicializado!");
  else
    Serial.println("Error inicializando CAN1...");
  CAN1.setMode(MCP_NORMAL);
  pinMode(CAN1_INT, INPUT);
  
  if(CAN0.begin(MCP_ANY, CAN_50KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("CAN0 Inicializado!");
  else
    Serial.println("Error inicializando CAN0...");
  CAN0.setMode(MCP_NORMAL);
  pinMode(CAN0_INT, INPUT);
}

byte data23[5] = {0x00, 0x58, 0x08, 0x0C, 0x10};
byte data19A[2] = { 0x71, 0x10 };
byte data10C[8] = { 0x00, 0x4E, 0x01, 0x1D, 0x00, 0x4D, 0x00, 0x4F};
byte data1BF[6] = { 0x00, 0x10, 0x00, 0x0F, 0x00, 0x11};
byte data202[5] = { 0x00, 0x4A, 0x00, 0x4A, 0x52};
byte data12[2] = { 0x06, 0x22 };

void loop()
{ 
  while(true)
  {
    CAN0.sendMsgBuf(0x19A, 2, data19A);
    CAN0.sendMsgBuf(0x23, 5, data23);
    //CAN0.sendMsgBuf(0x202, 5, data202);
    //CAN1.sendMsgBuf(0x10C, 8, data10C);
    //CAN1.sendMsgBuf(0x1BF, 6, data1BF);
    //CAN1.sendMsgBuf(0x12, 2, data12);
    delay(100);
  }
  
  if(!digitalRead(CAN0_INT))
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
    sprintf(msgString, "CAN0: %.3lX %1d ", rxId, len);
    Serial.print(msgString);
    for(byte i = 0; i<len; i++){
        sprintf(msgString, "%02X", rxBuf[i]);
        Serial.print(msgString);
    }
    Serial.println(); 
    //CAN1.sendMsgBuf(rxId, len, rxBuf);
  }
  
  if(!digitalRead(CAN1_INT))
  {
    CAN1.readMsgBuf(&rxId, &len, rxBuf);
    sprintf(msgString, "CAN1: %.3lX %1d ", rxId, len);
    Serial.print(msgString);
    for(byte i = 0; i<len; i++){
        sprintf(msgString, "%02X", rxBuf[i]);
        Serial.print(msgString);
    }
    Serial.println(); 
    //CAN0.sendMsgBuf(rxId, len, rxBuf);
  }
}
