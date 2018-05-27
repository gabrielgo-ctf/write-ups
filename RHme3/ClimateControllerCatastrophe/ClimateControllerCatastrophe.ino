#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char lenRec = 0;
unsigned char rxBuf[8];
char msgString[128];

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10

void setup()
{
  Serial.begin(115200);
  
  if(CAN0.begin(MCP_ANY, CAN_100KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);
  pinMode(CAN0_INT, INPUT);
}


void hexdump(byte* data, int len)
{
  for (int i = 0; i < len; i++)
  {
    sprintf(msgString, "%02X", data[i]);
    Serial.print(msgString);    
  }
  Serial.println();
}

void printPacket(int id, byte* data, int len)
{
  sprintf(msgString, " - ID: %04X   LEN: %01X   DATA: ", id, len);
  Serial.print(msgString);    
  hexdump(data, len);
}


byte sendCANPacket(int id, byte len, byte* data)
{
  byte sndStat = CAN0.sendMsgBuf(id, 0, len, data);
  return sndStat;
}

void sendCANData(int id, int len, byte* data)
{
  Serial.println("Sending to CAN...");
  byte CANpacket[8];
  int c = 0;
  int lenPacket = 0;
  int counter = 0;
  while (c < len)
  {
    if (c == 0)
    {
      CANpacket[0] = 0x10 | ((len & 0xFFF) >> 8);
      CANpacket[1] = len & 0xFF;
      lenPacket = (len > 6 ? 6 : len);
      memcpy(CANpacket + 2, data, lenPacket);
      c += lenPacket;
      lenPacket += 2;
    }
    else
    {
      CANpacket[0] = 0x20 | (counter & 0x0F);
      lenPacket = ((len - c) > 7 ? 7 : (len - c));
      memcpy(CANpacket + 1, data + c, lenPacket);
      c += lenPacket;
      lenPacket += 1;
    }
    printPacket(id, CANpacket, lenPacket);
    sendCANPacket(id, lenPacket, CANpacket);
    counter++;
  }
  Serial.println("Sent!");
}


void loop()
{
  //IDID LLLL DATA
  if (Serial.available() >= 4) {
    //Read ID
    byte id1 = Serial.read();
    byte id2 = Serial.read();
    int id = (id1 << 8) + id2;

    //Read data len
    byte len1 = Serial.read();
    byte len2 = Serial.read();
    int len = (len1 << 8) + len2;

    //Read data
    byte data[0x900];
    int i = 0;
    while(i < len) 
    {
      if (Serial.available() > 0) {
        data[i++] = Serial.read();
      }
    }

    Serial.println("Serial message received:");
    hexdump(data, len);
    
    sendCANData(id, len, data);
  }

  if(!digitalRead(CAN0_INT))
  {    
    CAN0.readMsgBuf(&rxId, &lenRec, rxBuf);
    
    if((rxId & 0x80000000) == 0x80000000)
      sprintf(msgString, "CAN msg received -> ID: %.3lX   LEN: %1X   DATA: ", (rxId & 0x1FFFFFFF), lenRec);
    else
      sprintf(msgString, "CAN msg received -> ID: %.3lX   LEN: %1X   DATA: ", rxId, lenRec);
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
    } else {
      hexdump(rxBuf, lenRec);
    }
  }
}
