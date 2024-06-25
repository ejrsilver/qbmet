#include <linux/spi/spidev.h>
#include <linux/types.h>

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

// Some sample values for acceleration and velocity
#define MOT0_AMAX 500
#define MOT0_VMAX 1200

#define MOT1_AMAX 50
#define MOT1_VMAX 2047

#define MOT2_VMAX 2047

/* The following array contains:
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

/*
 * Function: uint8_t read_write_spi(uint8_t writebyte)
 * Purpose: Write one byte to the SPI interface
 * and read back one byte.
 * Parameter: writebyte: byte to be written.
 * Return value: byte that has been read back.
 */
uint8_t read_write_spi(uint8_t writebyte) {
  struct spi_ioc_transfer xfer;
  unsigned long rx_buf;
  int status;

  // This function is controller specific and has to to
  // the following things:

  // Send the contents of "writebyte" to the TCM429 via the SPI interface
  xfer.tx_buf = writebyte;
  xfer.rx_buf = rx_buf;
  xfer.len = 1;

  // (MSB first!!!!!!!!!!!).
  ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
  if (status < 0) {
    perror("Could not write byte IOC_MESSAGE\n");
    printf("Could not write byte IOC_MESSAGE\n");
  }

  // Return the byte that has been written back from the TMC429.
  return rx_buf;
}

/*
 * Function:  Send429
 * Purpose: Send four Bytes to the TMC429
 * Parameter:    a,b,c,d: the bytes that are to be written
 */
void Send429(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
  read_write_spi(a);
  read_write_spi(b);
  read_write_spi(c);
  read_write_spi(d);
}

/*
 * Function:  Send429
 * Purpose: Initialize the TMC429
 * Return value: 0 for success, 1 for failure.
 */
int Init429(void) {
  int i;

  fd = open(SPI_DEVICE, O_RDWR);

  if (fd < 0) {
    perror("Could not open SPI file.");
    return 1;
  }

  // Write the driver configuration data to the TMC429.
  // This is the most important part of the intialization and should be done
  // first of all!
  for (i = 128; i < 256; i += 2) {
    Send429(i, 0, driver_table[i - 127], driver_table[i - 128]);
  }

  // Set the "Stepper Motor Global Parameter"-Registers (SMGP).
  // Set SPI_CONTINUOS_UPDATE  and PoFD, SPI_CLKDIV=7
  //       LSMD = 2 (which means 3 motors!)
  Send429(IDX_SMGP, 0x01, 0x07, 0x22);

  // Set the coil current parameters (which is application specific)
  Send429(IDX_AGTAT_ALEAT | MOTOR0, 0x00, 0x10,
          0x40); // i_s_agtat/i_s_aleat/i_s_v0/a_threshold
  Send429(IDX_AGTAT_ALEAT | MOTOR1, 0x00, 0x10, 0x40);
  Send429(IDX_AGTAT_ALEAT | MOTOR2, 0x00, 0x10, 0x40);

  // Set the pre-dividers and the microstep resolution (which is also
  // application and motor specific)
  Send429(IDX_PULSEDIV_RAMPDIV | MOTOR0, 0x00, 0x55,
          0x04); // pulsdiv/rampdiv/µStep
  Send429(IDX_PULSEDIV_RAMPDIV | MOTOR1, 0x00, 0x55, 0x04);
  Send429(IDX_PULSEDIV_RAMPDIV | MOTOR2, 0x00, 0x55, 0x04);

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

  // Example for motor #1: VELOCITY mode
  Send429(IDX_VMIN | MOTOR1, 0, 0, 1); // Vmin = 1
  Send429(IDX_VMAX | MOTOR1, 0, MOT1_VMAX >> 8,
          MOT1_VMAX & 0xff); // VMax = MOT1_VMAX (s. oben)
  Send429(IDX_AMAX | MOTOR1, 0, MOT1_AMAX >> 8,
          MOT1_AMAX & 0xff); // AMax = MOT1_AMAX
  Send429(IDX_REFCONF_RM | MOTOR1, 0, NO_REF,
          RM_VELOCITY); // VELOCITY mode, no stop switches

  // Motor #1 can now be driven by setting the VTarget register (IDX_VTARGET) to
  // the desired velocity. The AMax value will be used to accelerate or
  // decelerate the motor.

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

int main() {
  Init429();

  // Trying to set the target velocity.
  Send429(IDX_VTARGET | MOTOR1, 0, 0, 4);

  while (1) {
  }
}
