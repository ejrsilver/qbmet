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

   File        : rs232win.c       
   
   Version     : 1.0 / December 12, 2002
                 2.0 /  October 14, 2007
                                              
   History     :    June 21, 2005 : company address updated
                 October 14, 2007 : COM1 .. COM2 updated to COM1..COMn for 
				                    use with USB-to-Serial adapters (see
					 			    open_rs232), return type changed from 
					 			    char to int for getc_rs232() to allow 
					 			    error handling
				   
   Description : This is a collection of routines to access RS232 interfaces    
                 within a 32 bit MS windows environments. It (Win9x,NT4,2K,.)   
                 It is also suitable for Win32 Console Applications. Although
				 byte-by-byte communication might be less efficient, it has 
				 been chosen to keep the accessing (SPI) routines more general.
                                          
   *************************************************************************** */
                                                                                

#include <windows.h>

//#include <comdef.h>
#include <accctrl.h>
#include <winbase.h>

#include <stdlib.h>
#include <stdio.h>


HANDLE open_rs232 ( int com, int baud, int *error_code ) // return CommHandle
{
    HANDLE ComHandle = NULL;

	char ComName[8];
    DCB CommDCB;
	COMMTIMEOUTS CommTimeouts;

  //if(com==2) strcpy(ComName, "COM2");
  //else       strcpy(ComName, "COM1");	
    if (com>0 && com<99) sprintf(ComName,"COM%d",com); // updated October 7, 2007
	else  strcpy(ComName, "COM1");	

	// open selected serial interface {COM1, COM2}
	ComHandle=CreateFile(ComName, GENERIC_READ|GENERIC_WRITE, 0, NULL, 
		                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(GetLastError()!=ERROR_SUCCESS)
    {
		*error_code = -1; // MessageBox("ERROR : RS232 interface not opened.");
	}
	else
	{
		*error_code = 0;

		 // set communication parameters ...

		GetCommState(ComHandle, &CommDCB);

	 // CommDCB.BaudRate=19200;
		CommDCB.BaudRate=baud;
		CommDCB.Parity=NOPARITY;
		CommDCB.StopBits=ONESTOPBIT;
		CommDCB.ByteSize=8;
 
		CommDCB.fBinary=1;  // set binary mode for transparent data transmission
		CommDCB.fParity=0;          
		CommDCB.fOutxCtsFlow=0;       
		CommDCB.fOutxDsrFlow=0;     
		CommDCB.fDtrControl=0;      
		CommDCB.fDsrSensitivity=0;  
		CommDCB.fTXContinueOnXoff=0;
		CommDCB.fOutX=0;           
		CommDCB.fInX=0;            
		CommDCB.fErrorChar=0;      
		CommDCB.fNull=0;           
		CommDCB.fRtsControl=0;      
		CommDCB.fAbortOnError=0;    

		SetCommState(ComHandle, &CommDCB);

		// set buffer size
		SetupComm(ComHandle, 100, 100);  // receive buffer size, send buffer size 

		// set time out, non-set time out may significantly slow down the program
		GetCommTimeouts(ComHandle, &CommTimeouts);

		CommTimeouts.ReadIntervalTimeout=MAXDWORD;
		CommTimeouts.ReadTotalTimeoutMultiplier=0; 
		CommTimeouts.ReadTotalTimeoutConstant=0;

		SetCommTimeouts(ComHandle, &CommTimeouts);
	}

	return ( ComHandle );
}


void close_rs232 ( HANDLE ComHandle ) 
{
	// clear send buffer and receive buffer
	PurgeComm(ComHandle, PURGE_TXCLEAR|PURGE_RXCLEAR);

	// close interface
	CloseHandle(ComHandle);
}

void putc_rs232 ( HANDLE ComHandle, int c ) // send single char via RS232
{
	char buffer[100];
	ULONG BytesWritten;

    buffer[0] = ((char) (0xff & c));  
    buffer[1] = '\0'; 

	// to send more than one character at once
    // UpdateData(TRUE);
    // strcpy(buffer, LPCTSTR(EditToSend));

	// write content of buffer to RS232
	WriteFile(ComHandle, buffer, 1, &BytesWritten, NULL);
}

int getc_rs232 ( HANDLE ComHandle ) // read single char from RS232 
{
	char buffer[100];
	int c; 
	ULONG BytesRead;

    BytesRead = 0;
    while (BytesRead==0)
    {
	   // read only up to 1 byte from RS232 into buffer 
	   ReadFile(ComHandle, buffer, 1, &BytesRead, NULL);
       if (BytesRead==0) Sleep( 1 ); // sleep for 1 ms 
    }

	if ( BytesRead == 0 ) c = '\0'; // return '\0' if buffer empty
	else                { c = 0xff & buffer[0]; buffer[BytesRead] = '\0'; }

	return ( c ); // return buffer as '\0' terminated string
}

/* *************************************************************************** */

