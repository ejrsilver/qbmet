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

   File        : tmc428_base.h
   
   Version     : 1.0 / December 12, 2002 
                 2.0 /  October 14, 2007


   Description : Header file with constants and basic TMC428 routines


   History     :    July 12, 2005 : prototypes updated, IDX_I_SCALES_A_THRESHOLD
                 October 14, 2007 : prototypes updated

   *************************************************************************** */

#define WRITE 0
#define READ  1

#define IDX_X_TARGET                       0x0
#define IDX_X_ACTUAL                       0x1
#define IDX_V_MIN                          0x2
#define IDX_V_MAX                          0x3
#define IDX_V_TARGET                       0x4
#define IDX_V_ACTUAL                       0x5
#define IDX_A_MAX                          0x6
#define IDX_A_ACTUAL                       0x7
#define IDX_I_SCALES_A_THRESHOLD           0x8
#define IDX_PMUL_PDIV                      0x9
#define IDX_REF_CONF_RM                    0xA
#define IDX_INTERRUPT_MASK_INTERRUPT_FLAGS 0xB
#define IDX_PULSE_DIV_RAMP_DIV_USRS        0xC
#define IDX_REF_TOLERANCE                  0xD
#define IDX_X_LATCHED                      0xE

#define JDX_DATAGRAMM_LOW                  0x0
#define JDX_DATAGRAMM_HIGH                 0x1
#define JDX_CW_COVER_POS_COVER_LEN         0x2
#define JDX_COVER_DATAGRAMM                0x3 
#define JDX_POWER_DOWN                     0x8
#define JDX_L3R3L2R2L1R1                   0xE
#define JDX_SM_GLOBAL_PARAM_REGISTER       0xF

#define RAMP_MODE      0
#define SOFT_MODE      1
#define VELOCITY_MODE  2
#define HOLD_MODE      3

/* -------------------------------------------------------------------------- */

long int spi428glue( int rrs, int smda, int idx, int rw, long int data ); 

void CalcPMulPDiv(int a_max, int ramp_div, int pulse_div, int *p_mul, int *p_div, 
                  double *PIdeal, double *PBest); 

void CalcRampSteps( int v_max, int a_max, int pulse_div, int ramp_div, int usrs, int *ramp_usteps, int *ramp_steps ); 

void tmc428_set_pdrdus( int sm, int pulse_div, int ramp_div, int usrs ); 
void tmc428_get_pdrdus( int sm, int *pulse_div, int *ramp_div, int *usrs ); 

void tmc428_set_pmulpdiv( int sm, int pmul, int pdiv ); 

void tmc428_set_i_scales_a_threshold(int sm, int is_agtat, int is_aleat, int is_v0, int a_th); 

void tmc428_set_refconf_rm( int sm, int ref_conf, int rm ); 

void tmc428_set_xtarget( int sm, int x_target ); 
void tmc428_get_xtarget( int sm, int *x_target ); 

void tmc428_set_xactual( int sm, int x_actual ); 
void tmc428_get_xactual( int sm, int *x_actual ); 

void tmc428_set_vmin( int sm, int v_min ); 
void tmc428_set_vmax( int sm, int v_max ); 

void tmc428_set_amax( int sm, int a_max ); 
void tmc428_get_amax( int sm, int *a_max ); 

void tmc428_set_va_single( int sm, int v, int a );
void tmc428_set_va( int vx, int vy, int vz, int ax, int ay, int az );

void tmc428_set_pos( int x, int y, int z );
void tmc428_get_pos( int *x, int *y, int *z );

void tmc428_set_vtarget( int sm, int v_target ); 
void tmc428_get_vactual( int sm, int *v_actual ); 

void tmc428_get_aactual( int sm, int *a_actual ); 

void write_tmc428_configuration_ram(unsigned char *ram_tab); 
void read_tmc428_configuration_ram(unsigned char *ram_tab); 

void tmc428_get_lsmd( int *lsmd ); 
void tmc428_get_datagram_words( long int *datagram_low_word, long int *datagram_high_word );
void tmc428_get_tmc24x_stallguard_bits( int sm, int *ld );

/* *************************************************************************** */

