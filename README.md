# SerialComm
A simple protocol for uart serial communication between a computer running Python and an Arduino microcontroller. Has error checking.

Implements a protocol for the transfer of UTF-8 strings, 4-byte and 2-byte integers, and 4-byte floats. The implementation is project-specific (it was originally made for a motor-test-stand) but can be adapted to fit any project. Error checking uses a longitudinal redundancy check (LRC) byte, and erroneous messages are ignored.

**Uses Python 2.7**

**Documentation:** <https://austinstover.github.io/SerialComm/>
 - Currently has issues with theming/display but this will be fixed (at some point)

**Required Packages:**
  - [PySerial 2.7](https://web.archive.org/web/20150808165248/http://pyserial.sourceforge.net:80/pyserial_api.html)
