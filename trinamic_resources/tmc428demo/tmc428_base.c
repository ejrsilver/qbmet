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

   File        : tmc428_base.c
   
   Version     : 1.0 / December 12, 2002
				 2.0 /  October 14, 2007

   History     :      June 21, 2005 : company address updated                                                          
                      July 12, 2005 : include file names updated, routines added
				      July 24, 2007 : tmc428_get_xactual() signed position corrected
				      July 24, 2007 : tmc428_get_datagram_words() added
				 September 30, 2007 : required StallGuard profiler routines added: 
									  CalcRampSteps(), 
									  tmc428_get_xtarget(),  
									  tmc428_set_xactual(), tmc428_get_xactual(), 
									  tmc428_set_vtarget(), tmc428_get_vactual(), 
									  tmc428_get_lsmd(), 
									  tmc428_get_tmc24x_status_bits(), 
									  tmc428_get_tmc24x_stallguard_bits()
				   October  7, 2007 : tmc428_set_vmin(), tmc428_set_vmax() updated
				   October 14, 2007 : Version 2.0 completed
				   October 21, 2007 : CalcRampSteps() added for StallGuard Profiler
									  
				 				                      
   Description : Examples of basic routines to access TMC428 registers          
                 via RS232 interfaces under 32 bit MS Windows.
                                                                                
   Requirement : An interface routine tmc428spi() is required 
                 (tmc428spi4win.c for win32, spi428uc.c for uC)                 

   *************************************************************************** */


#include <windows.h>

#include <accctrl.h>
#include <winbase.h>

#include "rs232win.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <math.h>

#include "tmc428spi.h"
#include "tmc428_base.h"

#define TRUE 1
#define FALSE 0

#define MAXLEN 1000

#define PI2 6.283185307
#define PI  3.141592653

/* -------------------------------------------------------------------------- */

long int spi428glue( int rrs, int smda, int idx, int rw, long int data )
{
     long int spo; 
	 
  // check ranges of 'rrs, idx, smda, rw, data'

     // printf("spi428glue : rrs smda idx rw data = \n%d %d %d %d %d\n",rrs,smda,idx,rw,data);

	 spo = (long int)(((0x1&rrs)<<31)|((0x3&smda)<<29)|((0xF&idx)<<25)|((0x1&rw)<<24)|(0xFFFFFF&data)); 

	 // printf("spi428glue : spo = %x\n",spo);

	 return ( spo ); 
}


// long int spi428reg( idx, smda, rw, data ); // access TMC428 register


void CalcPMulPDiv(int a_max, int ramp_div, int pulse_div, int *p_mul, int *p_div, double *PIdeal, double *PBest)
{
	int d, d_best, m, m_best;
	double q_ideal, q_best, q;

	q_ideal = ( 0.95 * a_max ) / (pow(2, ramp_div-pulse_div)*128.0);
	q_best = -1;
	d_best = 0;
	m_best = 128;
	for(d=0; d<=13; d++)
	{
		m = (int) (q_ideal*pow(2, d+3));
		if(m>127 && m<256)
		{
			q = m / pow(2, d+3);
			if(fabs(q-q_best) < fabs(q_ideal-q_best) || q_best<0)
			{
				q_best = q;
				d_best = d;
				m_best = m;
			}
		}
	}

	*p_mul = m_best;
	*p_div = d_best;
	*PIdeal = q_ideal;
	*PBest = q_best;
}

void CalcRampSteps( int v_max, int a_max, int pulse_div, int ramp_div, int usrs, int *ramp_usteps, int *ramp_steps )
{
	double RampSteps; 
	
  // S = 1/2 * velocity^2 / a_max * 2^ramp_div / 2^pulse_div / 2^3 
  // (pls. refer TMC428 datasheet, section 'pmul & pdiv & usrs)

  RampSteps = 0.5 * pow((double)abs(v_max),2.0) / ((double)a_max) * pow(2.0,((double)ramp_div)) / pow(2.0,((double)pulse_div)) / 8.0; 

  *ramp_usteps = (int)(RampSteps + 1.0);        // 
  *ramp_steps  = (int)(RampSteps/pow(2,usrs));  // the number of full steps is RampStepsFS = RampSteps / 2^usrs
}


