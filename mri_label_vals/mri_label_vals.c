#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "mri.h"
#include "macros.h"
#include "error.h"
#include "diag.h"
#include "proto.h"
#include "utils.h"
#include "timer.h"
#include "version.h"
#include "label.h"
#include "cma.h"

int main(int argc, char *argv[]) ;
static int get_option(int argc, char *argv[]) ;

char *Progname ;
static char *log_fname = NULL ;
static void usage_exit(int code) ;

static int  annot_flag = -1 ;
static int quiet = 0 ;
static int scaleup_flag = 0 ;
int cras =0; // 0 is false.  1 is true

int
main(int argc, char *argv[])
{
  char   **av, *label_name, *vol_name, *out_name ;
  int    ac, nargs ;
  int    msec, minutes, seconds,  i  ;
	LABEL  *area ;
  struct timeb start ;
  MRI    *mri,  *mri_seg ;
	Real   xw, yw, zw, xv, yv, zv, val;

  /* rkt: check for and handle version tag */
  nargs = handle_version_option (argc, argv, "$Id: mri_label_vals.c,v 1.9 2004/01/08 19:05:11 fischl Exp $", "$Name:  $");
  if (nargs && argc - nargs == 1)
    exit (0);
  argc -= nargs;

  Progname = argv[0] ;
  ErrorInit(NULL, NULL, NULL) ;
  DiagInit(NULL, NULL, NULL) ;

  TimerStart(&start) ;

  ac = argc ;
  av = argv ;
  for ( ; argc > 1 && ISOPTION(*argv[1]) ; argc--, argv++)
  {
    nargs = get_option(argc, argv) ;
    argc -= nargs ;
    argv += nargs ;
  }

  if (argc < 3)
    usage_exit(1) ;

  vol_name = argv[1] ;
  label_name = argv[2]  ;
  out_name = argv[3] ;

  mri = MRIread(vol_name) ;
  if (!mri)
    ErrorExit(ERROR_NOFILE, "%s: could not read volume from %s",Progname, vol_name) ;
	if (scaleup_flag)
	{
		float scale, fov_x, fov_y, fov_z  ;

		scale = 1.0/MIN(MIN(mri->xsize, mri->ysize),mri->zsize) ;
		fprintf(stderr, "scaling voxel sizes up by %2.2f\n", scale) ;
		mri->xsize *= scale ; mri->ysize *= scale ; mri->zsize *= scale ;
    fov_x = mri->xsize * mri->width;
    fov_y = mri->ysize * mri->height;
    fov_z = mri->zsize * mri->depth;
    mri->xend = fov_x / 2.0;
    mri->xstart = -mri->xend;
    mri->yend = fov_y / 2.0;
    mri->ystart = -mri->yend;
    mri->zend = fov_z / 2.0;
    mri->zstart = -mri->zend;

    mri->fov = (fov_x > fov_y ? (fov_x > fov_z ? fov_x : fov_z) : (fov_y > fov_z ? fov_y : fov_z) );
	}

	if (annot_flag >= 0)
	{
		int x, y, z  ;

		mri_seg = MRIread(argv[2]) ;
		if (!mri_seg)
			ErrorExit(ERROR_NOFILE, "%s: could not read volume from %s",Progname, argv[2]) ;

		for (x = 0  ; x  < mri_seg->width ; x++)
		{
			for (y = 0  ; y  < mri_seg->width ; y++)
			{
				for (z = 0  ; z  < mri_seg->width ; z++)
				{
					if (MRIvox(mri_seg, x, y,  z) == annot_flag)
					{
						MRIvoxelToSurfaceRAS(mri_seg, x, y,  z, &xw,  &yw, &zw) ;
						MRIsurfaceRASToVoxel(mri, xw,  yw,  zw, &xv, &yv, &zv) ;
						MRIsampleVolumeType(mri, xv,  yv, zv, &val, SAMPLE_NEAREST);
#if  0
						if (val < .000001)
						{  
							val *= 1000000;
							printf("%f*0.000001\n", val);
						}
						else
#endif
							printf("%f\n", val);
					}
				}
			}
		}
	}
	else
	{
		area = LabelRead(NULL, label_name) ;
		if (!area)
			ErrorExit(ERROR_NOFILE, "%s: could not read label from %s",Progname, label_name) ;
		
		if (cras == 1)
			fprintf(stderr,"using the label coordinates to be c_(r,a,s) != 0.\n");
		
		for (i = 0 ;  i  < area->n_points ; i++)
		{
			
			xw =  area->lv[i].x ;
			yw =  area->lv[i].y ;
			zw =  area->lv[i].z ;
			if (cras == 1)
				MRIworldToVoxel(mri, xw, yw,  zw, &xv, &yv, &zv) ;
			else
				MRIsurfaceRASToVoxel(mri, xw, yw, zw, &xv, &yv, &zv);
			MRIsampleVolumeType(mri, xv,  yv, zv, &val, SAMPLE_NEAREST);
#if 0
			if (val < .000001)
			{  
				val *= 1000000;
				printf("%f*0.000001\n", val);
			}
			else
#endif
				printf("%f\n", val);
		}
	}
  msec = TimerStop(&start) ;
  seconds = nint((float)msec/1000.0f) ;
  minutes = seconds / 60 ;
  seconds = seconds % 60 ;

  if (DIAG_VERBOSE_ON)
    fprintf(stderr, "label value extractiong took %d minutes and %d seconds.\n", 
            minutes, seconds) ;

  exit(0) ;
  return(0) ;
}
/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static int
get_option(int argc, char *argv[])
{
  int  nargs = 0 ;
  char *option ;
  
  option = argv[1] + 1 ;            /* past '-' */
  if (strcmp("cras", option) == 0)
    cras = 1;
	else if (strcmp("scaleup", option) == 0)
		scaleup_flag = 1 ;
  else
  {
    switch (toupper(*option))
    {
    case 'Q':
      quiet = 1 ;
      break ;
    case 'A':
      annot_flag = atoi(argv[2]) ;
			nargs =  1  ;
			fprintf(stderr,"using  annotation %d as label...\n", annot_flag)  ;
      break ;
    case 'L':
      log_fname = argv[2] ;
      nargs = 1 ;
      fprintf(stderr, "logging results to %s\n", log_fname) ;
      break ;
    case 'U':
      usage_exit(0) ;
      break ;
    default:
      fprintf(stderr, "unknown option %s\n", argv[1]) ;
      exit(1) ;
      break ;
    }
  }
  return(nargs) ;
}
/*----------------------------------------------------------------------
            Parameters:

           Description:
----------------------------------------------------------------------*/
static void
usage_exit(int code)
{
  printf("usage: %s [options] <volume> <label file>\n",  Progname) ;
  printf("where optins are\n");
  printf("   -cras   label created in the coordinates where c_(r,a,s) != 0\n");
  printf("             if it did not work, try using this option.\n");
  // printf("   -q      quiet\n");
  // printf("   -a      all\n");
  // printf("   -l file log results to a file.\n");
  printf("   -u      print this help.\n");
  exit(code) ;
}
