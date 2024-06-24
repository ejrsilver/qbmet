/* **************************************************************************

   Copyright (C) 2007 TRINAMIC Motion Control GmbH & Co. KG
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
  
   ************************************************************************** */

/* **************************************************************************

   File        : tmc428demo.c    
   
   Version     : 1.0 / December 12, 2002
				 2.0 /  October 14, 2007 : StallGuard Profiler(s) version

   History     :      July 12, 2005 : company address updated, routines for 
                                      full initialization of the TMC428 added
				 September 30, 2007 : StallGuard Profiler(s) added, -com added, 
				                      pls. refer 'tmc428_misc.c' for details, 
									  the StallGuard Profiler demonstrates 
									  how to read out the TRINAMIC stepper 
									  motor diagnosis bits with the TMC428 
									  by reading the datagram_low_word and 
									  datagram_high_word
				   October 14, 2007 : Version 2.0 completed

                                                                                
   Description : Example, how to use the tmc428spi, tmc428spi4win resp.         
                 tmc428spi4avr routines to control the TMC428.
				 
                 It is an console application without GUI. Execute the 
				 cmd.exe command shell from the Win Start Menu. Then go 
				 to the directory path where the 'tmc428demo.exe' is. 
				 Then you can run the tmc428demo with different command
				 line options. 
				 
				 For example, the command line 

				      tmc428demo -s 1 -a 100 -v 200 -x 10000 

                 drives the stepper motor 0 with an acceleration of 100 
				 and with a velocity of 200 to position 10000. 

  
				 To run a simple StallGuard demo with e.g. a_max = 100
				 just enter a speed by -v and a StallGuard level by -sgl

					  tmc428demo -s 1 -a 100 -v 500 -sgl 3


				 To run the StallGuard profiler in VELOCITY_MODE with e.g. 
				 a_max = 100 from v_target = 100 to v_target = 1000 with 
				 velocity step increments of width 50 enter

					  tmc428demo -s 1 -a 100 -sgp 100 1000 50 0 0


				 To run the StallGuard profiler in RAMP_MODE with e.g. 
				 a_max = 1000 from v_target = 100 to v_target = 1000 with 
				 velocity step increments of width 50 between position 
				 x_min -2000 and x_max 2000

					  tmc428demo -s 1 -a 1000 -sgp 100 1000 50 -2000 2000

                 Within this example - running the StallGuard profiler in 
				 RAMP_MODE - a higher acceleration a_max is required to 
				 reach the target velocity within the limited motion range 
				 between position x_min -2000 and x_max 2000. 


				 For unit of these motion parameters please refer the 
				 TMC428 datasheet and spread sheet 'tmc428_frequencies.xls'
				 that is available on the TECHlibCD and www.trinamic.com
  
                 This application directly controls TMC428 Evaluation Kits. 
         
		         Please take into account that many other important TMC428 
				 parameters are initialized by TMC428 Eval-Board Firmware 
				 (also available as C source on the TECHlibCD) on power-up. 

                 Please refer to the application note entitled 
					"TMC428 - Getting Started" that is available on the 
				 TRINAMIC TECHlibCD and www.trinamic.com

   ************************************************************************* */


#include <windows.h>

#include <accctrl.h>
#include <winbase.h>

#include "rs232win.h"

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

#include <math.h>


#include "tmc428spi4win.h"
#include "tmc428_base.h"

#include "tmc428_misc.h"

#include "tmc428spi.h"


#define TRUE 1
#define FALSE 0

#define MAXLEN 1000

#define PI2 6.283185307
#define PI  3.141592653


HANDLE GLOBAL_ComHandle = NULL;


/* ------------------------------------------------------------------------- */

void header(void)
{
     fprintf(stderr," tmc428demo - A TMC428 demo (runs with TMC428-EvalKit)\n");
     fprintf(stderr,"              ( Version : %s / %s )\n",__DATE__,__TIME__);
}

