/*
* Author: Ethan Silver
*
* Constants useful for addressing the TMC2130.
*
* Refer to TMC2130 Datasheet Rev 1.16 pages 24-31.
*/
#define WRITE 1
#define READ 0

// General configuration registers.
#define ADDR_GCONF 0x0
#define ADDR_GSTAT 0x1
#define ADDR_IOIN 0x4

// Velocity Dependent registers.
#define ADDR_IHOLD_IRUN 0x10
#define ADDR_TPOWER_DOWN 0x11
#define ADDR_TSTEP 0x12
#define ADDR_TPWNTHRS 0x13
#define ADDR_TCOOLTHRS 0x14
#define ADDR_THIGH 0x15

// SPI Mode register.
#define ADDR_XDIRECT 0x2D

// DcStep minumum velocity register.
#define ADDR_VDCMIN 0x33

// Motor driver registers.
#define ADDR_MSLUT0 0x60
#define ADDR_MSLUTSEL 0x68
#define ADDR_MSLUTSTART 0x69
#define ADDR_MSCNT 0x6A
#define ADDR_MSCURACT 0x6B
#define ADDR_CHOPCONF 0x6C
#define ADDR_COOLCONF 0x6D
#define ADDR_DCCTRL 0x6E
#define ADDR_DRV_STATUS 0x6F
#define ADDR_PWMCONF 0x70
#define ADDR_PWM_SCALE 0x71
#define ADDR_ENCM_CTRL 0x72
#define ADDR_LOST_STEPS 0x73