void write_tmc428_configuration_ram(unsigned char *ram_tab)
{
	int i;  long int spi, spo; 
	
	for (i=0; i<(128-2); i+=2)  // initialize TMC428 RAM table (SPI conf. & quarter sine wave LUT) 
	{
		spo = (long int) (0x80000000 | (i<<(25-1)) | (ram_tab[i+1]<<8) | (ram_tab[i])); // RRS=1, RW=0
		spi = tmc428spi( spo ); 
	}
}

void read_tmc428_configuration_ram(unsigned char *ram_tab)
{
	int i;  long int spi, spo; 
	
	for (i=0; i<127; i+=2) 
	{
		spo = (long int) (0x81000000 | (i<<(25-1)) );  // RRS=1, RW=1
		spi = tmc428spi( spo ); 

        ram_tab[i+1] = (unsigned char) (0x3f & (spi>>8)); 
	    ram_tab[i  ] = (unsigned char) (0x3f & (spi>>0)); 
	}
}

//void tmc428_set_glob_reg(int mot1r, int refmux, int cont_updt, int clk2_div, 
//						 int cs_comind, int dac_ab, int fd_ab, int ph_ab, 
//						 int sck, int nscs, int lsmd)
//{
//	long int spi, spo, data; 
//
//	data = ...
//
//	spi = spi428glue( 0, 3, JDX_SM_GLOBAL_PARAM_REGISTER, WRITE, data ); 
//	spo = tmc428spi( spi ); 
//}


void tmc428_get_pdrdus( int sm, int *pulse_div, int *ramp_div, int *usrs )
{
	long int spi, spo; 

	spi = spi428glue( 0, sm, IDX_PULSE_DIV_RAMP_DIV_USRS, READ, 0 ); 
	spo = tmc428spi( spi ); 

	*pulse_div = (int)(0xF & (spo >> 12)); 
 	 *ramp_div = (int)(0xF & (spo >>  8)); 
	     *usrs = (int)(0x7 & (spo >>  0));
}

void tmc428_set_pdrdus( int sm, int pulse_div, int ramp_div, int usrs )
{
	long int spi, spo, data; 

	data = ( (0xF&pulse_div)<<12 ) | ( (0xF&ramp_div)<<8 ) | ( (0x7&usrs)<<0 ); 

	spi = spi428glue( 0, sm, IDX_PULSE_DIV_RAMP_DIV_USRS, WRITE, data ); 
	spo = tmc428spi( spi ); 
}

void tmc428_set_pmulpdiv( int sm, int pmul, int pdiv )
{
	long int spi, spo, data; 

	data = ( (0xFF&pmul)<<8 ) | ( (0xF&pdiv)<<0 ); 

	spi = spi428glue( 0, sm, IDX_PMUL_PDIV, WRITE, data ); 
	spo = tmc428spi( spi ); 
}

void tmc428_set_refconf_rm( int sm, int ref_conf, int rm )
{
	long int spi, spo, data; 

	data = ( (0xf&ref_conf)<<8 ) | ((0x3&rm)<<0); 

	spi = spi428glue( 0, sm, IDX_REF_CONF_RM, WRITE, data ); 
	spo = tmc428spi( spi ); 
}

void tmc428_set_i_scales_a_threshold(int sm, int is_agtat, int is_aleat, int is_v0, int a_th)
{
	long int spi, spo, data; 

	data = ( (0x7&is_agtat)<<20 ) | ((0x7&is_aleat)<<16) | ((0x7&is_v0)<<12) | ((0x7&a_th)<<0); 

	spi = spi428glue( 0, sm, IDX_I_SCALES_A_THRESHOLD, WRITE, data ); 
	spo = tmc428spi( spi ); 
}

void tmc428_get_refconf_rm( int sm, int *lp, int *ref_conf, int *rm )
{
	long int spi, spo; 

	spi = spi428glue( 0, sm, IDX_REF_CONF_RM, READ, 0 ); 
	spo = tmc428spi( spi ); 

   *lp       = (0x00010000 & spo) >> 16; // read only
   *ref_conf = (0x00000f00 & spo) >>  8;
   *rm       = (0x00000003 & spo) >>  0; 
}

void tmc428_set_xtarget( int sm, int x_target )
{
	long int spi, spo;  long int data; 

    data = x_target; 

	spi = spi428glue( 0, sm, IDX_X_TARGET, WRITE, data); 
	spo = tmc428spi( spi ); 
}

