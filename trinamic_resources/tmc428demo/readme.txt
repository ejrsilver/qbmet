
-------------------------------------------------------------------------------------------

  These routines all together build a simple example application to control the TMC428. 
  One can directly run it together with an TMC428 Evaluation Kit. 

  These sources are distributed in the hope that they will be useful. They might be a 
  basis for your own application. 

-------------------------------------------------------------------------------------------

       THE CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
       EITHER EXPRESSED OR IMPLIED.

-------------------------------------------------------------------------------------------

  rs232win.c       : a couple of routines to access RS232 under win32 (Win9x, NT4, ...)
  rs232win.h       : header file for rs232win.c

  tmc428spi.c      : SPI IO routine tmc428spi() - here just calling tmc428spi4win()
  tmc428spi.h      : header file of tmc428spi.c


  tmc428spi4win.c  : SPI routines to access the TMC428-Eval via RS232 under win32
  tmc428spi4win.h  : header file of tmc428spi4win.c


  tmc428_base.c    : basic tmc428 access routines (basic register IO, ...)
  tmc428_base.h    : header file of tmc428spi_base.c

  tmc428_misc.c    : miscellaneous routines (TMC428 RAM table dump)
  tmc428_misc.h    : header file of tmc428_misc.c

  tmc428demo.c     : MAIN program demo using 'tmc428spi4win.c', 'tmc428spi_base.c', ...

  tmc428demo.exe   : Win32 executable - run this program from console (cmd)

-------------------------------------------------------------------------------------------

  The code has been compiled using MS Visual C++ 5.0 (sources on local drive C:\)

-------------------------------------------------------------------------------------------

  Version : 1.0              : tmc428demo (with basic routines demonstrating TMC428 usage)
  Version : 2.0              : tmc428demo with StallGuard Demo & StallGuard Profiler

  History : October 14, 2007 : StallGuard Demo & StallGuard Profiler together with some 
                               additional required subroutines added, some routines updated, 
                               e.g. open_rs232() updated to handle COM1 ... COM99, pls. 
                               refer history hints within C source files for details
            October 21, 2007 : calculation of required steps for RAMP_MODE StallGuard 
                               Profiler added

-------------------------------------------------------------------------------------------
