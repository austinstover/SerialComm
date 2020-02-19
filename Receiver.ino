/*
 * @author Austin Stover
 * July 2017
 * A helper program to receive serial data
 */

int readData(byte typeOfDataByte, char charArray[25], int charArraySize)
{
  switch(typeOfDataByte)
  {
    case DEBUG_STRING:
//      sendDebug("Debug: ");
      return readUTF(charArray, charArraySize);
    case ERROR_STRING:
//      sendDebug("Error: ");
      return readUTF(charArray, charArraySize);
    case TIMESTAMP:
//      sendDebug("Timestamp: ");
      return intToCharArray(readInt(), charArray, charArraySize);
    case POWER_SETTING:
//      sendDebug("Power Setting: ");
      return boolToCharArray(readShort(), charArray, charArraySize);
    case THROTTLE_SETTING:
//      sendDebug("Throttle Setting: ");
      return shortToCharArray(readShort(), charArray, charArraySize);
    case MAX_CURRENT:
//      sendDebug("Max Current: ");
      return floatToCharArray(readFloat(), charArray, charArraySize);
    case MAX_VOLTAGE:
//      sendDebug("Max Voltage: );
      return floatToCharArray(readFloat(), charArray, charArraySize);
    default:
      return 1; //Error
  }
}

int readUTF(char charArray[26], int charArraySize)
{
  byte byteArray[2];
  delay(1);
  byteArray[0] = Serial.read();
  delay(1);
  byteArray[1] = Serial.read();
  unsigned short strLength = (byteArray[0] << 8) | byteArray[1];
  if(strLength > charArraySize)
  { //If message size exceeds char array size (usually due to bad strLength bytes)
    //sendError("Message Size Exceded Max:");
    char lgth[6];
    shortToCharArray(strLength, lgth, 6);
    //sendError(lgth);
    return 1;
  }
  else
  {
    byte dataIn[strLength + 2];
    dataIn[0] = byteArray[0];
    dataIn[1] = byteArray[1];
    for(int i = 0; i < strLength; i++)
    {
      delay(1);
      charArray[i] = Serial.read();
      dataIn[i + 2] = charArray[i];
    }
    calcLRC = lRCheck(dataIn, strLength + 2);
    return 0;
  }
}

long readInt()
{
  byte byteArray[4];
  delay(1);
  byteArray[3] = Serial.read();
  delay(1);
  byteArray[2] = Serial.read();
  delay(1);
  byteArray[1] = Serial.read();
  delay(1);
  byteArray[0] = Serial.read();
  calcLRC = lRCheck(byteArray, 4);
  return *((long*)(byteArray));
}

short readShort()
{
  byte byteArray[2];
  delay(1);
  byteArray[1] = Serial.read();
  delay(1);
  byteArray[0] = Serial.read();
  calcLRC = lRCheck(byteArray, 2);
  return *((short*)(byteArray));
}

double readFloat()
{
  byte byteArray[4];
  delay(1);
  byteArray[0] = Serial.read();
  delay(1);
  byteArray[1] = Serial.read();
  delay(1);
  byteArray[2] = Serial.read();
  delay(1);
  byteArray[3] = Serial.read();
  calcLRC = lRCheck(byteArray, 4);
  return *((float*)(byteArray));
}




/**
 * Prints an int as a string of chars
 * @param toConvert The int (Arduino long) to convert to a string
 * @param charArray An empty buffer of at least length 11
 * @param charArraySize The length of charArray (not including null terminator)
 * @return 0 if successful, 1 if error
 */
int intToCharArray(long toConvert, char charArray[11], int charArraySize)
{
  const int INT_SIZE = 11;
  if(charArraySize < INT_SIZE)
  {
    sendError("Array too small for int");
    return 1;
  }
  snprintf(charArray, INT_SIZE, "%ld", toConvert);
  return 0;
}

/**
 * Prints a short as a string of chars
 * @param toConvert The short to convert to a string
 * @param charArray An empty buffer of at least length 6
 * @param charArraySize The length of charArray (not including null terminator)
 * @return 0 if successful, 1 if error
 */
int shortToCharArray(short toConvert, char charArray[6], int charArraySize)
{
  const int SHORT_SIZE = 6;
  if(charArraySize < SHORT_SIZE)
  {
    sendError("Array too small for short");
    return 1;
  }
  snprintf(charArray, SHORT_SIZE, "%d", toConvert);
  return 0;
}

/**
 * Prints a short as a string of chars representing boolean vals
 * @param toConvert The short to convert to a string ("True" or "False")
 * @param charArraySize The length of charArray (not including null terminator)
 * @return 0 if successful, 1 if error
 */
int boolToCharArray(short toConvert, char charArray[5], int charArraySize)
{
  const int BOOL_SIZE = 5;
  if(charArraySize < BOOL_SIZE)
  {
    sendError("Array too small for bool");
    return 1;
  }
  
  if(toConvert == 1)
  {
    charArray[0] = 'T'; charArray[1] = 'r'; charArray[2] = 'u'; charArray[3] = 'e';
    return 0;
  }
  else if(toConvert == 0)
  {
    charArray[0] = 'F'; charArray[1] = 'a'; charArray[2] = 'l'; charArray[3] = 's';
    charArray[4] = 'e';
    return 0;
  }
  else
  { //If toConvert is neither 1 nor 0, it may have been corrupted
    return 1;
  }
}

/**
 * Prints a float as a string of chars
 * @param toConvert The float to convert to a string
 * @param charArray An empty buffer of at least length 14 to hold the converted float
 * @param charArraySize The length of charArray (not including null terminator)
 * @return 0 if successful, 1 if error
 */
int floatToCharArray(float toConvert, char charArray[14], int charArraySize)
{
  const int FLOAT_SIZE = 14;
  if(charArraySize < FLOAT_SIZE)
  {
    sendError("Array to small for float");
    return 1;
  }
  int powToConvert = floor(log10(abs(toConvert)));
  int len = (toConvert >= 0) ? 9 : 10; //If - sign, move 'E' one over
  if(toConvert > 3.4028235E38) //Above float MAX
  {
    charArray[0] = 'I'; charArray[1] = 'N'; charArray[2] = 'F';
  }
  else if(toConvert < -3.4028235E38) //Below float MIN
  {
    charArray[0] = '-'; charArray[1] = 'I'; charArray[2] = 'N'; charArray[3] = 'F';
  }
  else if(powToConvert >= 6)  //Sci. notation if float is too big
  {
    dtostrf(toConvert * pow(10, -powToConvert), 9, 7, charArray);
    
    charArray[len] = 'E';
    if(powToConvert > 9)
    {
      charArray[len + 1] = powToConvert / 10 + 0x30;
      charArray[len + 2] = powToConvert % 10 + 0x30;
    }
    else
    {
      charArray[len + 1] = powToConvert + 0x30;
    }
  }
  else if(powToConvert <= -3)
  {
    dtostrf(toConvert * pow(10, -powToConvert), 9, 7, charArray);
    charArray[len] = 'E';
    charArray[len + 1] = '-';
    if(abs(powToConvert) > 9)
    {
      charArray[len + 2] = abs(powToConvert) / 10 + 0x30;
      charArray[len + 3] = abs(powToConvert) % 10 + 0x30;
    }
    else
    {
      charArray[len + 2] = abs(powToConvert) + 0x30;
    }
  }
  else
  {
    dtostrf(toConvert, 9, 7 - powToConvert, charArray);
  }
  return 0;
}

