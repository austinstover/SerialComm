/*
 * @author Austin Stover
 * October 2016
 * A helper program to send serial data
 * NOTE: Serial.begin() must be called before any of the send functions in this file
 */

void sendDebug(const char* message)
{
  int len = strlen(message);
  Serial.write(MAGIC_NUM); //Header: Magic number
  Serial.write(DEBUG_STRING); //Payload: key
  Serial.write(len >> 8); //Payload: Value: length of string
  Serial.write(len);
  byte dataOut[len + 2];
  dataOut[0] = len >> 8;
  dataOut[1] = len;
  for (int n = 0; n < len; n++)
  {
    Serial.write(message[n]); //Payload: Value: string
    dataOut[n + 2] = (byte)message[n];
  }
  Serial.write(lRCheck(dataOut, len + 2));
  Serial.flush();
}

void sendError(const char* message)
{
  int len = strlen(message);
  Serial.write(MAGIC_NUM);
  Serial.write(ERROR_STRING);
  Serial.write(len >> 8);
  Serial.write(len);
  byte dataOut[len + 2];
  dataOut[0] = len >> 8;
  dataOut[1] = len;
  for (int n = 0; n < len; n++)
  {
    Serial.write(message[n]); //Payload: Value: string
    dataOut[n + 2] = (byte)message[n];
  }
  Serial.write(lRCheck(dataOut, len + 2));
  Serial.flush();
}

void sendTimestamp(unsigned long timestamp)
{
  Serial.write(MAGIC_NUM);
  Serial.write(TIMESTAMP);
  byte dataOut[4] = {timestamp >> 24, timestamp >> 16, timestamp >> 8, timestamp};
  Serial.write(dataOut, 4);
  Serial.write(lRCheck(dataOut, 4));
  Serial.flush();
}

void sendThrust(float thrust)
{
  Serial.write(MAGIC_NUM);
  Serial.write(THRUST);
  sendFloat(thrust);
}

void sendRotSpeed(float rotSpeed)
{
  Serial.write(MAGIC_NUM);
  Serial.write(ROT_SPEED);
  sendFloat(rotSpeed);
}

void sendCurrent(float current)
{
  Serial.write(MAGIC_NUM);
  Serial.write(CURRENT);
  sendFloat(current);
}

void sendVoltage(float voltage)
{
  Serial.write(MAGIC_NUM);
  Serial.write(VOLTAGE);
  sendFloat(voltage);
}

void sendFloat(float floatToSend)
{
  union data //Float conversion from Puneet Sashdeva
  {
    float a;
    byte datas[4];
  }
  data;

  data.a = floatToSend;
  Serial.write(data.datas[0]);
  Serial.write(data.datas[1]);
  Serial.write(data.datas[2]);
  Serial.write(data.datas[3]);
  Serial.write(lRCheck(data.datas, 4));
  Serial.flush();
}
