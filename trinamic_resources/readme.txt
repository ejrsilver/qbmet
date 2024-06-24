sample428_236\ : sample programm for uC to initialize the TMC428 for TMC236
tmc428demo\    : tmc428 demo program (sources) for TMC428-EvalBoard / Win32


tmc428_frequencies.xls : former 'tmc428_rhz.xls' and 'tmc428_rhz2va.xls'
                         within one spreadsheet (see below)

     tmc428_rhz.xls    : Spreadsheet calculating microstep frequency and 
                         acceleration and different motion parameters in 
                         common units (e.g. rotations per second) for a 
                         given set of TMC428 parameters and stepper motor 
                         parameters

     tmc428_rhz2va.xls : Spreadsheet calculating velocity and acceleration 
                         parameters (v_max, a_max) in units of the TMC428 
                         for a given set of motion parameters (steps/s, 
                         steps/s^2) and clock frequency, etc.  


tmc236x3.428      : Example of a RAM table configuration file for a TMC428 
                    controlling up to three TMC236 / TMC239 /TMC246 / TMC249
                    or a combination of them. This configuration file can be 
                    loaded by the TMC428-Evaluation Software into the TMC428 
                    configuration RAM. The Mixed Decay if of for all three 
                    drivers. 

tmc236x3_datasheet.428 : Example of the RAM configuration file for the TMC428 
                         controlling up to three TMC236 / TMC239 /TMC246 / TMC249
                         or a combination of them. This example is given in the 
                         TMC428 datasheet from version 2.00 on. 

tmc428_pmulpdiv.c~ : First example a program that calculates pmul & pdiv
                     (according to TMC428 datasheet up to version 1.02)

tmc428_pmulpdiv.c  : Example of a program that automatically calculates 
                     pmul & pdiv for given amax & pulse_div & ramp_div
                     (1st included within TMC428 datasheet version 2.00)

tmc428_pmulpdiv.xls : Spreadsheet calculating pmul & pdiv for given 
                      amax & pulse_div & ramp_div (also that variant 
                      included according to description within TMC428 
                      data sheet up to version 1.02)

microstep_table_64_hex.xls: converts 64 microstep tables with inverted bits to graphics, 
		            or generates hexadecimal tables from manual input
