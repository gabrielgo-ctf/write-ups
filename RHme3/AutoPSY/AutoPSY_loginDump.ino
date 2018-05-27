#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[80];

//From 0000 to 8000 ID 7E0
//From 8000 to C000 ID 7D3
unsigned int initialAddress = 0x8000;
unsigned int lenght = 0x4000;
unsigned int pageSize = 0x400;
byte data[0x400];

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

  pinMode(RESET, OUTPUT);
  pinMode(CS, INPUT);
  pinMode(SI, INPUT);
  pinMode(CLK, INPUT);
  digitalWrite(RESET, HIGH);
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

int genPass(int seed)
{
    byte seedPass[] = { 0x45, 0x71, 0x3D, 0x8B, 0x4F, (byte)(seed >> 8), (byte)(seed) };
    byte r24 = 0, r25 = 0;
    for (int i = 0; i < 7; i++)
    {
        byte r22 = seedPass[i];
        r24 ^= r22;
        r22 = r24;
        r22 = (r22 << 4) | (r22 >> 4);
        r22 ^= r24;
        byte r0 = r22;
        r22 >>= 1;
        r22 >>= 1;
        r22 ^= r0;
        r0 = r22;
        r22 >>= 1;
        r22 ^= r0;
        r22 &= 0x07;
        r0 = r24;
        r24 = r25;
        int carry1 = r22 & 1;
        int carry2 = r0 & 1;
        r22 >>= 1;
        r0 = (carry1 << 7) | (r0 >> 1);
        r22 = (carry2 << 7) | (r22 >> 1);
        r25 = r0;
        r24 ^= r22;
        carry2 = r0 & 1;
        r0 >>= 1;
        r22 = (carry2 << 7) | (r22 >> 1);
        r25 ^= r0;
        r24 ^= r22;
    }
    if ((byte)(seed >> 8) == (byte)seed) return (r25 << 8) + r25;
    else return (r25 << 8) + (r25 ^ 1);
}

//int ID = 0x7E0;
int ID = 0x7D3;
void loop()
{
  int c = 0;
  while (c < 1) //bruteforce key (no needed because it is generated correctly by genPass function)
  {
    byte SessionControl[8] = {0x02, 0x10, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
    CAN0.sendMsgBuf(ID, 8, SessionControl);
    Serial.println("Enviado Session Control");
    readPacket(ID + 8, true);
  
    byte RequestSeed[8] = {0x02, 0x27, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
    CAN0.sendMsgBuf(ID, 8, RequestSeed);
    Serial.println("Enviado Request Seed");
    readPacket(ID + 8, true);
    sprintf(msgString, "SEED = %02X%02X", rxBuf[3], rxBuf[4]);
    Serial.println(msgString);

    int key = genPass((rxBuf[3] << 8) + rxBuf[4]);
    /*
    int key = 0x2345;
    switch((rxBuf[3] << 8) + rxBuf[4])
    {
      case 0x103C: key = 0x0234; break;
      case 0x28CD: key = 0x4632; break;
      case 0x1C03: key = 0x1234; break;
      case 0x11C2: key = 0x1235; break;
      case 0xC342: key = 0x1234; break;
    }*/
    byte SendKey[8] = {0x04, 0x27, 0x02, (byte)(key>>8), (byte)key, 0x00, 0x00, 0x00};
    CAN0.sendMsgBuf(ID, 8, SendKey);
    sprintf(msgString, " Enviado Send Key");
    Serial.println(msgString);
    readPacket(ID + 8, true);
    if (rxBuf[3] != 0x35)
    {
      sprintf(msgString, "KEY OK = %04X", key);
      Serial.println(msgString);
      break;
    }
  }
  
  for (int address = initialAddress; address < initialAddress + lenght; address+=pageSize)
  {
    Serial.println("Enviado 35....");
    byte cmd35[8] = {0x07, 0x35, 0x00, 0x22, (byte)(address >> 8), (byte)address, (byte)(pageSize >> 8), (byte)pageSize};
    CAN0.sendMsgBuf(ID, 8, cmd35);
    Serial.println("Enviado 35");
    readPacket(ID + 8, true);
    if (rxBuf[1] != 0x75)
    {
      while(true) delay(100);
    }
    int lenDataTransfer = rxBuf[3];
  
    int idxData = 0;
    for (int i = 0; i < (pageSize + lenDataTransfer - 1) / lenDataTransfer; i++)
    {
      byte cmd36[8] = {0x02, 0x36, (byte)(i + 1), 0x00, 0x00, 0x00, 0x00, 0x00};
      CAN0.sendMsgBuf(ID, 8, cmd36);
      Serial.println("Enviado 36");
      int bytestoread = (pageSize - idxData > lenDataTransfer ? lenDataTransfer : pageSize - idxData);
      int bytescopied = 0;
      for (int j = 0; j < bytestoread; ) 
      {
        readPacket(ID + 8, j==0);
        int offset = 1;
        if (j == 0) offset = 4;
        for (int k = 0; k < (8 - offset) && (bytescopied++ < bytestoread); k++) data[idxData++] = rxBuf[offset + k];
        j += 8 - offset;
      }
    }
    Hexdump(data, pageSize);
  
    byte cmd37[8] = {0x01, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    CAN0.sendMsgBuf(ID, 8, cmd37);
    Serial.println("Enviado 37");
    readPacket(ID + 8, true);
  }
  
  while(true) delay(100);
}