void tmc428_get_xtarget( int sm, int *x_target )
{
	long int spi, spo, x_target_tmp; 

	spi = spi428glue( 0, sm, IDX_X_TARGET, READ, 0 ); 
	spo = tmc428spi( spi ); 

    x_target_tmp = 0xFFFFFF & (spo >>  0); // read raw data

    if (x_target_tmp & 0x00800000) x_target_tmp|=0xff000000;

    *x_target = x_target_tmp;
 // *x_target = 0xFFFFFF & (spo >>  0);
}

void tmc428_set_xactual( int sm, int x_actual )
{
	long int spi, spo;  long int data; 

    data = x_actual; 

	spi = spi428glue( 0, sm, IDX_X_ACTUAL, WRITE, data); 
	spo = tmc428spi( spi ); 
}

void tmc428_get_xactual( int sm, int *x_actual )
{
	long int spi, spo, x_actual_tmp; 

	spi = spi428glue( 0, sm, IDX_X_ACTUAL, READ, 0 ); 
	spo = tmc428spi( spi ); 

    x_actual_tmp = 0xFFFFFF & (spo >>  0); // read raw data

    if (x_actual_tmp & 0x00800000) x_actual_tmp|=0xff000000;

    *x_actual = x_actual_tmp;
 // *x_actual = 0xFFFFFF & (spo >>  0);
}


void tmc428_set_vmin( int sm, int v_min )
{
	long int spi, spo;  long int data; 

    data = 0x7FF & ((long int)v_min); 

	spi = spi428glue( 0, sm, IDX_V_MIN, WRITE, data); 
	spo = tmc428spi( spi ); 
}


void tmc428_set_vmax( int sm, int v_max )
{
	long int spi, spo;  long int data; 

    data = 0x7FF & ((long int)v_max); 

	spi = spi428glue( 0, sm, IDX_V_MAX, WRITE, data); 
	spo = tmc428spi( spi ); 
}


void tmc428_set_amax( int sm, int a_max  )
{
	long int spi, spo;  long int data; 

    data = a_max; 

	spi = spi428glue( 0, sm, IDX_A_MAX, WRITE, data); 
	spo = tmc428spi( spi ); 
}


void tmc428_get_amax( int sm, int *a_max )
{
	long int spi, spo;  int a_max_tmp; 

	spi = spi428glue( 0, sm, IDX_A_MAX, READ, 0 ); 
	spo = tmc428spi( spi ); 

    a_max_tmp = 0x0007FF & (spo >>  0); // read raw data

    *a_max = a_max_tmp;
}


void tmc428_get_aactual( int sm, int *a_actual )
{
	long int spi, spo, a_actual_tmp; 

	spi = spi428glue( 0, sm, IDX_A_ACTUAL, READ, 0 ); 
	spo = tmc428spi( spi ); 

    a_actual_tmp = 0xFFFFFF & (spo >>  0); // read raw data

    if (a_actual_tmp & 0x00000800) a_actual_tmp|=0xff000000;

    *a_actual = a_actual_tmp;

   // *a_actual = 0xFFFFFF & (spo >>  0);
}


void tmc428_set_pos( int x, int y, int z )
{
	tmc428_set_xtarget( 0, x );
	tmc428_set_xtarget( 1, y );
	tmc428_set_xtarget( 2, z );
}


void tmc428_get_pos( int *x, int *y, int *z )
{
	int a, b, c; 

	tmc428_get_xactual( 0, &a );
	tmc428_get_xactual( 1, &b );
	tmc428_get_xactual( 2, &c );

	*x = a; 
	*y = b; 
	*z = c; 
}

void tmc428_set_vtarget( int sm, int v_target )
{
	long int spi, spo;  long int data; 

	// fprintf(stderr,"v_target = %d\n"); // debug output
	
	v_target = ( v_target >=  2047 ) ?  2047 : v_target; 
	v_target = ( v_target <= -2048 ) ? -2048 : v_target; 

	// fprintf(stderr,"V_TRAGET = %d\n"); // debug output

    data = 0x000FFF & ((long int)v_target); 

	spi = spi428glue( 0, sm, IDX_V_TARGET, WRITE, data); 
	spo = tmc428spi( spi ); 
}

