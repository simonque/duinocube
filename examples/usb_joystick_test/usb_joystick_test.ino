#include <DuinoCube.h>
#include <SPI.h>

static FILE uart_stdout;  // For linking UART to printf, etc.
static int uart_putchar (char c, FILE *stream) {
  Serial.write(c);
  return 0;
}

static JoystickState prev_state;

void setup() {
  // Set up standard output over UART.
  Serial.begin(115200);
  fdev_setup_stream(&uart_stdout, uart_putchar, NULL,
                    _FDEV_SETUP_WRITE);
  stdout = &uart_stdout;

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
