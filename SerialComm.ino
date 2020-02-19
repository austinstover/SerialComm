/*
 * @author Austin Stover
 * July 2017
 * This program transmits motor test stand data over a serial connection between an 
 * Arduino and a computer.
 */


long lastDeltaTime = 0;

const byte MAGIC_NUM = '!';
// Types of Messages:
const byte DEBUG_STRING = 0x30;     //UTF-8 Encoded String
const byte ERROR_STRING = 0x31;     //UTF-8 Encoded String
const byte TIMESTAMP = 0x32;        //4-Byte Integer
// Python -> Arduino
const byte POWER_SETTING = 0x36;    //2-Byte Integer (Convert to Boolean)
const byte THROTTLE_SETTING = 0x37; //2-Byte Integer
const byte MAX_CURRENT = 0x38;      //4-Byte Float
const byte MAX_VOLTAGE = 0x39;      //4-Byte Float
// Arduino -> Python
const byte THRUST = 0x40;           //4-Byte Float
const byte ROT_SPEED = 0x41;        //4-Byte Float
const byte CURRENT = 0x42;          //4-Byte Float
const byte VOLTAGE = 0x43;          //4-Byte Float

int calcLRC; //Calculated Longitudinal Redundancy Check byte

const int DATA_STRING_SIZE = 51;
char dataString[DATA_STRING_SIZE] = ""; //A char array to hold each message's data--its length limits message size


void setup()
{
  Serial.begin(9600);
}

void loop()
{
  if(newPeriod(20))
  {
    sendDebug("Hello World");
    sendError("Help");
    sendTimestamp(millis());
    sendThrust(0.5);
    sendRotSpeed(-0.25);
    sendCurrent(-0.3);
    sendVoltage(0.4);
  }


  if(Serial.available())
  {
    delay(1);
    if(Serial.read() == MAGIC_NUM)
    {
      delay(1);
      byte typeByte = Serial.read();
      int returnInt;
      memset(dataString, 0, DATA_STRING_SIZE * sizeof(char)); //Make char array empty by setting it to nulls
      returnInt = readData(typeByte, dataString, DATA_STRING_SIZE);
      delay(1);
      byte lRCIn = Serial.read();
      
      if(returnInt == 0 && calcLRC == lRCIn) //If no problems and no transcription errors
      {
        sendDebug(dataString);
      }
    }
  }
  
}

/**
 * Calculates the Longitudinal Redundancy Check byte
 * @param data The payload as an array of bytes
 * @param dataLength The length of data[]
 * @return The LRC byte
 */
byte lRCheck(byte data[], int dataLength)
{
  int lRC = 0;
  for(int i = 0; i < dataLength; i++)
  {
    lRC = (lRC + data[i]) & 0xFF;
  }
  lRC = ((lRC ^ 0xFF) + 2) & 0xFF;
  return (byte)lRC;
}

/*
 * Sets a delta time loop for the communication--use this with an if() block
 * @param period The time in between runs, in milliseconds
 * @return True if the period has passed
 */
bool newPeriod(long period)
{
  long deltaTime = millis() - lastDeltaTime;
  if (deltaTime > period)
  {
    lastDeltaTime = millis();
    return true;
  }
  return false;
}
