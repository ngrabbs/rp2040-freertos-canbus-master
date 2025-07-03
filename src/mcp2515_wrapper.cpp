// mpc2515_wrapper.cpp
// Thin C-callable wrapper around the C++ Adafruit_MCP2515 object

#include "Adafruit_MCP2515.h"

static Adafruit_MCP2515 mcp(CS_PIN);

extern "C"
{

#include <stdint.h>
#include <stdbool.h>

#define CAN_BAUDRATE 1000000

  bool mcp2515_init()
  {
    printf("MCP2515 init()!\n");

    if (!mcp.begin(CAN_BAUDRATE))
    {
      printf("Error initializing MCP2515.\n");
      sleep_ms(10);
    }
    printf("MCP2515 chip found\n");
    return
  }
}
