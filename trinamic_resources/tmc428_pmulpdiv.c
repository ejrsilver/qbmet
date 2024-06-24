/* PROGRAM EXAMPLE 'pmulpdiv.c' : How to Calculate p_mul & p_div for the TMC428 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void CalcPMulPDiv(int a_max, int ramp_div, int pulse_div, float p_reduction, 
                  int *p_mul, int *p_div, double *PIdeal, double *PBest, double *PRedu )
{
  int    pdiv, pmul, pm, pd; 
  double p_ideal, p_best, p, p_reduced; 

  pm=-1; pd=-1; // -1 indicates : no valid pair found
  p_ideal   = a_max /  (pow(2, ramp_div-pulse_div)*128.0); 
  p         = a_max / ( 128.0 * pow(2, ramp_div-pulse_div) ); 
  p_reduced = p * ( 1.0 - p_reduction ); 

  for (pdiv=0; pdiv<=13; pdiv++)
  {
       pmul = (int)(p_reduced * 8.0 * pow(2, pdiv)) - 128; 

	   if ( (0 <= pmul) && (pmul <= 127) ) 
	   {
	       pm = pmul + 128; 
               pd = pdiv; 
	   }
  }

  *p_mul = pm; 
  *p_div = pd; 

  p_best = ((double)(pm)) / ((double)pow(2,pd+3)); 

  *PIdeal = p_ideal;
  *PBest  = p_best;
  *PRedu  = p_reduced; 
}

int main(int argc, char **argv)
{
    int  a_max=0, ramp_div=0, pulse_div=0, p_mul, p_div, 
		 a_max_lower_limit=0, a_max_upper_limit=0; 
	double pideal, pbest, predu;  
	float  p_reduction=0.0; 

    char **argp;

    if (argc>1)
    {
      while (argv++, argc--)
      {
        argp = argv + 1;   if (*argp==NULL) break;

        if (      (!strcmp(*argv,"-a")) ) sscanf(*argp,"%d",&a_max);
        else if ( (!strcmp(*argv,"-r")) ) sscanf(*argp,"%d",&ramp_div);
        else if ( (!strcmp(*argv,"-p")) ) sscanf(*argp,"%d",&pulse_div);
        else if ( (!strcmp(*argv,"-pr"))) sscanf(*argp,"%f",&p_reduction); 
      }
    }
    else 
    {   
      fprintf(stderr,"\n  USAGE : pmulpdiv -a <a_max> -r <ramp_div> -p <pulse_div> -pr <0.00 .. 0.10>\n"
                     "  EXAMPLE : pmulpdiv -a 10 -r 3 -p 3 -pr 0.05\n"); 
      return 1;
    }

    printf("\n\n  a_max=%d\tramp_div=%d\tpulse_div=%d\tp_reduction=%f\n\n", 
		          a_max, ramp_div, pulse_div, p_reduction); 

    CalcPMulPDiv(a_max, ramp_div, pulse_div, p_reduction, &p_mul, &p_div, &pideal, &pbest, &predu );

	printf("  p_mul = %3.3d\n  p_div = %3d\n\n p_ideal = %f\n  p_best = %f\n  p_redu = %f\n\n", 
              p_mul, p_div, pideal, pbest, predu);

    a_max_lower_limit = (int)pow(2,(ramp_div-pulse_div-1)); 
    printf("\n a_max_lower_limit = %d",a_max_lower_limit); 
    if (a_max < a_max_lower_limit) printf("   [WARNING: a_max < a_max_lower_limit]");
    a_max_upper_limit = ((int)pow(2,(12+(ramp_div-pulse_div)))) -1; 
    printf("\n a_max_upper_limit = %d",a_max_upper_limit); 
    if (a_max > a_max_upper_limit) printf("   [WARNING: a_max > a_max_upper_limit]");
    printf("\n\n");

    return 0;
}

/* -------------------------------------------------------------------------- */
