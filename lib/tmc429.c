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
#define SELECT_429()   // FILL IN HERE: Set the  nCS line of the TMC429 to low.
#define DESELECT_429() // FILL IN HERE: Set the  nCS line of the TMC429 to high.
#define SPI_DEVICE "/dev/spidev0.0"

#define SPI_SPEED 5000
#define SPI_DELAY 5
#define SPI_BITS_PER_WORD 8

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
uint8_t driver_table[128] = {
    0x10, 0x05, 0x04, 0x03, 0x02, 0x06, 0x10, 0x0D, 0x0C, 0x0B, 0x0A, 0x2E,
    0x10, 0x05, 0x04, 0x03, 0x02, 0x06, 0x10, 0x0D, 0x0C, 0x0B, 0x0A, 0x2E,
    0x10, 0x05, 0x04, 0x03, 0x02, 0x06, 0x10, 0x0D, 0x0C, 0x0B, 0x0A, 0x2E,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0B,
    0x0C, 0x0E, 0x10, 0x11, 0x13, 0x14, 0x16, 0x17, 0x18, 0x1A, 0x1B, 0x1D,
    0x1E, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x27, 0x29, 0x2A, 0x2B, 0x2C,
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x38, 0x39, 0x3A, 0x3B, 0x3B, 0x3C, 0x3C, 0x3D, 0x3D, 0x3E, 0x3E, 0x3E,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F};

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
  uint32_t data_in, data_out;
  uint8_t buf_tx[32], buf_rx[32];
  int32_t status;

  memset(&xfer, 0, sizeof(xfer));

  data_in = ((0x1 & rrs) << 31) | ((0x3 & smda) << 29) | ((0xF & idx) << 25) |
            ((0x1 & rw) << 24) | data & 0xFFFFFF;

  buf_tx[0] = data_in >> 24;
  buf_tx[1] = (data_in >> 16) & 0xFF;
  buf_tx[2] = (data_in >> 8) & 0xFF;
  buf_tx[3] = data_in & 0xFF;

  xfer.tx_buf = (unsigned long)buf_tx;
  xfer.rx_buf = (unsigned long)buf_rx;
  xfer.len = 4;
  xfer.delay_usecs = SPI_DELAY;
  xfer.speed_hz = SPI_SPEED;
  xfer.bits_per_word = SPI_BITS_PER_WORD;

  printf("original: %x, in_arr: 0x", data_in);

  for (int i = 0; i < 4; i++) {
    printf("%x", buf_tx[i]);
  }

  printf("\n");

  status = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0) {
    perror("Could not write byte IOC_MESSAGE\n");
    exit(EXIT_FAILURE);
  }

  data_out = (((uint32_t)buf_rx[0]) << 24) | (((uint32_t)buf_rx[1]) << 16) |
             (((uint32_t)buf_rx[2]) << 8) | (((uint32_t)buf_rx[3]));

  printf("output: %x, out_arr: 0x", data_out);

  for (int i = 0; i < 4; i++) {
    printf("%x", buf_rx[i]);
  }

  printf("\n");

  // Get the data back out of the buffer (This can probably be optimized).
  return data_out;
}

/**
 * Write the ram tab into the TMC429.
 *
 * Negative on failure, positive on success.
 */
void write_config(const uint8_t *const ram_tab) {
  for (int i = 0; i < 126; i += 2) {
    rw_spi(1, i >> 4, i & 0xFF, 0,
           (((uint32_t)ram_tab[i]) << 8) | ram_tab[i + 1]);
  }
}

/**
 * Read the config and write it into the ram tab.
 *
 * Negative on failure, positive on success.
 */
void read_config(uint8_t *ram_tab) {
  int32_t status;
  for (int i = 0; i < 126; i += 2) {
    status = rw_spi(1, i >> 4, i & 0xFF, 1,
                    (((uint32_t)ram_tab[i]) << 8) | ram_tab[i + 1]);
    ram_tab[i] = status >> 8 & 0xFF;
    ram_tab[i + 1] = status >> 16 & 0xFF;
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
