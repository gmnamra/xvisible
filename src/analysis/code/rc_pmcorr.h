



#include <rc_analysis.h>

#ifdef __ppc__
void altivec16bitPelProducts ( const unsigned short *input1, const unsigned short *input2, rcCorr& res, unsigned int m8, 
                              vector unsigned char, vector unsigned char);
void altivec8bitPelProducts ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
                              vector unsigned char, vector unsigned char);
void altivec8bitPelProductsSumI ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
                                  vector unsigned char, vector unsigned char);
void altivec8bitPelProductsSumM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
                                  vector unsigned char, vector unsigned char);
void altivec8bitPelProductsSumIM ( const unsigned char *input1, const unsigned char *input2, rcCorr& res, unsigned int m16, 
                                   vector unsigned char, vector unsigned char);
#endif

// Instruction Snippets for AltiVec Implementation of Correlation


#define ACUMMOM1S \
   mom11 = vec_sums((vector signed int) mom11, (vector signed int) zero);\
   mom01 = vec_sums((vector signed int) mom01, (vector signed int) zero);\
   mom02 = vec_sums((vector signed int) mom02, (vector signed int) zero);\
   mom22 = vec_sums((vector signed int) mom22, (vector signed int) zero)

#define REPLMOMS \
   mom01 = vec_splat( mom01, 3 ); \
   mom11 = vec_splat( mom11, 3 ); \
   mom02 = vec_splat( mom02, 3 ); \
   mom22 = vec_splat( mom22, 3 )

#define STRMOMS \
   vec_ste( (vector unsigned int) mom11, 0, &sii); \
   vec_ste( (vector unsigned int) mom01, 0, &si ); \
   vec_ste( (vector unsigned int) mom02, 0, &sm); \
   vec_ste( (vector unsigned int) mom22, 0, &smm)

#define ACUMMOM1 \
   mom11 = (vector unsigned int) (vec_sums((vector signed int) mom11, (vector signed int) zero));\
   mom01 = (vector unsigned int) (vec_sums((vector signed int) mom01, (vector signed int) zero))

#define ACUMMOM2 \
   mom02 = (vector unsigned int) (vec_sums((vector signed int) mom02, (vector signed int) zero)); \
   mom22 = (vector unsigned int) (vec_sums((vector signed int) mom22, (vector signed int) zero))

#define REPLMOM1 \
   mom01 = vec_splat( mom01, 3 ); \
   mom11 = vec_splat( mom11, 3 ) 

#define REPLMOM2 \
   mom02 = vec_splat( mom02, 3 ); \
   mom22 = vec_splat( mom22, 3 )

#define STRMOM1 \
   vec_ste( mom11, 0, &sii); \
   vec_ste( mom01, 0, &si )

#define STRMOM2 \
   vec_ste( mom02, 0, &sm); \
   vec_ste( mom22, 0, &smm)

#define INITMOM1 \
   mom01 = (vector unsigned int)(0);\
   mom11 = (vector unsigned int)(0)

#define INITMOM2 \
   mom02 = (vector unsigned int)(0);\
   mom22 = (vector unsigned int)(0)

#define INITCOMMON \
    zero = (vector unsigned int)(0);\
    cross = (vector unsigned int)(0);\
    sumim = (vector unsigned int)(0)  

#define INITCOMMON16 \
   mom01 = (vector signed int)(0);\
   mom11 = (vector signed int)(0);\
   mom02 = (vector signed int)(0);\
   mom22 = (vector signed int)(0);\
    zero = (vector signed int)(0);\
    cross = (vector signed int)(0);\
    sumim = (vector signed int)(0)  



#define MAX_TILE_WIDTH 256
#define MAX_TILE_HEIGHT 256

      
   

