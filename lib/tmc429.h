// SMDA.
#define SMDA_MOTOR0 0x0
#define SMDA_MOTOR1 0x1
#define SMDA_MOTOR2 0x2
#define SMDA_OTHER 0x3

// Three stepper motor register set adresses.
#define ADDR_XTARGET 0x0
#define ADDR_XACTUAL 0x1
#define ADDR_VMIN 0x2
#define ADDR_VMAX 0x3
#define ADDR_VTARGET 0x4
#define ADDR_VACTUAL 0x5
#define ADDR_AMAX 0x6
#define ADDR_AACTUAL 0x7
#define ADDR_THRESHOLD 0x8
#define ADDR_PMUL_PDIV 0x9
#define ADDR_REFCONF_RM 0xA
#define ADDR_INTERRUPT 0xB
#define ADDR_PULSEDIV_RAMPDIV 0xC
#define ADDR_DX_REFTOLERANCE 0xD
#define ADDR_XLATCHED 0xE
#define ADDR_USTEPCOUNT 0xF

// Common register addresses.
#define ADDR_DATAGRAM_LOW 0x0
#define ADDR_DATAGRAM_HIGH 0x1
#define ADDR_COVER_POSITION_LEN 0x2
#define ADDR_COVER_DATAGRAM 0x3
#define ADDR_IF_CONFIGURATION 0x4
#define ADDR_POS_COMP 0x5
#define ADDR_POS_COMP_INT 0x6
#define ADDR_POWER_DOWN 0x8
#define ADDR_TYPE_VERSION 0x9
#define ADDR_LEFT_RIGHT 0xE
#define ADDR_SMGP 0xF

// Other.
#define RM_RAMP 0x0
#define RM_SOFT 0x1
#define RM_VELOCITY 0x2
#define RM_HOLD 0x3
#define NO_REF 0x0f
