#define CLK A1
#define ENABLE A2
#define SCANIN A3
#define SCANOUT A4
#define RESET A5

#define frecuency 1000  //Hz
#define usDelay 1000000/frecuency/2 - 8

char msgString[128];
unsigned long timeLastChangeCLK;

void setup()
{
  Serial.begin(115200);
  pinMode(CLK, OUTPUT);
  pinMode(ENABLE, OUTPUT);
  pinMode(SCANIN, OUTPUT);
  pinMode(SCANOUT, INPUT);

  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, HIGH);

  timeLastChangeCLK = micros();
  Serial.println("Inicializado!");
}

void reset()
{
  digitalWrite(RESET, LOW);
  delay(5);
  digitalWrite(RESET, HIGH);
}

void setPIN(int pin)
{
  digitalWrite(pin, HIGH);
}

void clearPIN(int pin)
{
  digitalWrite(pin, LOW);
}

void writePIN(int pin, int value)
{
  if (value == 0)
  {
    clearPIN(pin);
  }
  else if (value == 1)
  {
    setPIN(pin);
  }
}

int readPIN(int pin)
{
  if (digitalRead(pin) == HIGH)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

int chain_sendValue(int valueENABLE, int valueSCANIN)
{
  //Wtite ENABLE & SCANIN
  writePIN(ENABLE, valueENABLE);
  writePIN(SCANIN, valueSCANIN);

  //Check that CLK have been 0, at least, the minimun time required
  unsigned long now = micros();
  if (now < timeLastChangeCLK + usDelay)
  {
    delayMicroseconds(timeLastChangeCLK + usDelay - now);
  }

  //set CLK
  setPIN(CLK);
  //Wait
  delayMicroseconds(usDelay);
  //clear TCK
  clearPIN(CLK);

  //Read the SCANOUT
  int scanout = readPIN(SCANOUT);

  //Store the time to check it when we send next bit
  timeLastChangeCLK = micros();

  return scanout;
}

void chain_ShiftIn(int chainSize, byte* scanIn)
{
  for (int i = chainSize - 1; i >= 0; i--)
  {
    chain_sendValue(1, scanIn[i]);
  }
}

void chain_Capture()
{
  chain_sendValue(0, 0);
}

void chain_ShiftOut(int chainSize, byte* scanIn, byte* scanOut)
{
  for (int i = chainSize - 1; i >= 0; i--)
  {
    scanOut[i] = chain_sendValue(1, scanIn[i]);
  }
}

int detectChainLen(int maxLen)
{
  chain_sendValue(1, 1);
  for (int i = 1; i <=  maxLen; i++)
  {
    int scanOut = chain_sendValue(1, 0);
    if (scanOut != 0)
      return i;
  }
  return 0;
}

void ByteArrayPrint(int len, byte* d, bool writeNL)
{
  for (int i = 0; i < len; i++)
  {
    sprintf(msgString, "%02X", d[i]);
    Serial.print(msgString);
  }
  if (writeNL) Serial.println();
}

void printInOut(int len, byte* in, byte* out)
{
  byte bytes[len / 8];
  bit2byte(len, in, bytes);
  ByteArrayPrint(len / 8, bytes, false);
  Serial.print(" -> ");
  bit2byte(len, out, bytes);
  ByteArrayPrint(len / 8, bytes, true);
}

void bit2byte(int len, byte* in, byte* out)
{
  memset(out, 0, len / 8);
  for (int i = 0; i < len / 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      out[i] |= in[8 * i + j] << (7 - j);
    }
  }
}

byte password[20];
const int chainLen = 512;
byte shifInValues[chainLen];
byte shifOutValues[chainLen];
bool logEnabled = false;
int offset = 0;

void loop()
{
  if (Serial.available() > 0)
  {
    byte val = Serial.read();
    switch (val)
    {  
      case 0x00:  //RESET
        {
          if (logEnabled) Serial.println("Reset...");
          reset();
          delay(2000);
        }
        break;     
      case 0x01:  //GET CHALLENGE
        {
          //create shiftIn values
          memset(shifInValues, 0, chainLen);
          shifInValues[157] = 1; //this bit enable the challenge value

          //Shift In, Capture & Shift Out
          chain_ShiftIn(chainLen, shifInValues);
          chain_Capture();
          chain_ShiftOut(128, shifInValues, shifOutValues);
          
          byte out[16];
          bit2byte(128, shifOutValues, out);
          ByteArrayPrint(16, out, false);
        }
        break;
      case 0x02:  //SEND PASSWORD
        {
          int i = 0;
          while (i < 16)
          {
            if (Serial.available() > 0)
            {
              password[i++] = Serial.read();
            }
          }
          memset(shifInValues, 0, chainLen);
          shifInValues[156] = 1;

          for (int i = 0; i < 16; i++)
            for (int j = 0; j < 8; j++)
              shifInValues[8 * i + j + offset] = (password[i] >> (7 - j)) & 0x01;
          chain_ShiftOut(chainLen, shifInValues, shifOutValues);
          chain_Capture();
        }
        break;
      case 0x04:  //SET OFFSET PASSWORD
        {
          int i = 0;
          byte tempOffset[2];
          while (i < 2)
          {
            if (Serial.available() > 0)
            {
              tempOffset[i++] = Serial.read();
            }
          }
          offset = (tempOffset[0] << 8) + tempOffset[1];
        }
        break;
      case 0x05:  //GET SCAN CHAIN STATUS PASSWORD
        {
          memset(shifInValues, 0, chainLen);
          chain_ShiftOut(chainLen, shifInValues, shifOutValues);
          byte out[chainLen/8];
          bit2byte(chainLen, shifOutValues, out);
          ByteArrayPrint(chainLen/8, out, false);
        }
        break;
      case 0x06:  //GET COMPLETE SCANOUT (SCANIN = 512 bits 1 except the two Self Destruction bits)
        {
          //create shiftIn values
          memset(shifInValues, 1, chainLen);
          shifInValues[51] = 0; //disable the bits that produce self destruction
          shifInValues[61] = 0; //disable the bits that produce self destruction
          
          //Shift In, Capture & Shift Out
          chain_ShiftIn(chainLen, shifInValues);
          chain_Capture();
          chain_ShiftOut(chainLen, shifInValues, shifOutValues);
          
          byte out[chainLen/8];
          bit2byte(chainLen, shifOutValues, out);
          ByteArrayPrint(chainLen/8, out, false);
        }
        break;
    }
  }

  /*
    reset();
    delay(3000);
  */

  /*
    //DETECT CHAIN LEN
    int chainLen = detectChainLen(2000);
    sprintf(msgString, "Detected chain lenght = %d", chainLen);
    Serial.println(msgString);
  */

  /*
    //SCAN FFs
    int chainLen = 512;
    byte shifInValues[chainLen];
    byte shifOutValues[chainLen];
    for (int i = 0; i < chainLen; i++)
    {
      //create shiftIn values
      memset(shifInValues, 0, chainLen);
      shifInValues[i] = 1;
      
      //Shift In, Capture & Shift Out
      chain_ShiftIn(chainLen, shifInValues);
      chain_Capture();
      chain_ShiftOut(chainLen, shifInValues, shifOutValues);
      
      //print
      printInOut(chainLen, shifInValues, shifOutValues);
  
      //Reset for next test
      delay(30);
      reset();
      delay(2000);
    }
  */

  /*
    while(true)
    {
		delay(1000);
    }
  */
}