void usage(void)
{
    fprintf(stderr,"\n");
    header();
    fprintf(stderr,"\n");

	fprintf(stderr,
	" USAGE:    tmc428demo {-option <parameter>} > <output file>                 \n"
	"                                                                            \n"
	" EXAMPLES:                                                                  \n"
	"                                                                            \n"
	"  tmc428demo -s 1 -a 100 -v 500 -x 10000 (RAMP_MODE)                        \n"
	"                                                                            \n"
	"  tmc428demo -s 1 -a 100 -v 500 -sgl 5                (StallGuard Demo)     \n"
	"                                                                            \n"
    "  tmc428demo -s 1 -a 100 -sgp 100 1000 50 0 0         (StallGuard Profiler) \n"
	"  tmc428demo -s 1 -a 1000 -sgp 100 1000 50 -2000 2000 (StallGuard Profiler) \n"
	"                                                                            \n"
	" OPTIONS:  -s {0,1,2}      : stepper motor number                           \n"
	"           -a <file name>  : acceleration parameter a_max                   \n"
	"           -v <file name>  : velocity parameter v_max                       \n"
	"           -x <file name>  : target position x_target                       \n"
	"           -com {1,2,..}   : set COM1:, COM2:, ... (default = COM1:)        \n"
	"           -f_clk <f>      : clock frequency in Hz                          \n"
	"                                                                            \n"
	"                                                                            \n"
	);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// void tmc428demo_ramp_mode_run(...)     // run positioning to x_target (TBD)
// void tmc428demo_velocity_mode_run(...) // run with velocity v_target  (TBD)

/* ------------------------------------------------------------------------- */

void main(int argc, char **argv)
{
	// long int spi, spo; // 32 bit SPI datagramms 

	HANDLE ComHandle = NULL; 

	char **argp; 
	int  ch=0; 

    double PIdeal, PBest; 
	int pulse_div, ramp_div, pmul, pdiv, usrs; 
	int c=1, s, a, v, x, sm, v_min, v_max, a_max, x_target, sgl=9, sgp=0, sgpx=0, vmin=1, vmax=2047, vstp=1, xmin=0, xmax=0; 

	float f_clk=8000000.0, fs_per_s=0.0; // f_clk = 8 MHz for the TMC428 Eval Board

	int lsmd=3; 

    // int mot1r, refmux, cont_updt, clk2_div, cs_comind, dac, fd, ph, sck, nscs, lsmd; 


	int com=1, error_code=-1, baud=19200; // parameters for the RS232 of the PC


	unsigned char tmc428_ram_tab[128] = // RAM table according to example within TMC428 data sheet
	{
		0x10, 0x05, 0x04, 0x03, 0x02, 0x06, 0x10, 0x0D, 0x0C, 0x0B, 0x0A, 0x2E, 0x10, 0x05, 0x04, 0x03,
		0x02, 0x06, 0x10, 0x0D, 0x0C, 0x0B, 0x0A, 0x2E, 0x07, 0x05, 0x04, 0x03, 0x02, 0x06, 0x0f, 0x0D,
		0x0C, 0x0B, 0x0A, 0x2E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
		0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,

		0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0B, 0x0C, 0x0E, 0x10, 0x11, 0x13, 0x14, 0x16, 0x17,
		0x18, 0x1A, 0x1B, 0x1D, 0x1E, 0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x27, 0x29, 0x2A, 0x2B, 0x2C,
		0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x38, 0x39, 0x3A, 0x3B,
		0x3B, 0x3C, 0x3C, 0x3D, 0x3D, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F
	};


    s = a = v = x = v_min = v_max = a_max = x_target = 0; 

	vmin = vmax = vstp = xmin = xmax = 0; 
	

// 1st scan the input parameters ------------------------------------------- 

	if (argc>1) 
	{
		while (argv++, argc--)
		{
			argp = argv + 1;   if (*argp==NULL) break;

			if      ( (!strcmp(*argv,"-s")) )     { sscanf(*argp,"%d",&s); }
			else if ( (!strcmp(*argv,"-a")) )     { sscanf(*argp,"%d",&a); }
			else if ( (!strcmp(*argv,"-v")) )     { sscanf(*argp,"%d",&v); }
			else if ( (!strcmp(*argv,"-x")) )     { sscanf(*argp,"%d",&x); }
			else if ( (!strcmp(*argv,"-sgl")) )   { sscanf(*argp,"%d",&sgl); }  // StallGuard level 'sgl' for interactive mode
		    else if ( (!strcmp(*argv,"-sgp")) )   
					{ sgp=1; 
					  if (argc>5)   sscanf(*(argp+0),"%d",&vmin); 
					  if (argc>5) { sscanf(*(argp+1),"%d",&vmax); v=vmax; }     // sets v resp. overwrites -v
					  if (argc>5)   sscanf(*(argp+2),"%d",&vstp); 
                      if (argc>5)   sscanf(*(argp+3),"%d",&xmin); 
                      if (argc>5)   sscanf(*(argp+4),"%d",&xmax); 
					}
			else if ( (!strcmp(*argv,"-com")) )   { sscanf(*argp,"%d",&c);     }
			else if ( (!strcmp(*argv,"-f_clk")) ) { sscanf(*argp,"%f",&f_clk); }
	    }
     }
	else { usage();  exit(1); } 

	fprintf(stderr,"\ntmc428demo : ( Compiled : %s / %s )\n",__DATE__,__TIME__); 


 // 2nd open the PC's RS232 interface COM<c>: with 19200 baud with 1N8 --------

	com = c; 

	ComHandle = open_rs232( com, baud, &error_code );
	GLOBAL_ComHandle = ComHandle; // :-( global parameter, simplifies the usage of the RS232 routines

	if (error_code==-1){fprintf(stderr,"\nERROR : COM%d: could not be opend.\n\n",com); exit(-1);}
	else if (!error_code) ;// fprintf(stderr,"\nCOM%d: open successfull (1N8, %d baud).\n\n",com,baud);


	tmc428spi4win_id_string_request( ComHandle );   // detect the TMC428 evaluation board (sends a string back)

    
 // 3rd control the TMC428 at the TMC428 EvalKit with your own software -----

	write_tmc428_configuration_ram(tmc428_ram_tab); // write the TCM428 configuration RAM

    read_tmc428_configuration_ram(tmc428_ram_tab);  // read back RAM table
    dump_tmc428_ram_tab(tmc428_ram_tab);            // dump RAM table


    (void) tmc428spi( 0x7E010701 );                 // initialize stepper motor global parameter register 
	                                                // with LSMD=1 for TMC428 evaluation kit
  
    pulse_div = 3;                                  // set step pulse pre-divider
	ramp_div  = 6;                                  // set acceleration pre-divider
	                                                // a choise where pulse_div == ramp_div should be prefered
    
    usrs      = 4;                                  // set highest 16 times micro step resolution
    

    sm = s;                                                 // select stepper motor (SMDA)

	tmc428_set_pdrdus( sm,  pulse_div,  ramp_div,  usrs );  // set pulse_div, ramp_div, usrs for stepper SMDA
    tmc428_set_i_scales_a_threshold(sm, 0, 0, 1, 1);        // set automatic down scaling of current at rest

    v_min = 1; 
    v_max = v; 
    
    a_max = a;
    
    x_target = x;

   /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

   tmc428_get_pdrdus( sm, &pulse_div, &ramp_div, &usrs );                   // get required parameters
   CalcPMulPDiv(a_max, ramp_div, pulse_div, &pmul, &pdiv, &PIdeal, &PBest); // calculate pmul & pdiv
   tmc428_set_pmulpdiv( sm, pmul, pdiv );                                   // set pmul & pdiv 

   tmc428_set_refconf_rm( sm, 0x3, RAMP_MODE );                             // ref_conf = disaple stop SW, 
                                                                            // select RAMP_MODE

   fs_per_s = r_fs_hz(f_clk, v_max, pulse_div, usrs ); 

   fprintf(stdout, "\n"
		           "        sm = %8d\n"
		           "\n"
				   "     v_min = %8d\n" 
				   "     v_max = %8d  (= %6.3f [FS/s] @ %6.3f MHz)\n\n" 

				   "     a_max = %8d\n\n"

				   " pulse_div = %8d\n"
				   " ramp_div  = %8d\n\n" 
				   " usrs      = %8d\n\n" 

				   "     p_mul = %8d\n" 
				   "     p_div = %8d\n\n" 

				   "  x_target = %8d\n\n",
					
				   sm, v_min, v_max, fs_per_s, f_clk/1000000.0, 
				   a_max, pulse_div, ramp_div, usrs, pmul, pdiv, x_target );
			

 // now 1st run in position mode (RAMP_MODE) ...

    tmc428_set_vmin( sm, v_min );        // vmin = 1
    tmc428_set_vmax( sm, v_max );        // set desired velocity limit
  
    tmc428_set_amax( sm, a_max );        // set a_max (p_mul, p_div, usrs)


    tmc428_set_xtarget( sm, x_target );  // now do the positioning



 // now run the StallGuard demo if the StallGuard Level 'sgl' is set lower then 9
	if (sgl<9) tmc428_stallguard_demo( sm, v_max, sgl ); 



 // now run the StallGuard profiler if the command line option -sgp is set ...
	if (sgp==1) tmc428_stallguard_profiler( sm, vmin, v_max, vstp, xmin, xmax ); 

    
// ************************************************************************
// *** IMPORTANT HINT : TMC428 parameters are initialized by the TMC428 ***
// ***                  evaluation board Firmware after power-on reset, ***
// ***                  but this code does its own initialization       ***
// ************************************************************************
 
   
//  3rd close RS232 interface and terminate the program ---------------------

	close_rs232( ComHandle ); 
}

/* *************************************************************************** */

