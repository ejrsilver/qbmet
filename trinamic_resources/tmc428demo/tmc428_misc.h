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

   File        : tmc428_misc.h
   
   Version     : 1.0 /    July 12, 2005
                 2.0 / October 14, 2007 : prototypes updated


   Description : Header file for 'tmc428_misc.c'


   History     : September 30, 2007 : tmc428_get_tmc24x_status_bits(), 
                                      tmc428_stallguard_demo(), 
                                      tmc428_stallguard_profiler() added, 
				   October  7, 2007 : tmc428spi4win_id_string_request() added
				   October 14, 2007 : StallGuard routines completed

   *************************************************************************** */

void dump_tmc428_ram_tab(unsigned char *ram_tab); 
float r_fs_hz( float f_clk, int v, int pulse_div, int usrs ); 
void tmc428spi4win_id_string_request( HANDLE comHANDLE ); 
void tmc428_get_tmc24x_status_bits( int *sm0_status, int *sm1_status, int *sm_2_status ); 
void tmc428_get_tmc24x_stallguard_bits_n_times(int sm, int n, double *x_mean, double *x_sigm); 
void tmc428_stallguard_profiler( int sm, int v_max_min, int v_max_max, int v_step, int x_min, int x_max ); 
void tmc428_stallguard_demo( int sm, int vmax, int sgl ); 

/* *************************************************************************** */

