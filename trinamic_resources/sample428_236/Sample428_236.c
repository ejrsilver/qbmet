/***********************************************************
        Sample program for initializing the TMC428
		          when using it with the TMC236.

  Attention: Controller-specific code has been left out 
	           here. These functions have to be filled
						 in by the user (especially SPI interface).

   Copyright (C) 2005 TRINAMIC Motion Control GmbH & Co. KG
                      Sternstrasse 67
                      D - 20357 Hamburg, Germany
                      http://www.trinamic.com/
   
   This program is free software; you can redistribute it and/or modify it 
   under the terms of the GNU General Public License as published by the 
   Free Software Foundation; either version 2 of the License, or (at your 
   option) any later version.
   
   This program is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
   or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
   for more details.
   
   You should have received a copy of the GNU General Public License along 
   with this program; if not, write to the Free Software Foundation, Inc., 
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
************************************************************/


#define SELECT_428()      //FILL IN HERE: Set the  nCS line of the TMC428 to low.
#define DESELECT_428()    //FILL IN HERE: Set the  nCS line of the TMC428 to high.

#include "sample428_236.h"   //Header file containing the register names of the TMC428

//Some sample values for acceleration and velocity
#define MOT0_AMAX  500
#define MOT0_VMAX  1200

#define MOT1_AMAX  50
#define MOT1_VMAX  2047

#define MOT2_VMAX  2047


//The following array contains:
//  -the first 64 bytes: configuration data of the TMC428 containing 
//   the structure of the SPI telegrams that are to be sent to the
//   drivers by the TMC428 (in this example: three TMC236).
//  
//  -the second 64 bytes: microstep wave table
//   (sine wave, 1/4 period).
UCHAR driver_table[128]=
{
0x10, 0x05, 0x04, 0x03, 0x02, 0x06, 0x10, 0x0D, 0x0C, 0x0B, 0x0A, 0x2E, 0x10, 0x05, 0x04, 0x03,
0x02, 0x06, 0x10, 0x0D, 0x0C, 0x0B, 0x0A, 0x2E, 0x10, 0x05, 0x04, 0x03, 0x02, 0x06, 0x10, 0x0D,
0x0C, 0x0B, 0x0A, 0x2E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0B, 0x0C, 0x0E, 0x10, 0x11, 0x13, 0x14, 0x16, 0x17,
0x18, 0x1A, 0x1B, 0x1D, 0x1E, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x27, 0x29, 0x2A, 0x2B, 0x2C,
0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x38, 0x39, 0x3A, 0x3B,
0x3B, 0x3C, 0x3C, 0x3D, 0x3D, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F
};



/**************************************************
  Function: UCHAR read_write_spi(UCHAR writebyte)
  Purpose: Write one byte to the SPI interface 
	         and read back one byte.
  Parameter: writebyte: byte to be written.
  Return value: byte that has been read back.
***************************************************/
UCHAR read_write_spi(UCHAR writebyte)
{
	//This function is controller specific and has to to
	//the following things:

  // Send the contents of "writebyte" to the TCM428 via the SPI interface
  // (MSB first!!!!!!!!!!!).

  // Return the byte that has been written back from the TMC428.
}



/*****************************************************************
  Function:  Send428
  Purpose: Send four Bytes to the TMC428
  Parameter:    a,b,c,d: the bytes that are to be written
*****************************************************************/ 
void Send428(UCHAR a, UCHAR b, UCHAR c, UCHAR d)
{
  SELECT_428();			//Set the nCS line of the TMC428 low (controller specific).
  read_write_spi(a);
  read_write_spi(b);
  read_write_spi(c);
  read_write_spi(d);
  DESELECT_428();	    //Set the nCS line of the TMC428 high (controller specific).
}


