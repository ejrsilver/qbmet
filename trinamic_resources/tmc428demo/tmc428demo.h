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

   File        : tmc428demo.h
   
   Version     : 1.0 / December 12, 2002
                 2.0 /  October 14, 2007 : prototypes added (not yet implemented)


   Description : Header file for tmc428demo.c


   History     : October 3, 2007 : creation

   *************************************************************************** */

void tmc428demo_ramp_mode_run(int sm, int a, int v, int x ); 
void tmc428demo_velocity_mode_run(int sm, int a, int v ); 

/* *************************************************************************** */

