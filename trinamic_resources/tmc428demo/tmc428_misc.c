/* ***************************************************************************

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
  
   *************************************************************************** */

/* ***************************************************************************

   File        : tmc428_misc.c
   
   Version     : 1.0 /    July 12, 2005 
                 2.0 / October 14, 2007 : StallGuard Profiler added

   History     :      July 12, 2005 : initial version
                 September 30, 2007 : StallGuard profiler(s) added, routines 
				                      added here for the StallGuard Profiler: 
									  tmc428_stallguard_demo(), 
									  tmc428_get_tmc24x_stallguard_bits_n_times(),
									  tmc428_stallguard_profiler() 
                   October  7, 2007 : tmc428spi4win_id_string_request() added
				   October 14, 2007 : StallGuard profiler routines completed
				   October 21, 2007 : calculation of sufficient motion range 
				                      for StallGuard profiler in RAMP_MODE 
									  added
  
   Description : some additional routines for the TMC428DEMO
                                                                                                 
   Requirement : 

   *************************************************************************** */


#include <windows.h>

#include <accctrl.h>
#include <winbase.h>

#include <stdio.h>
#include <conio.h>
#include <math.h>

#include "rs232win.h"
#include "tmc428_base.h"
#include "tmc428_misc.h"


void dump_tmc428_ram_tab(unsigned char *ram_tab)
{
	int i, j; 
	
	printf("\n--------------- TMC428 RAM Table ----------------\n\n"); 
   
	for(i=0; i<8;i++)
	{
		for (j=0;j<16;j++) printf(" %02x", ram_tab[16*i+ j]); 
		printf("\n"); 
	}

    printf("\n-------------------------------------------------\n\n"); 

}

float r_fs_hz( float f_clk, int v, int pulse_div, int usrs )
{
	return (float)( f_clk * ((float)v) / pow(2.0,pulse_div) / 2048.0 / 32.0 / pow(2.0,usrs) ); 
}


/* *************************************************************************** */

// unsigned char tmc428_ram_tab_test[128] = // RAM table for TMC428 configuration RAM access test
// {
//	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
//	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
//	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
//	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
//
//	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
//	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
//	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
//	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
// };

/* *************************************************************************** */


void tmc428spi4win_id_string_request( HANDLE comHANDLE ) 
{
	char c, s[80];  int i=0; 

	//putc_rs232( comHANDLE, 0x23 ); // re-initialize TMC428 Eval Board (TBD)
	//putc_rs232( comHANDLE, 0x23 ); // (if tmc428 was terminated by ^C)
	//putc_rs232( comHANDLE, 0x23 ); // send 0x23 until four characters are send back
	//putc_rs232( comHANDLE, 0x23 ); //
    //putc_rs232( comHANDLE, 0x23 ); //

	putc_rs232( comHANDLE, 0xFF );   // ask for TMC428 Evaluation Board identifier string

	// *s = NULL; 
	i = 0; 

	for (i=0;i<10;i++) 
	{
		s[i] = c = getc_rs232( comHANDLE ); 
	 // fprintf(stderr, "i = %3d : %3d : %c\n",i,c,c); 
	} 

	s[10] = '\0'; 

 	fprintf(stderr, "\nTMC428 Eval Board ID string = %s\n\n",s); 

 // return ( s ); 
}

/* *************************************************************************** */

