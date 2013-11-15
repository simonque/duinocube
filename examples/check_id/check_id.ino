#include <DuinoCube.h>
#include <SPI.h>

void setup() {
  Serial.begin(115200);
  DC.begin();
}

void loop() {
  uint16_t id = DC.Core.readWord(REG_ID);
  printf("ID: 0x%04x\n", id);
}
