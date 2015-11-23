#include <Wire.h>
#include <SPI.h>

void setup() {
  // Initialize communications
  Wire.begin(0xf8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  SPI.setBitOrder(LSBFIRST);
  SPI.begin();

  // Set D-Register to OUTPUT & HIGH
  DDRD = 0xff;
  PORTD = 0xff;
}

void loop() {
  // Nothing here, only do stuff if I2C receives something
  delay(100);
}

/*
 * I2C receive event
 */
void receiveEvent(int howMany) {
  if (Wire.read() == 0xaa)
    // 0xaa -> Set LED's
    updateBoard();
}

/*
 * Read 8 bytes from I2C and write them to the shift-registers
 */
void updateBoard() {
  int currentRow = 0;
  while (Wire.available() > 0 && currentRow < 8) {
    byte row = Wire.read();
    SPI.transfer(row);
    enableRow(currentRow);
    currentRow++;
  }
}

/*
 * Pulse RCK of ShiftRegister to write data from Storage to Output Register
 */
void enableRow (int row) {
  PORTD = ~(1 << row);
  PORTD = 0xff;
}