/*************************************************************
  Function:  Send428
  Purpose: Initialize the TMC428
**************************************************************/ 
void Init428(void)
{
  int i;
  
  //Write the driver configuration data to the TMC428.
  //This is the most important part of the intialization and should be done first of all!
  for(i=128; i<256; i+=2)
    Send428(i, 0, driver_table[i-127], driver_table[i-128]);

  //Set the "Stepper Motor Global Parameter"-Registers (SMGP).
  //Set SPI_CONTINUOS_UPDATE  and PoFD, SPI_CLKDIV=7
  //      LSMD = 2 (which means 3 motors!)
  Send428(IDX_SMGP, 0x01, 0x07, 0x22);


  //Set the coil current parameters (which is application specific)
  Send428(IDX_AGTAT_ALEAT|MOTOR0, 0x00, 0x10, 0x40); //i_s_agtat/i_s_aleat/i_s_v0/a_threshold
  Send428(IDX_AGTAT_ALEAT|MOTOR1, 0x00, 0x10, 0x40);
  Send428(IDX_AGTAT_ALEAT|MOTOR2, 0x00, 0x10, 0x40);

  //Set the pre-dividers and the microstep resolution (which is also application and motor specific)
  Send428(IDX_PULSEDIV_RAMPDIV|MOTOR0, 0x00, 0x55, 0x04); //pulsdiv/rampdiv/µStep
  Send428(IDX_PULSEDIV_RAMPDIV|MOTOR1, 0x00, 0x55, 0x04);
  Send428(IDX_PULSEDIV_RAMPDIV|MOTOR2, 0x00, 0x55, 0x04);

 
	//Now some examples of the driving parameters. This is highly application specific!

  //Example for motor #0: RAMP mode
  Send428(IDX_VMIN|MOTOR0, 0, 0, 1);  //Vmin = 1
  Send428(IDX_VMAX|MOTOR0, 0, MOT0_VMAX >> 8, MOT0_VMAX & 0xff);  //VMax = MOT0_VMAX
  Send428(IDX_AMAX|MOTOR0, 0, MOT0_AMAX >> 8, MOT0_AMAX & 0xff);  //AMax = MOT0_AMAX
  Send428(IDX_PMUL_PDIV|MOTOR0, 0, 150, 1);
  Send428(IDX_REFCONF_RM|MOTOR0, 0, NO_REF, RM_RAMP);  //RAMP mode, stop switches not active

  //Motor #0 can now be driven by setting the XTarget register (IDX_XTARGET) to the desired end position.
	//ATTENTION: The values in IDX_PMUL_PDIV depend on the AMax and Pulsediv/Rampdiv values and have to be
	//calculated using the algorithm given in the data sheet.
	//The values given here have been calculated using the Calc428.exe program on the TechLibCD.

  //Example for motor #1: VELOCITY mode
  Send428(IDX_VMIN|MOTOR1, 0, 0, 1);  //Vmin = 1
  Send428(IDX_VMAX|MOTOR1, 0, MOT1_VMAX >> 8, MOT1_VMAX & 0xff);  //VMax = MOT1_VMAX (s. oben)
  Send428(IDX_AMAX|MOTOR1, 0, MOT1_AMAX >> 8, MOT1_AMAX & 0xff);  //AMax = MOT1_AMAX
  Send428(IDX_REFCONF_RM|MOTOR1, 0, NO_REF, RM_VELOCITY);  //VELOCITY mode, no stop switches

  //Motor #1 can now be driven by setting the VTarget register (IDX_VTARGET) to the desired velocity.
	//The AMax value will be used to accelerate or decelerate the motor.
  
  //Example for motor #2: HOLD mode
  Send428(IDX_VMIN|MOTOR2, 0, 0, 1);  //Vmin = 1
  Send428(IDX_VMAX|MOTOR2, 0, MOT2_VMAX >> 8, MOT2_VMAX & 0xff);  //VMax = MOT2_VMAX (s. oben)
  Send428(IDX_REFCONF_RM|MOTOR2, 0, NO_REF, RM_HOLD);

  //Der Motor 2 kann nun verfahren werden, indem das Register VActual (IDX_VACTUAL) auf die gewünschte
  //Geschwindigkeit gesetzt wird. Diese Geschwindigkeit ist dann sofort wirksam, es wird nicht
  //beschleunigt bzw. abgebremst.

	//Motor #2 can now be driven by setting the VActual register (IDX_VACTUAL) to the desired value.
	//The motor will then uses that velocity immediately, there is no accelerating or decelerating.
}