void tmc428_stallguard_demo( int sm, int vmax, int sgl ) // 'sgl' = StallGuard Level
{
   // run the TMC428 in velocity mode (VELOCITY_MODE) ...

	fprintf(stderr,"\nStallGuard Demo : Now run in VELOCITY_MODE ...\n"); 
    fprintf(stderr,"\nStallGuard Level set by '-sgl' to %d\n\n",sgl); 
	fprintf(stderr,"*** (Hit any key to break) ***\n\n"); 

    tmc428_set_refconf_rm( sm, 0x3, VELOCITY_MODE ); // set VELOCITY_MODE
	tmc428_set_vtarget( sm,    0 );                  // set v_target = vmax, and now run the StallGuard Demo
	tmc428_set_vtarget( sm, vmax );                  // set v_target = vmax, and now run the StallGuard Demo
    tmc428_set_xactual( sm, 0);                      // zero the actual position
    
	// tmc428_get_lsmd( &lsmd );
    // fprintf(stderr,"\nLSMD = %d\n",lsmd); 

	while (!_kbhit()) 
	{
		int ld,ld_i, x_actual=0; 
		//int sm0_status, sm1_status, sm2_status; 
		//long int datagram_low_word, datagram_high_word; 

	    //tmc428_get_datagram_words( &datagram_low_word, &datagram_high_word );
		//fprintf(stderr,"l = %4x  h = %4x\n", datagram_low_word,  datagram_high_word); 

		// tmc428_get_tmc24x_status_bits( &sm0_status, &sm1_status, &sm2_status ); 
		// fprintf(stderr,"%4x %4x %4x   : ",sm0_status, sm1_status, sm2_status); 
		tmc428_get_xactual( sm, &x_actual ); 
		tmc428_get_tmc24x_stallguard_bits( sm, &ld );
		fprintf(stderr,"LD = %d : ",ld); 
		fprintf(stderr,"*"); 
		for (ld_i=0;ld_i<ld;ld_i++)   fprintf(stderr,"*"); 
		for (ld_i=ld_i;ld_i<8;ld_i++) fprintf(stderr," "); 
		if (ld < sgl) fprintf(stderr,"  StallGuard @ %6d",x_actual); // delay due to RS232
		fprintf(stderr,"\n");  
	}
	_getch(); // remove character from buffer
	tmc428_set_vtarget( sm, 0); // stop the motor and ...
	tmc428_set_xtarget( sm, 0); // zero the target position
	tmc428_set_xactual( sm, 0); // zero the actual position
}

void tmc428_get_tmc24x_stallguard_bits_n_times(int sm, int n, double *x_mean, double *x_sigm)
{
	double x=0.0, xx=0.0, xmean=0.0, xvari=0.0, xsigm=0.0; 
	int i, ld; 

	for ( i=0; i<n; i++ ) // read n times the StallGuard LD bits
	{
	    tmc428_get_tmc24x_stallguard_bits(sm, &ld); 

		// (a-b)^2 = a^2 - 2ab + b^2
		// x_mean  = 1/n * sum(x_i)
		// x_vari  = sum((x_mean-x_i)^2)
		// x_sigma = sqrt(x_vari)

        x  += (float)   (ld); 
		xx += (float)(ld*ld); 
			
		Sleep(1); 
	}

	xmean = x / ((float)n); 
	xvari =     ( ( n * xx ) - ( x * x ) ) / ( (float)(n*(n-1)) ); 
	xsigm = sqrt( ( n * xx ) - ( x * x ) ) / ( (float)(n*(n-1)) ); 

    *x_mean = xmean; 
	*x_sigm = xsigm; 
}

