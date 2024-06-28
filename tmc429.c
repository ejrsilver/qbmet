#include <linux/spi/spidev.h>
#include <linux/types.h>

#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#define SELECT_429()   // FILL IN HERE: Set the  nCS line of the TMC429 to low.
#define DESELECT_429() // FILL IN HERE: Set the  nCS line of the TMC429 to high.
#define SPI_DEVICE "/dev/spidev0.0"

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
 * Formats data for sending to the TMC429;
 *
 * rrs  - 1 bit.
 * smda - 2 bits.
 * idx  - 4 bits.
 * rw   - 1 bit.
 * data - 24 bits.
 */
uint32_t format_data(uint8_t rrs, uint8_t smda, uint8_t idx, uint8_t rw,
                     uint32_t data) {
  return (uint32_t)(((0x1 & rrs) << 31) | ((0x3 & smda) << 29) |
                    ((0xF & idx) << 25) | ((0x1 & rw) << 24) |
                    (0xFFFFFF & data));
}

/**
 * Writes a 32-bit word to the TMC429 and receives a word back.
 */
uint32_t rw_spi(uint32_t data) {
  struct spi_ioc_transfer xfer[2];
  uint8_t buf[32], *bp;
  int status;
  uint32_t ret;

  memset(xfer, 0, sizeof(xfer));
  memset(buf, 0, sizeof(buf));

  // Move data into buffer.
  memcpy(buf, &data, 4);

  xfer[0].tx_buf = (unsigned long)buf;
  xfer[0].len = 4;

  xfer[1].rx_buf = (unsigned long)buf;
  xfer[1].len = 1;

  status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
  if (status < 0) {
    perror("Could not write byte IOC_MESSAGE\n");
    return NULL;
  }

  // Get the data back out of the buffer (This can probably be optimized).
  memcpy(&ret, buf, 4);
  return ret;
}

/**
 * Write the ram tab into the TMC429.
 */
void write_config(uint8_t *ram_tab) {
  for (int i = 0; i < (128 - 2);
       i +=
       2) // initialize TMC428 RAM table (SPI conf. & quarter sine wave LUT)
  {
    rw_spi((long int)(0x80000000 | (i << (25 - 1)) | (ram_tab[i + 1] << 8) |
                      (ram_tab[i]))); // RRS = 1, RW = 1
  }
}

/**
 * Read the config and write it into the ram tab.
 */
void read_config(uint8_t *ram_tab) {
  uint8_t i;
  uint32_t spi;

  for (i = 0; i < 127; i += 2) {
    spi = rw_spi((uint32_t)(0x81000000 | (i << (25 - 1)))); // RRS=1, RW=1

    ram_tab[i + 1] = (uint8_t)(0x3f & (spi >> 8));
    ram_tab[i] = (uint8_t)(0x3f & (spi >> 0));
  }
}

/**
 * Initialize the TMC429.
 */
int init() {
  int i;
  fd = open(SPI_DEVICE, O_RDWR);
  if (fd < 0) {
    perror("Could not open SPI file.");
    return -1;
  }

  // Write the driver configuration data to the TMC429.
  write_config(driver_table);

  // Stepper Motor Global Parameter.
  // Set SPI_CONTINUOS_UPDATE=1, PoFD=1, SPI_CLKDIV=7, LSMD=0 (1 motor)
  // Send429(IDX_SMGP, 0x01, 0x07, 0x22);
  rw_spi(format_data(0, SMDA_OTHER, ADDR_SMGP, 0, 0x010720));

  // Set the coil current parameters (which is application specific)
  // i_s_agtat/i_s_aleat/i_s_v0/a_threshold
  rw_spi(format_data(0, SMDA_MOTOR0, ADDR_THRESHOLD, 0, 0x001040));

  // Set the pre-dividers and the microstep resolution (which is also
  // application and motor specific)
  // pulsdiv/rampdiv/µStep
  rw_spi(format_data(0, SMDA_MOTOR0, ADDR_PULSEDIV_RAMPDIV, 0, 0x005504));

  // Now some examples of the driving parameters. This is highly application
  // specific!

  // Example for motor #0: RAMP mode
  // Send429(IDX_VMIN | MOTOR0, 0, 0, 1); // Vmin = 1
  // Send429(IDX_VMAX | MOTOR0, 0, MOT0_VMAX >> 8,
  //         MOT0_VMAX & 0xff); // VMax = MOT0_VMAX
  // Send429(IDX_AMAX | MOTOR0, 0, MOT0_AMAX >> 8,
  //         MOT0_AMAX & 0xff); // AMax = MOT0_AMAX
  // Send429(IDX_PMUL_PDIV | MOTOR0, 0, 150, 1);
  // Send429(IDX_REFCONF_RM | MOTOR0, 0, NO_REF,
  //         RM_RAMP); // RAMP mode, stop switches not active

  // Motor #0 can now be driven by setting the XTarget register (IDX_XTARGET) to
  // the desired end position. ATTENTION: The values in IDX_PMUL_PDIV depend on
  // the AMax and Pulsediv/Rampdiv values and have to be calculated using the
  // algorithm given in the data sheet. The values given here have been
  // calculated using the Calc429.exe program on the TechLibCD.

  // Initialize motor 0 in velocity mode.
  rw_spi(format_data(0, SMDA_MOTOR0, ADDR_VMIN, 0, 1));        // Vmin = 1
  rw_spi(format_data(0, SMDA_MOTOR0, ADDR_VMAX, 0, 0x0007FF)); // Vmax = 2047
  rw_spi(format_data(0, SMDA_MOTOR0, ADDR_AMAX, 0, 0x0007FF)); // Amax = 2047
  rw_spi(format_data(0, SMDA_MOTOR0, ADDR_REFCONF_RM, 0,
                     0x000F02)); // NO_REF = 0x0f, RM_VELOCITY = 0x02

  // Start the motor moving.
  rw_spi(format_data(0, SMDA_MOTOR0, ADDR_VTARGET, 0, 0x0008F)); // Amax = 2047

  // Example for motor #2: HOLD mode
  // Send429(IDX_VMIN | MOTOR2, 0, 0, 1); // Vmin = 1
  // Send429(IDX_VMAX | MOTOR2, 0, MOT2_VMAX >> 8,
  //         MOT2_VMAX & 0xff); // VMax = MOT2_VMAX (s. oben)
  // Send429(IDX_REFCONF_RM | MOTOR2, 0, NO_REF, RM_HOLD);

  // Motor #2 can now be driven by setting the VActual register (IDX_VACTUAL) to
  // the desired value. The motor will then uses that velocity immediately,
  // there is no accelerating or decelerating.
  return 0;
}

int main() { init(); }