void tmc428_get_vactual( int sm, int *v_actual )
{
	long int spi, spo, v_actual_tmp; 

	spi = spi428glue( 0, sm, IDX_V_ACTUAL, READ, 0 ); 
	spo = tmc428spi( spi ); 

    v_actual_tmp = 0xFFFFFF & (spo >>  0); // read raw data

    if (v_actual_tmp & 0x00000800) v_actual_tmp|=0xff000000;

    *v_actual = v_actual_tmp;

 // *v_actual = 0xFFFFFF & (spo >>  0);
}

void tmc428_get_lsmd( int *lsmd )
{
	long int spi, spo; 
	int lsmd_tmp;      

	spi = spi428glue( 0, 0x3, JDX_SM_GLOBAL_PARAM_REGISTER, READ, 0 ); 
	spo = tmc428spi( spi ); 

    lsmd_tmp = 0x000003 & (spo >>  0); // read raw data

    *lsmd = lsmd_tmp;
}

void tmc428_get_datagram_words( long int *datagram_low_word, long int *datagram_high_word )
{
	long int spi, spo; 
	
 // write datagram_low_word (or high_word) for initialization, sets CDGW to '1'
    spi = spi428glue( 0, 0x3, JDX_DATAGRAMM_LOW, WRITE, 0); 
	spo = tmc428spi( spi ); 

 // wait until CDGW becomes '0'
	do 
	{
		Sleep(1); 

		spi = spi428glue( 0, 0x3, JDX_DATAGRAMM_LOW, READ, 0); 
		spo = tmc428spi( spi ); 

	} while (!(0x40000000 & (~spo))); // mask CDGW

	spi = spi428glue( 0, 0x3, JDX_DATAGRAMM_LOW, READ, 0); 
	spo = tmc428spi( spi ); 

	*datagram_low_word = 0x00FFFFFF & spo; 
	
	spi = spi428glue( 0, 0x3, JDX_DATAGRAMM_HIGH, READ, 0); 
	spo = tmc428spi( spi ); 

	*datagram_high_word = 0x00FFFFFF & spo; 
}

void tmc428_get_tmc24x_status_bits( int *sm0_status, int *sm1_status, int *sm2_status )
{
	long int datagram_low_word, datagram_high_word; 
	int      lsmd; 
	
	// this routine assumes a chain of up to three TMC246 / TMC249 drivers
	// pls. refer TMC428 datasheet v. 2.02 / April 26, 2007 or newer
	
	tmc428_get_datagram_words( &datagram_low_word, &datagram_high_word ); 

    // 1st, determine the number of TMC246 resp. TMC249 stepper motor drivers
	
	tmc428_get_lsmd( &lsmd ); 

	
	switch (((char)lsmd)) 
	{
		case 3 : ; // TMC428 interprets LSMD=3 as LSMD = 2 // NO "break;" 
		case 2 : *sm0_status = 0x0fff & ( datagram_high_word >>  0 ); // motor #1
			     *sm1_status = 0x0fff & ( datagram_low_word  >> 12 ); // motor #2
			     *sm2_status = 0x0fff & ( datagram_low_word  >>  0 ); // motor #3
			     break; 

		case 1 : *sm0_status = 0x0fff & ( datagram_low_word  >> 12 ); // motor #1
			     *sm1_status = 0x0fff & ( datagram_low_word  >>  0 ); // motor #2
			     *sm2_status = 0x0000;                                // motor #3
		         break; 

		case 0 : *sm0_status = 0x0fff & ( datagram_low_word >>   0 ); // motor #1
			     *sm1_status = 0x0000;                                // motor #2
			     *sm2_status = 0x0000;                                // motor #3
			     break; 
	}
}

void tmc428_get_tmc24x_stallguard_bits( int sm, int *ld )
{
	int sm0_status, sm1_status, sm2_status; 

	tmc428_get_tmc24x_status_bits( &sm0_status, &sm1_status, &sm2_status ); 
	
	switch (((char)sm)) 
	{
		case 0 : *ld = 0x7 & ( sm0_status >> 9 ); break; 
		case 1 : *ld = 0x7 & ( sm1_status >> 9 ); break; 
		case 2 : *ld = 0x7 & ( sm2_status >> 9 ); break; 
	}
}	

/* *************************************************************************** */

