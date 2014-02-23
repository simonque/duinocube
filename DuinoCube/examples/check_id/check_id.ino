#include <DuinoCube.h>
#include <SPI.h>

void setup() {
  Serial.begin(115200);
  DC.begin();

  loop();
}

void loop() {
  uint16_t id = DC.Core.readWord(REG_ID);
  printf("ID: 0x%04x\n", id);

  uint16_t id_rpc = DC.RPC.readCoreID();
  printf("ID over RPC: 0x%04x\n", id_rpc);

  while (true);
}
