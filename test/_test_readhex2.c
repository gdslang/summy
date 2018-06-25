#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <gdsl.h>
#include <decoder_config.h>

size_t readhex_hex_read(FILE *f, uint8_t **buffer) {
  size_t length_str = 0;
  char target[1];

  char next;
  target[0] = next - '0';

  return 0;
}

int main(int argc, char** argv) {
  char retval = 0;

  state_t state;

  struct config_handlers handlers;
  int i;
  while(1) {
     if(!strcmp(argv[i], "--")) {
      char success;
      decoder_config_from_args(&success, state, handlers, argc - i - 1, argv + i + 1);

      break;
    } else {
    }
  }

  uint8_t *buffer;
  size_t size = readhex_hex_read(stdin, &buffer);

  return 0;
}
