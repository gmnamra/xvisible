

// Define Print routine for altiVec vectors



#if defined(__APPLE__)
#ifdef __ppc__

#include <Carbon/Carbon.h>


typedef union
{
  unsigned char c[16];
  signed char sc[16];
  unsigned short s[8];
  signed short ss[8];
  unsigned long l[4];
  signed long sl[4];
  float f[4];
  vector unsigned char vc;
  vector unsigned short vs;
  vector unsigned int vl;
  vector float vf;
}
rc_simd;
    
enum{ BYTE, SHORT, LONG, FLOAT, HEX, DEC, NON };

void printVector(void * data, int type, int format, char *text = NULL)
{
	int	i;
      	rc_simd d;
        
	d.vc = *((vector unsigned char *) data);
	if (text) fprintf (stderr, "%s ", text);
	if ( ( type == BYTE ) && (format == HEX) )
		for ( i = 0; i < 16; i++ )
			fprintf(stderr,  "%.4x ", d.c[i]);
        else if ( (type == BYTE) && (format == DEC) )
                for ( i = 0; i < 16; i++ )
			fprintf(stderr,  "%4d ", d.sc[i]);
	else if ( ( type == SHORT ) && (format == HEX) )
		for ( i = 0; i < 8; i++ )
			fprintf(stderr,  "%.4x ", d.s[i]);
        else if ( (type == SHORT) && (format == DEC) )
                for ( i = 0; i < 8; i++ )
			fprintf(stderr,  "%7d ", d.ss[i]);
	else if ( ( type == LONG ) && (format == HEX) )
		for ( i = 0; i < 4; i++ )
			fprintf(stderr,  "%.8x ", (int) d.l[i]);
        else if ( ( type == LONG) && (format == DEC) )
                for ( i = 0; i < 4; i++ )
			fprintf(stderr,  "%10d ", (int) d.sl[i]);
	else if ( type == FLOAT )
		for ( i = 0; i < 4; i++ )
			fprintf(stderr,  "%10e ", d.f[i]);
        fprintf(stderr, "\n");

}

#endif
#endif // __APPLE__
