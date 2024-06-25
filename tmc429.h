// All the motor addresses are shifted by five to account for IDX and RW bits.
#define MOTOR0 0
#define MOTOR1 0x20
#define MOTOR2 0x40

// All the addresses are shifted by one so that the RW bit is always 0 (allow write).
#define IDX_XTARGET 0x00
#define IDX_XACTUAL 0x02
#define IDX_VMIN 0x04
#define IDX_VMAX 0x06
#define IDX_VTARGET 0x08
#define IDX_VACTUAL 0x0A
#define IDX_AMAX 0x0C
#define IDX_AACTUAL 0x0E
#define IDX_AGTAT_ALEAT 0x10
#define IDX_PMUL_PDIV 0x12
#define IDX_REFCONF_RM 0x14
#define IDX_IMASK_IFLAGS 0x16
#define IDX_PULSEDIV_RAMPDIV 0x18
#define IDX_DX_REFTOLERANCE 0x1A
#define IDX_XLATCHED 0x1C

#define IDX_SMGP 0x7e

#define READ_REG 0x01

#define RM_RAMP 0
#define RM_SOFT 1
#define RM_VELOCITY 2
#define RM_HOLD 3

#define NO_REF 0x0f