#define UUCROSSSUMIM(i)                                                 \
    t2  = vec_perm(previous[i], previous[i+1], perm2 );/* align previous vector */ \
    mom11  = vec_msum(t1, t1, mom11 );      /* Sum of the products           */ \
    mom01  = vec_sum4s(t1, mom01);      /* Sum of the products           */ \
    cross  = vec_msum(t1, t2, cross );      /* Sum of the products           */ \
    t1  = vec_perm(current[i+1], current[i+2], perm1 );  /* align current vector  */ \
    mom22  = vec_msum(t2, t2, mom22 );      /* Sum of the products           */ \
    mom02  = vec_sum4s(t2, mom02);      /* Sum of the products           */ \
    if (i == m16) goto uaccum;

#define UUCROSSSUMI(i)                                                  \
    t2  = vec_perm(previous[i], previous[i+1], perm2 );/* align previous vector */ \
    mom11  = vec_msum(t1, t1, mom11 );      /* Sum of the products           */ \
    mom01  = vec_sum4s(t1, mom01);      /* Sum of the products           */ \
    cross  = vec_msum(t1, t2, cross );      /* Sum of the products           */ \
    t1  = vec_perm(current[i+1], current[i+2], perm1 );  /* align current vector  */ \
    if (i == m16) goto uaccum;

#define UUCROSSSUMM(i)                                                  \
    t1  = vec_perm(current[i], current[i+1], perm1 );  /* align current vector  */ \
    mom22  = vec_msum(t2, t2, mom22 );      /* Sum of the products           */ \
    mom02  = vec_sum4s(t2, mom02);      /* Sum of the products           */ \
    cross  = vec_msum(t1, t2, cross );      /* Sum of the products           */ \
    t2  = vec_perm(previous[i+1], previous[i+2], perm2 );/* align previous vector */ \
    if (i == m16) goto uaccum;

#define UUCROSS(i)                                                      \
    t1  = vec_perm(current[i], current[i+1], perm1 );  /* align current vector  */ \
    t2  = vec_perm(previous[i], previous[i+1], perm2 );/* align previous vector */ \
    cross  = vec_msum(t1, t2, cross );      /* Sum of the products           */ \
    if (i == m16) goto uaccum;


#define ALCROSSSUMIM(i)                                                 \
    t2  = previous[i];               /* align previous vector */        \
    mom11  = vec_msum(t1, t1, mom11 );      /* Sum of the products           */ \
    mom01  = vec_sum4s(t1, mom01);      /* Sum of the products           */ \
    cross  = vec_msum(t1, t2, cross );      /* Sum of the products           */ \
    t1  = current[i+1];                /* align current vector  */        \
    mom22  = vec_msum(t2, t2, mom22 );      /* Sum of the products           */ \
    mom02  = vec_sum4s(t2, mom02);      /* Sum of the products           */ \
    if (i == m16) goto accum;

#define ALCROSSSUMI(i)                                                  \
    t2  = previous[i];               /* align previous vector */        \
    mom11  = vec_msum(t1, t1, mom11 );      /* Sum of the products           */ \
    mom01  = vec_sum4s(t1, mom01);      /* Sum of the products           */ \
    cross  = vec_msum(t1, t2, cross );      /* Sum of the products           */ \
    t1  = current[i+1];                /* align current vector  */        \
    if (i == m16) goto accum;

#define ALCROSSSUMM(i)                                                  \
    t1  = current[i];                /* align current vector  */        \
    mom22  = vec_msum(t2, t2, mom22 );      /* Sum of the products           */ \
    mom02  = vec_sum4s(t2, mom02);      /* Sum of the products           */ \
    cross  = vec_msum(t1, t2, cross );      /* Sum of the products           */ \
    t2  = previous[i+1];               /* align previous vector */        \
    if (i == m16) goto accum;

#define ALCROSS                                                      \
    t1  = *current++;                /* align current vector  */        \
    t2  = *previous++;               /* align previous vector */        \
    cross  = vec_msum(t1, t2, cross );      /* Sum of the products           */ \


