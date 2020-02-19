"""
* Author: Austin Stover
* Date: July-August 2017

* This program performs transmission and reception of serial data from the SerialComm    \
  class with the stated protocol

Useful Links:
   * `PySerial 2.7 Documentation <https://web.archive.org/web/20150808165248/http://pyserial.sourceforge.net:80/pyserial_api.html>`_

"""

import serial
import struct
from collections import deque


class SerialComm(object):
    """
    A Serial Communication Class to communicate with an Arduino

    :param serialPort: A PySerial object
    :param dictLength: The max # of messages of each type to store

    This class receives and transmits messages with the following protocol:
    
    ============ ================ ======================= ================ ================ ===== ==========================
    Protocol
    ------------------------------------------------------------------------------------------------------------------------
    Byte number:        1.                  2.                   3.               4.         ...              N.
    ============ ================ ======================= ================ ================ ===== ==========================
    Description: [ Magic Number ] [ Type Identifier Key ] [ Data Payload ] [ Data Payload ]  etc  [ Error Check (LRC) Byte ]
    ============ ================ ======================= ================ ================ ===== ==========================

    To Use:
    
       * Send messages with the send...() methods
       * Receive messages by putting processData() in a loop--This data will be stored   \
         in the dict variable, with a key for each message type and a deque with the     \
         messages for each key
    """
    
    MAGIC_NUM = ord('!')
    DEBUG_STRING = 0x30         #UTF-8 encoded string
    ERROR_STRING = 0x31         #UTF-8 encoded string
    TIMESTAMP = 0x32            #4-Byte Integer
    # Python -> Arduino
    POWER_SETTING = 0x36        #2-Byte Integer (Converted from Boolean)
    THROTTLE_SETTING = 0x37     #2-Byte Integer (0-100)
    MAX_CURRENT = 0x38          #4-Byte Float
    MAX_VOLTAGE = 0x39          #4-Byte Float
    #Arduino -> Python
    THRUST = 0x40               #4-Byte Float
    ROT_SPEED = 0x41            #4-Byte Float
    CURRENT = 0x42              #4-Byte Float
    VOLTAGE = 0x43              #4-Byte Float

    def __init__(self, serialPort, dictLength=100):
        """Instantiate the object
        
        :param serialPort: A PySerial object
        :param dictLength: The max # of messages of each type to store
        """
        self._calcLRC = 0       #Calculated error check byte
        self._ser = serialPort  #Define a pySerial object

        self.dict = {'Debug':       deque(maxlen=dictLength),
                     'Error':       deque(maxlen=dictLength),
                     'Timestamp':   deque(maxlen=dictLength),
                     'Thrust':      deque(maxlen=dictLength),
                     'Rot Speed':   deque(maxlen=dictLength),
                     'Current':     deque(maxlen=dictLength),
                     'Voltage':     deque(maxlen=dictLength) }
        """
        A Dictionary to hold messages
        
        Each key is a message type which holds a fixed-length deque with recent messages \
        of that type
        """
    
    def processData(self, numLoops=1):
        """Processes the next byte of data received; if this byte is the beginning of a \
        message, adds the message to dict
        
        :param numLoops: The # of times to do this
        """
        for i in xrange(numLoops):
            if(self._ser.inWaiting() > 1000):                           #If too many bytes fill buffer, clear it
                self._ser.flushInput()
            elif(self._ser.inWaiting() > 0):                            #If bytes are available in buffer
                #print("Avail")
                if(ord(self._ser.read()) == SerialComm.MAGIC_NUM):      #If 1st byte == magic number
                    #print("Mag")
                    typeByte = ord(self._ser.read())                    #Read the type identifier byte
                    typeOfMessage, message = self._readData(typeByte)   #Read the rest of the message
                    lRCIn = ord(self._ser.read())                       #Read error check byte
                    if(typeOfMessage != None and self._calcLRC == lRCIn): #Ensure type byte found and do error check
                        #print typeOfMessage, ": ", message
                        self.dict[typeOfMessage].append(message)

   
    #   Output Functions:

    def sendDebug(self, stringToSend):
        """Sends a debug message
        
        :param stringToSend: The message
        """
        self._ser.write(bytearray([SerialComm.MAGIC_NUM]))
        self._ser.write(bytearray([SerialComm.DEBUG_STRING]))
        dataOut = bytearray(struct.pack('>H', len(stringToSend)))
        dataOut += bytearray(stringToSend, 'UTF-8')
        self._ser.write(dataOut)
        self._ser.write(SerialComm._lRCheck(dataOut))
        self._ser.flush()
        
    def sendError(self, stringToSend):
        """Sends an error message
        
        :param stringToSend: The message
        """
        self._ser.write(bytearray([SerialComm.MAGIC_NUM]))
        self._ser.write(bytearray([SerialComm.ERROR_STRING]))
        dataOut = bytearray(struct.pack('>H', len(stringToSend)))
        dataOut += bytearray(stringToSend, 'UTF-8')
        self._ser.write(dataOut)
        self._ser.write(SerialComm._lRCheck(dataOut))
        self._ser.flush()

    def sendTimestamp(self, intToSend):
        """Sends a timestamp message
        
        :param intToSend: The timestamp as an integer, in the range of a valid Arduino long
        """
        self._ser.write(bytearray([SerialComm.MAGIC_NUM]))
        self._ser.write(bytearray([SerialComm.TIMESTAMP]))
        dataOut = bytearray(struct.pack('>i', intToSend))
        self._ser.write(dataOut)
        self._ser.write(SerialComm._lRCheck(dataOut))
        self._ser.flush()

    def sendPowerSetting(self, boolToSend):
        """Sends the power-setting
        
        :param boolToSend: True for on, False for off
        :raises: ValueError
        """
        self._ser.write(bytearray([SerialComm.MAGIC_NUM]))
        self._ser.write(bytearray([SerialComm.POWER_SETTING]))
        shortToSend = int(boolToSend)
        if shortToSend != 0 and shortToSend != 1:
            raise ValueError("boolToSend must be either 1 or 0, not " + str(shortToSend))
        dataOut = bytearray(struct.pack('>h', shortToSend))
        self._ser.write(dataOut)
        self._ser.write(SerialComm._lRCheck(dataOut))
        self._ser.flush()

    def sendThrottleSetting(self, shortToSend):
        """Sends the throttle setting
        
        :param shortToSend: An integer between 0 and 100, inclusive
        """
        self._ser.write(bytearray([SerialComm.MAGIC_NUM]))
        self._ser.write(bytearray([SerialComm.THROTTLE_SETTING]))
        dataOut = bytearray(struct.pack('>h', shortToSend))
        self._ser.write(dataOut)
        self._ser.write(SerialComm._lRCheck(dataOut))
        self._ser.flush()

    def sendMaxCurrent(self, floatToSend):
        """Sends the max current
        
        :param floatToSend: The max current as a floating-point # in the range of an \
        Arduino double/float
        """
        self._ser.write(bytearray([SerialComm.MAGIC_NUM]))
        self._ser.write(bytearray([SerialComm.MAX_CURRENT]))
        dataOut = bytearray(struct.pack('<f', floatToSend))
        self._ser.write(dataOut)
        self._ser.write(SerialComm._lRCheck(dataOut))
        self._ser.flush()
        
    def sendMaxVoltage(self, floatToSend):
        """Sends the max voltage
        
        :param floatToSend: The max current as a floating-point # in the range of an \
        Arduino double/float
        """
        self._ser.write(bytearray([SerialComm.MAGIC_NUM]))
        self._ser.write(bytearray([SerialComm.MAX_VOLTAGE]))
        dataOut = bytearray(struct.pack('<f', floatToSend))
        self._ser.write(dataOut)
        self._ser.write(SerialComm._lRCheck(dataOut))
        self._ser.flush()


    # Input Functions:

    def _readData(self, typeIDByte):
        """Reads data pertaining to the type identifier byte received
        
        :param typeIDByte: The type identifier byte (An integer)
        :returns: A tuple consisting of (message-type, message) or (None,None) if   \  
        typeIDByte not recognized
        """
        if(typeIDByte == SerialComm.DEBUG_STRING):
            return 'Debug', self._readUTF()
        elif(typeIDByte == SerialComm.ERROR_STRING):
            return 'Error', self._readUTF()
        elif(typeIDByte == SerialComm.TIMESTAMP):
            return 'Timestamp', self._readInt()
        elif(typeIDByte == SerialComm.THRUST):
            return 'Thrust', self._readFloat()
        elif(typeIDByte == SerialComm.ROT_SPEED):
            return 'Rot Speed', self._readFloat()
        elif(typeIDByte == SerialComm.CURRENT):
            return 'Current', self._readFloat()
        elif(typeIDByte == SerialComm.VOLTAGE):
            return 'Voltage', self._readFloat()
        else:
            return None, None
    
    def _readUTF(self):
        """Reads a string in UTF format (An Arduino C String)
        
        :returns: The received string"""
        dataIn = self._ser.read(size=2)
        lenStr = struct.unpack('>H', dataIn)[0] #First 2 bytes (unsigned short) define length of string
        message = '';
        for i in range(lenStr):                 #Remaining bytes define string
            datumIn = self._ser.read()
            dataIn += datumIn
            message += datumIn
        self._calcLRC = SerialComm._lRCheck(bytearray(dataIn))
        return message

    def _readInt(self):
        """Reads a signed 4-Byte integer (An Arduino C long) in big-endian format
        
        :returns: The received integer
        """
        dataIn = self._ser.read(size=4)
        self._calcLRC = SerialComm._lRCheck(bytearray(dataIn))
        return struct.unpack('>i', dataIn)[0]

    def _readShort(self):
        """Reads a signed 2-Byte integer (An Arduino C int) in big-endian format and returns it
        
        :returns: The received integer
        """
        dataIn = self._ser.read(size=2)
        self._calcLRC = SerialComm._lRCheck(bytearray(dataIn))
        return struct.unpack('>h', dataIn)[0]

    def _readFloat(self):
        """Reads a 4-Byte float (An Arduino C float) in little-endian format and returns it
        
        :returns: The received floating-point number
        """
        dataIn = self._ser.read(size=4)
        self._calcLRC = SerialComm._lRCheck(bytearray(dataIn))
        return struct.unpack('<f', dataIn)[0]


    # Error Check Function

    @staticmethod
    def _lRCheck(dataIn):
        """Calculates the longitudinal redundancy check byte
        
        :param data: The payload
        :returns: The LRC byte (An integer)
        """
        lRC = 0
        for b in dataIn:
            lRC = (lRC + b) & 0xFF
        lRC = ((lRC ^ 0xFF) + 2) & 0xFF
        return lRC


def main():
    """Demo Program: prints a dictionary of messages after some # received"""
    import sys
    
    ser = serial.Serial('COM3', 9600, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE)
    commChan = SerialComm(ser)

    while True:
        
        commChan.processData()

        #print commChan.dict
        '''if(len(commChan.dict['Rot Speed']) >= 1):
            print commChan.dict['Rot Speed'][0] #Print messages when there are enough of one kind stored
            #sys.exit()'''

if __name__ == '__main__':
    main()

