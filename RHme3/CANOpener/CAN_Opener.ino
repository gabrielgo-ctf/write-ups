#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

void setup()
{
  Serial.begin(115200);
  
  if(CAN0.begin(MCP_ANY, CAN_50KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);
  pinMode(CAN0_INT, INPUT);
}

void loop()
{
  if(!digitalRead(CAN0_INT))                  // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
      }
    }
  
    if (rxId== 0x332)
    {
      byte data[8] = {0x75,0x6E,0x6C,0x6F,0x63,0x6B,0x00,0x00 }; //unlock
      byte sndStat = CAN0.sendMsgBuf(0x332, 0, 8, data);
      if(sndStat == CAN_OK){
        sprintf(msgString, "Message Sent Successfully!(%02X)", sndStat);
      } else {
        sprintf(msgString, "Error Sending Message... (%02X)", sndStat);
      }
      Serial.println(msgString);
    }
  
    Serial.println();
  }
}