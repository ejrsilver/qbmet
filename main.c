#include "./lib/tmc429.c"

int main() {
  int32_t i, fd, data;
  fd = open(SPI_DEVICE, O_RDWR);
  if (fd < 0) {
    perror("Could not open SPI file.");
    exit(EXIT_FAILURE);
  }

  // Write the driver configuration data to the TMC429.
  write_config(fd, driver_table);

  // Stepper Motor Global Parameter.
  // Set SPI_CONTINUOS_UPDATE=1, PoFD=1, SPI_CLKDIV=7, LSMD=0 (1 motor)
  // Send429(IDX_SMGP, 0x01, 0x07, 0x22);
  rw_spi(fd, 0, SMDA_OTHER, ADDR_SMGP, 0, 0x010720);

  // Set the coil current parameters (which is application specific)
  // i_s_agtat/i_s_aleat/i_s_v0/a_threshold
  rw_spi(fd, 0, SMDA_MOTOR0, ADDR_THRESHOLD, 0, 0x001040);

  // Set the pre-dividers and the microstep resolution (which is also
  // application and motor specific)
  // pulsdiv/rampdiv/µStep
  rw_spi(fd, 0, SMDA_MOTOR0, ADDR_PULSEDIV_RAMPDIV, 0, 0x005504);

  // Initialize motor 0 in velocity mode.
  rw_spi(fd, 0, SMDA_MOTOR0, ADDR_VMIN, 0, 1);        // Vmin = 1
  rw_spi(fd, 0, SMDA_MOTOR0, ADDR_VMAX, 0, 0x0007FF); // Vmax = 2047
  rw_spi(fd, 0, SMDA_MOTOR0, ADDR_AMAX, 0, 0x0007FF); // Amax = 2047
  rw_spi(fd, 0, SMDA_MOTOR0, ADDR_REFCONF_RM, 0,
         0x000F02); // NO_REF = 0x0f, RM_VELOCITY = 0x02

  // Start the motor moving.
  rw_spi(fd, 0, SMDA_MOTOR0, ADDR_VTARGET, 0,
         0x00008F); // Set the target velocity.
  data = rw_spi(fd, 0, SMDA_MOTOR0, ADDR_VTARGET, 1,
                0x00008F); // Check that the target velocity was set.
}
