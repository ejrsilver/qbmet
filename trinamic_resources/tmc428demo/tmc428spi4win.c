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

    File        : tmc428spi4win.c      
    
    Version     : 1.0 / December 12, 2002
				  2.0 /  October 14, 2007
    
	History     :    June 21, 2005 : company address updated
	                 July 12, 2005 : include file names updated
				  October  6, 2007 : TMC428 Eval PCB answer string request added
                                                                            
    Description : Example of basic TMC428 access using 'rs232win.c' via RS232  
                  interface under 32 bit MS Windows using TMC428-Eval board.   
                                                                            
                  To adapt to own SPI interfaces, the routine 'tmc428spi' has     
                  to be replaced by an other own one (see 'tmc428spi.c')       
                                                                            
   *************************************************************************** */


#include <windows.h>

#include <accctrl.h>
#include <winbase.h>

#include "rs232win.h"

#include <stdlib.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0


#include "tmc428spi.h"
#include "tmc428_base.h"


long int tmc428spi4win( HANDLE comHANDLE, long int spi )
{
	int a, b, c, d; // type changed from int to char
	long int spo; 

    a = (char)(0xff & ( spi >> 24 )); 
    b = (char)(0xff & ( spi >> 16 )); 
    c = (char)(0xff & ( spi >>  8 )); 
    d = (char)(0xff & ( spi >>  0 )); 

    //printf("tmc428 < spi = %2x %2x %2x %2x : ", a, b, c, d); 

	putc_rs232( comHANDLE, 0x23 ); // '#'
	putc_rs232( comHANDLE,    a );	
	putc_rs232( comHANDLE,    b );	
	putc_rs232( comHANDLE,    c );
    putc_rs232( comHANDLE,    d );
 
	a = 0xff & getc_rs232( comHANDLE );
	b = 0xff & getc_rs232( comHANDLE );
	c = 0xff & getc_rs232( comHANDLE );
	d = 0xff & getc_rs232( comHANDLE );

    spo = 0xFFFFFFFF & ((a<<24) | (b<<16) | (c<<8) | (d<<0)); 

    //printf("tmc428 > spo = %2x %2x %2x %2x\n", a, b, c, d); 

	return ( spo ); 
}

/* *************************************************************************** */

