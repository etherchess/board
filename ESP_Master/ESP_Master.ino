#include <SPI.h>
#include <ESP8266WiFi.h>
#include <Espanol.h>
#include <Wire.h>

// Master-Slave Communication-Protocol
#define atmslave_addr     0xf8
#define updateboard_cmd   0xaa

// Variables for use in commandReceived()
String message, commandType;
byte rowBuffer;

// Chess-Board LED-Buffer
byte* rows = clearedByteArr(8);

// WiFi-Config
char* ssid     = "";
char* password = "";
char* broker   = "";
int port       = 1883;

Espanol denada(ssid, password, broker, port, commandReceived);

void setup()
{
  // Initialize communications 
  Serial.begin(115200);
  Wire.begin(0, 2);

  // Clear board
  updateBoard();

  // Subscribe to MQTT-Channels
  denada.subscribe("etherchess/board0/commands");
}

void loop()
{
    denada.loop();
}

void commandReceived(char* topic, byte* payload, unsigned int length)
{
  // Terminate payload at desired length with 0-byte
  // (Nescessary as payload doesn't get cleared of last message)
  payload[length] = 0;
  message = ((const char*) payload);

  // Parse command
  commandType = getValue(message, ' ', 0);
    
  if (commandType == "ssr") {
    // ssr -> set single row
    int rowToUpdate = getValue(message, ' ', 1).toInt();
    if (rowToUpdate >= 0 && rowToUpdate < 8) {
      rows[rowToUpdate] = strToByte(getValue(message, ' ', 2));
    }
  }

  else if (commandType == "u") {
    // u -> update board
    updateBoard;
  }
}

/*
 * Send Board-LED state to ATmega-Slave over i2c
 * Data taken from byte-array; size: 8
 * Clearing recommended before usage
 */
void updateBoard() {
  Wire.beginTransmission(atmslave_addr);
  Wire.write(updateboard_cmd);
  for (int i = 0; i < 8; i++) {
    Wire.write(rows[i]);
  }
  Wire.endTransmission();
}

/*
 * Sets byte-array contents to 0-bytes
 */
byte* clearedByteArr (int size) {
  byte* clearedArr = new byte[size];
  for (int i = 0; i < size; i++) {
    clearedArr[i] = 0x00;
  }
  return clearedArr;
}

/*
 * Split String by separator and get value at index
 * 
 * Snippet from http://stackoverflow.com/questions/9072320/split-string-into-string-array
 *         by   stackoverflow-user Alvaro Luis Bustamante
 */
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

/*
 * Convert first 8 chars of a String to a byte.
 * "10101010" -> 0b10101010
 * "ab010101" -> 0b00010101
 * "0110"     -> 0b01100000
 */
byte strToByte (String str) {
  byte returnVal = 0b00000000;
  for (int i = 0; i < 8 && i < str.length(); i++) {
    if (str.substring(i, i+1) == "1")
      returnVal = ((1 << 7-i) ^ returnVal);
  }
  return returnVal;
}

