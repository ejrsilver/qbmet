/* ***************************************************************************

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
  
   *************************************************************************** */

/* ***************************************************************************

   File        : tmc428spi.c     
   
   Version     : 1.0 / December  6, 2002
                 2.0 /  October 14, 2007 : still used for tmc428demo v. 2.0

   History     :    June 21, 2005 : company address updated, SPI routine spi428()
                                    renamed to tmc428spi()
				 October 12, 2006 : missing #include <windows.h> added
                                                               
                                                                                
   Description : Routine 'spi428()'. This has to be changed / implemented for    
                 own applications on other plat forms. Here its just the        
                 definition of a macro spi() instantiating tmc428spi4win()      
                                                                                
                                                                                
   Requirement : An interface routine spi428() is required (tmc428spi4win.c)    
                                                                                                                                          

   *************************************************************************** */
                                                                                
#include <windows.h>

#include <accctrl.h>
#include <winbase.h>

extern HANDLE GLOBAL_ComHandle; // = NULL; 

#include "tmc428spi.h"
#include "tmc428spi4win.h"		// replace this if 'tmc428spi4win' is not used


/* -------------------------------------------------------------------------- */

long int tmc428spi( long int spi ) // this function might be replaced by your own 
{
    long int spo; 

    spo = tmc428spi4win( GLOBAL_ComHandle, spi ); // comment this when using uC
 // spo = tmc428spi_uc( spi );                    // SPI routine when using uC

 // fprintf(stdout,"%08x < tmc428 < %08x\n",spo,spi); // trace SPI communication

    return ( spo ); 
}

/* *************************************************************************** */

