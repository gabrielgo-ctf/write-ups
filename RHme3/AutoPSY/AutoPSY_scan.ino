#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[80];

#define CAN0_INT 2
MCP_CAN CAN0(10);

#define CAN1_INT 4
MCP_CAN CAN1(9);

void setup()
{
  Serial.begin(115200);
  
  if(CAN0.begin(MCP_ANY, CAN_50KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("CAN0 Inicializado!");
  else
    Serial.println("Error inicializando CAN0...");
  CAN0.setMode(MCP_NORMAL);
  pinMode(CAN0_INT, INPUT);
  
  if(CAN1.begin(MCP_ANY, CAN_50KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("CAN1 Inicializado!");
  else
    Serial.println("Error inicializando CAN1...");
  CAN1.setMode(MCP_NORMAL);
  pinMode(CAN1_INT, INPUT);
}

void Hexdump(byte* data, int len)
{
  for (int i=0; i< len; i++)
  {
      sprintf(msgString, "%02X ", data[i]);
      Serial.print(msgString);
      if (((i + 1) % 0x10) == 0) Serial.println();
  }
}

void readPacket(int id, bool log)
{
  bool received = false;
  while(!received)
  {
    if(!digitalRead(CAN0_INT))
    {
      CAN0.readMsgBuf(&rxId, &len, rxBuf);
      if (rxId == id)
      {    
        if (log)
        {
          sprintf(msgString, "ID: 0x%.3lX  DLC: %1d  Data:", rxId, len);
          Serial.print(msgString);
          for(byte i = 0; i<8; i++){
            sprintf(msgString, " %.2X", rxBuf[i]);
            Serial.print(msgString);
          }
          Serial.println();
        }
        else
        {
            //sprintf(msgString, " %02X", rxBuf[0]);
            //Serial.print(msgString);
        }
        received = true;
      }
    }  
  }
}

int scanCMDs(int idscan)
{
  for (int i = 0; i < 0x100; i++)
  {
    byte cmd[8] = {0x02, (byte)i, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    CAN0.sendMsgBuf(idscan, 8, cmd);
    readPacket(idscan + 8, true);
  } 
}

void loop()
{
  Serial.println("Start scanning...");
  scanCMDs(0x7E0);
  //scanCMDs(0x7D3);
  //scanCMDs(0x7E5);
  while(true) delay(100);
}

