#include <DuinoCube.h>
#include <SPI.h>

static JoystickState prev_state;

void setup() {
  // Set up standard output over UART.
  Serial.begin(115200);

  DC.begin();
  memset(&prev_state, 0, sizeof(prev_state));
}

void loop() {
  // Repeatedly read in the joystick state.
  JoystickState state = DC.USB.readJoystick();
  if (memcmp(&state, &prev_state, sizeof(state)) == 0)
    return;

  // If the joystic state changed, print the new state.
  prev_state = state;
  printf("Joystick: buttons = 0x%04x, X = %d, Y = %d\n",
         state.buttons, state.x, state.y);
}