void tmc428_stallguard_profiler( int sm, int v_target_min, int v_target_max, int v_step, int x_min, int x_max )
{
 // tmc428_stallguard_profiler() expects 'pmul, pdiv, a_max, etc. ' to be initialized properly

	int   n, x_actual, v_target, v_actual, a_actual; 
	double x=0.0, xx=0.0, x_mean=0.0, x_sigm=0.0; 

	n = 10; // number of StallGuard load indicator (LD) bit vector measurements

    // GetUserDefaultUILanguage(); // to handle '.' vs. ',' output for floats

	if (x_min==x_max) fprintf(stderr,"\nTMC428 Demo StallGuard profiler started in VELOCITY_MODE\n"); 
	else              fprintf(stderr,"\nTMC428 Demo StallGuard profiler started in RAMP_MODE\n"
		                               "       moving between position x_min and x_max\n");

	                  fprintf(stderr,  "       with : vmin = %d, vmax = %d, vstp = %d, xmin = %d, xmax = %d\n\n", 
		                                              v_target_min, v_target_max, v_step, x_min, x_max); 

                      fprintf(stderr,  "*** (Hit ESC key to break) ***\n\n"); 

					  fprintf(stdout,"\n\n#   v   v_mean  v_sigma\n");

 /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - - - */

 // now do the StallGuard profiling ...

 /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	if ( x_min == x_max ) // run StallGuard profiler in VELOCITY_MODE ...
	{
		tmc428_set_refconf_rm( sm, 0x3, VELOCITY_MODE );  // ref_conf = 0x3 <=> disaple stop SW, select VELOCITY_MODE
		if (v_step==0) v_step = 1; 

		for ( v_target = v_target_min; v_target <= v_target_max; v_target += v_step ) 
		{
			tmc428_set_vtarget( sm, v_target );  Sleep(1); 

			do // wait until v_target is reached
			{
				tmc428_get_vactual( sm, &v_actual ); 
			}
			while ( (v_actual != v_target) ); 

			Sleep(10); // wait wait until internal damping of the stepper finished

		 // get n times the load indocator bits and calculate mean value & standard deviation
			tmc428_get_tmc24x_stallguard_bits_n_times(sm, n, &x_mean, &x_sigm); 

			fprintf(stdout," %6.1f\t%5.3f\t%5.3f\n",(float)v_target,x_mean,x_sigm);   // v ld_mean ld_sigma

		  //fprintf(stdout," %5.1f;%5.3f;%5.3f\n",(float)v_target,x_mean,x_vari);     // v ld_mean ld_sigma (CSV format)
		  //printf with '\t' worked for Win2K cmd.exe but not for WinXP because TABs are filled up by ' '.
		  //sprintf(s,"...",...) s[i++]=(s[i]=='.')?',':s[i]; printf(s); // declare : char s[255]; 

		  if (_kbhit()) { if (_getch()==0x1B) break; } // 0x1B = ESC
		}

		tmc428_set_vtarget( sm, 0); // motor stop
		tmc428_set_xtarget( sm, 0); // zero the target position
		tmc428_set_xactual( sm, 0); // zero the actual position
	}
	else // ( x_min != x_max ) : run StallGuard profiler in RAMP_MODE moving between position x_min and position x_max ...
	{
		//int sm, int v_target_min, int v_target_max, int v_step, int x_min, int x_max

        tmc428_set_refconf_rm( sm, 0x3, RAMP_MODE );  // ref_conf = 0x3 <=> disaple stop SW, select RAMP_MODE

		for ( v_target = v_target_min; v_target <= v_target_max; v_target += v_step ) 
		{
            int a_max, pulse_div, ramp_div, usrs, ramp_usteps, ramp_steps; 
			int required_StallGuard_FS_motion_range; 

			tmc428_set_vmin(sm, 1);         // set v_min always to '1'
			tmc428_set_vmax(sm, v_target);  // set profiling velocity
			tmc428_set_xtarget(sm,x_min);   // move to start position

			do // wait until start position is reached
			{
				tmc428_get_xactual(sm, &x_actual); // alternativly check status bit xEQt{sm} of associated stepper {sm}
			} while (x_min!=x_actual); 

		 // Hint : the target velocity might not be reached if x_min is to close to x_max, 
		 // so first calculate the numer of microsteps steps during acceleration : 
         //
		 //		S = 1/2 * velocity^2 / a_max * 2^ramp_div / 2^pulse_div / 2^3 
		 //		(pls. refer TMC428 datasheet, section 'pmul & pdiv & usrs)

			tmc428_get_pdrdus( sm, &pulse_div, &ramp_div, &usrs ); 
			tmc428_get_amax( sm, &a_max ); 
			CalcRampSteps( v_target, a_max, pulse_div, ramp_div, usrs, &ramp_usteps, &ramp_steps ); 
            // fprintf(stdout,"ramp_usteps = %5d  ramp_steps = %5d\n", ramp_usteps, ramp_steps); 

            required_StallGuard_FS_motion_range = n*( abs(x_max-x_min) / ((int)pow(2.0,usrs)) ) - 2*ramp_steps; 

			if ( required_StallGuard_FS_motion_range > 0 ) // run StallGuard profiling only for v_target 
				                                           // if the motion range is sufficient large
			{
				tmc428_set_xtarget(sm,x_max); // move to end position and measure TMC246/TMC249 
				                              // load indicator bits during motion

				do // wait until acceleration phase is finished
				{  
					tmc428_get_aactual( sm, &a_actual ); 
				}
				while ( a_actual != 0 ); 

				Sleep(10); // wait wait until internal damping of the stepper finished

			// get n times the load indicator bits and calculate mean value & standard deviation, for
			// short motion range it might me necessary to get the StallGuard bits only once per motion
				tmc428_get_tmc24x_stallguard_bits_n_times(sm, n, &x_mean, &x_sigm); 

				fprintf(stdout," %6.1f\t%5.3f\t%5.3f\n",(float)v_target,x_mean,x_sigm);   // v ld_mean ld_sigma
			}
			else // skip StallGuard profiling for v_target because motion range is to small
			{
				fprintf(stdout," %6.1f %6.1f : skipped due to insufficient short motion range.\n",(float)v_target,0); 
			}

            if (_kbhit()) { if (_getch()==0x1B) break; } // 0x1B = ESC
		}
	}

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	fprintf(stderr,"\nTMC428 Demo StallGuard profiler finished.\n"); 
}
