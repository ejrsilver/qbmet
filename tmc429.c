#include <linux/spi/spidev.h>
#include <linux/types.h>

#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SPI_DEVICE "/dev/spidev0.0"
#define SPI_SPEED 5000000
#define SPI_DELAY 0
#define SPI_BITS_PER_WORD 32

#include "tmc429.h" //Header file containing the register names of the TMC429

/**
 * The following array contains:
 * - the first 64 bytes: configuration data of the TMC429 containing
 *  the structure of the SPI telegrams that are to be sent to the
 *  drivers by the TMC429 (in this example: three TMC236).
 *
 * - the second 64 bytes: microstep wave table
 *   (sine wave, 1/4 period).
 */
const uint16_t driver_table[64] = {
    0x1005, 0x0403, 0x0206, 0x100D, 0x0C0B, 0x0A2E, 0x1005, 0x0403,
    0x0206, 0x100D, 0x0C0B, 0x0A2E, 0x1005, 0x0403, 0x0206, 0x100D,
    0x0C0B, 0x0A2E, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111, 0x1111,
    0x0002, 0x0305, 0x0608, 0x090B, 0x0C0E, 0x1011, 0x1314, 0x1617,
    0x181A, 0x1B1D, 0x1E20, 0x2122, 0x2425, 0x2627, 0x292A, 0x2B2C,
    0x2D2E, 0x2F30, 0x3132, 0x3334, 0x3536, 0x3738, 0x3839, 0x3A3B,
    0x3B3C, 0x3C3D, 0x3D3E, 0x3E3E, 0x3F3F, 0x3F3F, 0x3F3F, 0x3F3F};

// File for SPI communication.
int fd;

/**
 * Writes a 32-bit word to the TMC429 and receives a word back.
 *
 * rrs  - 1 bit.
 * smda - 2 bits.
 * idx  - 4 bits.
 * rw   - 1 bit.
 * data - 24 bits.
 */
int32_t rw_spi(uint8_t rrs, uint8_t smda, uint8_t idx, uint8_t rw,
               uint32_t data) {
  struct spi_ioc_transfer xfer;
  uint32_t data_in, data_out, *buf_tx, *buf_rx;
  int32_t status;

  memset(&xfer, 0, sizeof(xfer));

  // Move data into buffer.
  data_in = ((0x1 & rrs) << 31) | ((0x3 & smda) << 29) | ((0xF & idx) << 25) |
            ((0x1 & rw) << 24);

  xfer.tx_buf = (unsigned long)&data_in;
  xfer.rx_buf = (unsigned long)&data_out;
  xfer.len = 4;
  xfer.delay_usecs = SPI_DELAY;
  xfer.speed_hz = SPI_SPEED;
  xfer.bits_per_word = SPI_BITS_PER_WORD;

  status = ioctl(fd, SPI_IOC_MESSAGE(1), &xfer);
  if (status < 0) {
    perror("Could not write byte IOC_MESSAGE\n");
    exit(EXIT_FAILURE);
  }

  // Get the data back out of the buffer (This can probably be optimized).
  return data_out;
}

/**
 * Write the ram tab into the TMC429.
 *
 * Negative on failure, positive on success.
 */
void write_config(const uint16_t *const ram_tab) {
  int32_t status;
  for (int i = 0; i < 63; i++) {
    rw_spi(1, i >> 4, i & 0xFF, 0, ram_tab[i]);
  }
}

/**
 * Read the config and write it into the ram tab.
 *
 * Negative on failure, positive on success.
 */
void read_config(uint8_t *ram_tab) {
  for (int i = 0; i < 63; i++) {
    ram_tab[i] = rw_spi(1, i >> 4, i & 0xFF, 1, ram_tab[i]);
  }
}

/**
 * Initialize the TMC429.
 */
void init() {
  int32_t i, data;
  fd = open(SPI_DEVICE, O_RDWR);
  if (fd < 0) {
    perror("Could not open SPI file.");
    exit(EXIT_FAILURE);
  }

  // Write the driver configuration data to the TMC429.
  write_config(driver_table);

  // Stepper Motor Global Parameter.
  // Set SPI_CONTINUOS_UPDATE=1, PoFD=1, SPI_CLKDIV=7, LSMD=0 (1 motor)
  // Send429(IDX_SMGP, 0x01, 0x07, 0x22);
  rw_spi(0, SMDA_OTHER, ADDR_SMGP, 0, 0x010720);

  // Set the coil current parameters (which is application specific)
  // i_s_agtat/i_s_aleat/i_s_v0/a_threshold
  rw_spi(0, SMDA_MOTOR0, ADDR_THRESHOLD, 0, 0x001040);

  // Set the pre-dividers and the microstep resolution (which is also
  // application and motor specific)
  // pulsdiv/rampdiv/µStep
  rw_spi(0, SMDA_MOTOR0, ADDR_PULSEDIV_RAMPDIV, 0, 0x005504);

  // Initialize motor 0 in velocity mode.
  rw_spi(0, SMDA_MOTOR0, ADDR_VMIN, 0, 1);        // Vmin = 1
  rw_spi(0, SMDA_MOTOR0, ADDR_VMAX, 0, 0x0007FF); // Vmax = 2047
  rw_spi(0, SMDA_MOTOR0, ADDR_AMAX, 0, 0x0007FF); // Amax = 2047
  rw_spi(0, SMDA_MOTOR0, ADDR_REFCONF_RM, 0,
         0x000F02); // NO_REF = 0x0f, RM_VELOCITY = 0x02

  // Start the motor moving.
  rw_spi(0, SMDA_MOTOR0, ADDR_VTARGET, 0, 0x00008F); // Set the target velocity.
  data = rw_spi(0, SMDA_MOTOR0, ADDR_VTARGET, 1,
                0x00008F); // Check that the target velocity was set.
  printf("input: %x, output: %x\n", 0x1000008F, data);
}

int main() { init(); }
