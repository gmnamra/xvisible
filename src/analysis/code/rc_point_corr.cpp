/*
 *  rc_point_corr.cpp
 *
 *  Created by Peter Roberts on Tue Jun 24 2003.
 *  Copyright (c) 2003 Reify Corp. All rights reserved.
 *
 */



#include <stdio.h>
#include <rc_types.h>
#include <rc_point_corr.h>
#include <rc_analysis.h>

#ifdef __ppc__

typedef union{
  unsigned char c[16];
  signed char sc[16];
  unsigned short s[8];
  signed short ss[8];
  unsigned int l[4];
  signed int sl[4];
  float f[4];
  vector unsigned char vc;
  vector unsigned short vs;
  vector unsigned int vl;
  vector float vf;
}align;

enum{ BYTE, SHORT, LONG, FLOAT, HEX, DEC, NON };

#ifdef DEBUG_LOG
static void printVector(void * data, int type, int format)
{
  int     i;
  align   d;
  d.vc = *((vector unsigned char *)data);

  if ((type == BYTE) && (format == HEX))
    for (i = 0; i < 16; i++)
      printf("%.2x ", d.c[i]);
  else if ((type == BYTE) && (format == DEC))
    for (i = 0; i < 16; i++)
      printf("%4d ", d.sc[i]);
  else if ((type == SHORT) && (format == HEX))
    for (i = 0; i < 8; i++)
      printf("%.4x ", d.s[i]);
  else if ((type == SHORT) && (format == DEC))
    for (i = 0; i < 8; i++)
      printf("%7d ", d.ss[i]);
  else if ((type == LONG) && (format == HEX))
    for ( i = 0; i < 4; i++ )
      printf( "%.8x ", (int) d.l[i]);
  else if ((type == LONG) && (format == DEC))
    for (i = 0; i < 4; i++)
      printf("%10d ", (int) d.sl[i]);
  else if (type == FLOAT)
    for (i = 0; i < 4; i++)
      printf("%20e ", d.f[i]);
  printf("\n");
}
#endif

#define V0_UC (*((vector unsigned char*)&v0))
#define V0_UI (*((vector unsigned int*)&v0))
#define V0_SI (*((vector signed int*)&v0))
#define V0_F (*((vector float*)&v0))

#define T0_UC (*((vector unsigned char*)&t0))
#define T0_UI (*((vector unsigned int*)&t0))
#define T0_SI (*((vector signed int*)&t0))
#define T0_F (*((vector float*)&t0))

#define T1_UC (*((vector unsigned char*)&t1))
#define T1_UI (*((vector unsigned int*)&t1))
#define T1_SI (*((vector signed int*)&t1))
#define T1_F (*((vector float*)&t1))

/* Load macros for both image and model pixels. "Cross Boundary"
 * versions are for cases where the image/model pixels straddle a
 * modulo 16 boundary.
 *
 * Requires:
 *  "v0" = 0
 */
#define LOADI(IPTR,IDEST,IPERM)            { \
  T0_UC = *(IPTR);			     \
  (IDEST) = vec_perm(T0_UC, V0_UC, (IPERM)); \
}

#define LOADI_CB(IPTR,IDEST,IPERM)             { \
    T0_UC = *(IPTR); T1_UC = *((IPTR)+1);	 \
    ((IDEST) = vec_perm(T0_UC, T1_UC, (IPERM))); \
  }

#define LOADM(MPTR,MDEST,MPERM,D0)             { \
    T0_UC = *(MPTR);			         \
    ((MDEST) = vec_perm(T0_UC, V0_UC, (MPERM))); \
  }

#define LOADM_CB(MPTR,MDEST,MPERM1,MPERM2)    { \
    (MDEST) = *(MPTR); T1_UC = *((MPTR)+1);     \
    T0_UC = vec_perm((MDEST), T1_UC, (MPERM2)); \
    (MDEST) = vec_perm(T0_UC, V0_UC, (MPERM1)); }

/* GEN_COL5 - For the given image line and set of model lines,
 * generate cross products for one column position per line.
 */
#define GEN_COL5(I, M1, M2, M3, M4, M5, IM1, IM2, IM3,		\
		 IM4, IM5, MCNT, RPERM, DORPERM, SHIFT) {	\
    rmAssert((MCNT) && ((MCNT) < 6));				\
    T0_UI = vec_msum((I), (M1), V0_UI);				\
    if (DORPERM) {						\
      T0_SI = vec_sums(T0_SI, (IM1));				\
      (IM1) = vec_perm(T0_SI, (IM1), (RPERM));			\
    }								\
    else							\
      (IM1) = vec_sums(T0_SI, (IM1));				\
    if ((MCNT) > 1) {						\
      T0_UI = vec_msum((I), (M2), V0_UI);			\
      if (DORPERM) {						\
	T0_SI = vec_sums(T0_SI, (IM2));				\
	(IM2) = vec_perm(T0_SI, (IM2), (RPERM));		\
      }								\
      else							\
	(IM2) = vec_sums(T0_SI, (IM2));				\
    }								\
    if ((MCNT) > 2) {						\
      T0_UI = vec_msum((I), (M3), V0_UI);			\
      if (DORPERM) {						\
	T0_SI = vec_sums(T0_SI, (IM3));				\
	(IM3) = vec_perm(T0_SI, (IM3), (RPERM));		\
      }								\
      else							\
	(IM3) = vec_sums(T0_SI, (IM3));				\
    }								\
    if ((MCNT) > 3) {						\
      T0_UI = vec_msum((I), (M4), V0_UI);			\
      if (DORPERM) {						\
	T0_SI = vec_sums(T0_SI, (IM4));				\
	(IM4) = vec_perm(T0_SI, (IM4), (RPERM));		\
      }								\
      else							\
	(IM4) = vec_sums(T0_SI, (IM4));				\
    }								\
    if ((MCNT) > 4) {						\
      T0_UI = vec_msum((I), (M5), V0_UI);			\
      if (DORPERM) {						\
	T0_SI = vec_sums(T0_SI, (IM5));				\
	(IM5) = vec_perm(T0_SI, (IM5), (RPERM));		\
      }								\
      else							\
	(IM5) = vec_sums(T0_SI, (IM5));				\
    }								\
    if (SHIFT)							\
      (I) = vec_sld((I), V0_UC, 1);				\
  }

/* GEN_ROW5 - For the given image line and set of model lines,
 * generate cross products for all columns in the row.
 */
#define GEN_ROW5(I, M1, M2, M3, M4, M5, IM1_14, IM1_5, IM2_14,		\
		IM2_5, IM3_14, IM3_5, IM4_14, IM4_5, IM5_14,		\
		IM5_5, MCNT, RPERM) {					\
    GEN_COL5((I), (M1), (M2), (M3), (M4), (M5), (IM1_14), (IM2_14),	\
	     (IM3_14), (IM4_14), (IM5_14), (MCNT), (RPERM), true, true);\
    GEN_COL5((I), (M1), (M2), (M3), (M4), (M5), (IM1_14), (IM2_14),	\
	     (IM3_14), (IM4_14), (IM5_14), (MCNT), (RPERM), true, true);\
    GEN_COL5((I), (M1), (M2), (M3), (M4), (M5), (IM1_14), (IM2_14),	\
	     (IM3_14), (IM4_14), (IM5_14), (MCNT), (RPERM), true, true);\
    GEN_COL5((I), (M1), (M2), (M3), (M4), (M5), (IM1_14), (IM2_14),	\
	     (IM3_14), (IM4_14), (IM5_14), (MCNT), (RPERM), true, true);\
    GEN_COL5((I), (M1), (M2), (M3), (M4), (M5), (IM1_5), (IM2_5),	\
	     (IM3_5), (IM4_5), (IM5_5), (MCNT), (RPERM), false, false); \
  }

#define GEN_LRG_5X5_SPACE(ILOAD, MLOAD, MHGT, LBL) {		   \
    vector signed int im1_14, im1_5, im2_14, im2_5, im3_14, im3_5; \
    vector signed int im4_14, im4_5, im5_14, im5_5;		   \
    vector unsigned char curI, m1, m2, m3, m4, m5;		   \
    im1_14 = vec_splat_s32(0);					   \
    im1_5 = vec_splat_s32(0);					   \
    im2_14 = vec_splat_s32(0);					   \
    im2_5 = vec_splat_s32(0);					   \
    im3_14 = vec_splat_s32(0);					   \
    im3_5 = vec_splat_s32(0);					   \
    im4_14 = vec_splat_s32(0);					   \
    im4_5 = vec_splat_s32(0);					   \
    im5_14 = vec_splat_s32(0);					   \
    im5_5 = vec_splat_s32(0);					   \
    rmAssert((MHGT >= 5) && (MHGT <= 12));			   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);			   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    GEN_ROW5(curI, m1, m5, m4, m3, m2, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 1, rotatePerm);				   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    MLOAD(mPtr, m2, mModelPerm, mAlignPerm);			   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    GEN_ROW5(curI, m2, m1, m5, m4, m3, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 2, rotatePerm);				   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    MLOAD(mPtr, m3, mModelPerm, mAlignPerm);			   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    GEN_ROW5(curI, m3, m2, m1, m5, m4, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 3, rotatePerm);				   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    MLOAD(mPtr, m4, mModelPerm, mAlignPerm);			   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    GEN_ROW5(curI, m4, m3, m2, m1, m5, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 4, rotatePerm);				   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    MLOAD(mPtr, m5, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m5, m4, m3, m2, m1, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 5, rotatePerm);				   \
    switch (MHGT) { /*Permute model data and start processing */   \
    case 0:							   \
    case 1:							   \
    case 2:							   \
    case 3:							   \
    case 4:							   \
    case 5:							   \
      m1 = m4;							   \
      m4 = m2;							   \
      m2 = m5;							   \
      m5 = m3;							   \
      goto LBL##5x5procResults;					   \
    case 6:							   \
      m1 = m5;							   \
      m5 = m4;							   \
      m4 = m3;							   \
      m3 = m2;							   \
      goto LBL##5x5proc6;					   \
    case 7:							   \
      goto LBL##5x5proc7;					   \
    case 8:							   \
      m1 = m2;							   \
      m2 = m3;							   \
      m3 = m4;							   \
      m4 = m5;							   \
      goto LBL##5x5proc8;					   \
    case 9:							   \
      m1 = m3;							   \
      m3 = m5;							   \
      m5 = m2;							   \
      m2 = m4;							   \
      goto LBL##5x5proc9;					   \
    case 10:							   \
      m1 = m4;							   \
      m4 = m2;							   \
      m2 = m5;							   \
      m5 = m3;							   \
      goto LBL##5x5proc10;					   \
    case 11:							   \
      m1 = m5;							   \
      m5 = m4;							   \
      m4 = m3;							   \
      m3 = m2;							   \
      goto LBL##5x5proc11;					   \
    case 12:							   \
      goto LBL##5x5proc12;					   \
    }								   \
LBL##5x5proc12:						           \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m1, m5, m4, m3, m2, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 5, rotatePerm);				   \
LBL##5x5proc11:						           \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    MLOAD(mPtr, m2, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m2, m1, m5, m4, m3, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 5, rotatePerm);				   \
LBL##5x5proc10:						           \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    MLOAD(mPtr, m3, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m3, m2, m1, m5, m4, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 5, rotatePerm);				   \
LBL##5x5proc9:						           \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    MLOAD(mPtr, m4, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m4, m3, m2, m1, m5, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 5, rotatePerm);				   \
LBL##5x5proc8:						           \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    MLOAD(mPtr, m5, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m5, m4, m3, m2, m1, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 5, rotatePerm);				   \
LBL##5x5proc7:						           \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m1, m5, m4, m3, m2, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 5, rotatePerm);				   \
LBL##5x5proc6:						           \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
    MLOAD(mPtr, m2, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m2, m1, m5, m4, m3, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 5, rotatePerm);				   \
LBL##5x5procResults:					           \
    *destP++ = im1_5;						   \
    *destP++ = im1_14;						   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    GEN_ROW5(curI, m2, m1, m5, m4, m3, im2_14, im2_5, im3_14,	   \
	     im3_5, im4_14, im4_5, im5_14, im5_5, im1_14,	   \
	     im1_5, 4, rotatePerm);				   \
    *destP++ = im2_5;						   \
    *destP++ = im2_14;						   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    GEN_ROW5(curI, m2, m1, m5, m4, m3, im3_14, im3_5, im4_14,	   \
	     im4_5, im5_14, im5_5, im1_14, im1_5, im2_14,	   \
	     im2_5, 3, rotatePerm);				   \
    *destP++ = im3_5;						   \
    *destP++ = im3_14;						   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    GEN_ROW5(curI, m2, m1, m5, m4, m3, im4_14, im4_5, im5_14,	   \
	     im5_5, im1_14, im1_5, im2_14, im2_5, im3_14,	   \
	     im3_5, 2, rotatePerm);				   \
    *destP++ = im4_5;						   \
    *destP++ = im4_14;						   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    GEN_ROW5(curI, m2, m1, m5, m4, m3, im5_14, im5_5, im1_14,	   \
	     im1_5, im2_14, im2_5, im3_14, im3_5, im4_14,	   \
	     im4_5, 1, rotatePerm);				   \
    *destP++ = im5_5;						   \
    *destP++ = im5_14;						   \
  }

#define GEN_SML_5X5_SPACE(ILOAD, MLOAD, MHGT) {			   \
    vector signed int im1_14, im1_5, im2_14, im2_5, im3_14, im3_5; \
    vector signed int im4_14, im4_5, im5_14, im5_5;		   \
    vector unsigned char curI, m1, m2, m3, m4, md;		   \
    im1_14 = vec_splat_s32(0);					   \
    im1_5 = vec_splat_s32(0);					   \
    im2_14 = vec_splat_s32(0);					   \
    im2_5 = vec_splat_s32(0);					   \
    im3_14 = vec_splat_s32(0);					   \
    im3_5 = vec_splat_s32(0);					   \
    im4_14 = vec_splat_s32(0);					   \
    im4_5 = vec_splat_s32(0);					   \
    im5_14 = vec_splat_s32(0);					   \
    im5_5 = vec_splat_s32(0);					   \
    rmAssert((MHGT >= 1) && (MHGT <= 4));			   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);			   \
    GEN_ROW5(curI, m1, md, m4, m3, m2, im1_14, im1_5, im2_14,	   \
	    im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	    im5_5, 1, rotatePerm);				   \
    if ((MHGT) == 1) {						   \
      *destP++ = im1_5;						   \
      *destP++ = im1_14;					   \
      ILOAD(iPtr, curI, iAlignPerm);				   \
      iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
      GEN_ROW5(curI, m1, md, m4, m3, m2, im2_14, im2_5, im3_14,	   \
	       im3_5, im4_14, im4_5, im5_14, im5_5, im1_14,	   \
	       im1_5, 1, rotatePerm);				   \
      *destP++ = im2_5;						   \
      *destP++ = im2_14;					   \
      ILOAD(iPtr, curI, iAlignPerm);				   \
      iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
      GEN_ROW5(curI, m1, md, m4, m3, m2, im3_14, im3_5, im4_14,	   \
	       im4_5, im5_14, im5_5, im1_14, im1_5, im2_14,	   \
	       im2_5, 1, rotatePerm);				   \
      *destP++ = im3_5;						   \
      *destP++ = im3_14;					   \
      ILOAD(iPtr, curI, iAlignPerm);				   \
      GEN_ROW5(curI, m1, md, m4, m3, m2, im4_14, im4_5, im5_14,	   \
	       im5_5, im1_14, im1_5, im2_14, im2_5, im3_14,	   \
	       im3_5, 1, rotatePerm);				   \
      *destP++ = im4_5;						   \
      *destP++ = im4_14;					   \
    } else { /* MHGT > 1 */					   \
      ILOAD(iPtr, curI, iAlignPerm);				   \
      iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
      mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);	   \
      MLOAD(mPtr, m2, mModelPerm, mAlignPerm);			   \
      GEN_ROW5(curI, m2, m1, md, m4, m3, im1_14, im1_5, im2_14,	   \
	       im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
	       im5_5, 2, rotatePerm);				   \
      if ((MHGT) == 2) {					   \
	*destP++ = im1_5;					   \
	*destP++ = im1_14;					   \
	ILOAD(iPtr, curI, iAlignPerm);				   \
	iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);   \
	GEN_ROW5(curI, m2, m1, md, m4, m3, im2_14, im2_5,	   \
		 im3_14, im3_5, im4_14, im4_5, im5_14,		   \
		 im5_5, im1_14, im1_5, 2, rotatePerm);		   \
	*destP++ = im2_5;					   \
	*destP++ = im2_14;					   \
	ILOAD(iPtr, curI, iAlignPerm);				   \
	GEN_ROW5(curI, m2, m1, md, m4, m3, im3_14, im3_5,	   \
		 im4_14, im4_5, im5_14, im5_5, im1_14,		   \
		 im1_5, im2_14, im2_5, 2, rotatePerm);		   \
	*destP++ = im3_5;					   \
	*destP++ = im3_14;					   \
      } else { /* MHGT > 2 */					   \
	ILOAD(iPtr, curI, iAlignPerm);				   \
	iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);   \
	mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);   \
	MLOAD(mPtr, m3, mModelPerm, mAlignPerm);		   \
	GEN_ROW5(curI, m3, m2, m1, md, m4, im1_14, im1_5, im2_14,  \
		 im2_5, im3_14, im3_5, im4_14, im4_5, im5_14,	   \
		 im5_5, 3, rotatePerm);				   \
	if ((MHGT) == 3) {					   \
	  *destP++ = im1_5;					   \
	  *destP++ = im1_14;					   \
	  ILOAD(iPtr, curI, iAlignPerm);			   \
	  GEN_ROW5(curI, m3, m2, m1, md, m4, im2_14, im2_5,	   \
		   im3_14, im3_5, im4_14, im4_5, im5_14,	   \
		   im5_5, im1_14, im1_5, 3, rotatePerm);	   \
	  *destP++ = im2_5;					   \
	  *destP++ = im2_14;					   \
	} else { /* MHGT == 4 */				   \
	  ILOAD(iPtr, curI, iAlignPerm);			   \
	  iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate); \
	  mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate); \
	  MLOAD(mPtr, m4, mModelPerm, mAlignPerm);		   \
	  GEN_ROW5(curI, m4, m3, m2, m1, md, im1_14, im1_5,	   \
		   im2_14, im2_5, im3_14, im3_5, im4_14, im4_5,	   \
		   im5_14, im5_5, 4, rotatePerm);		   \
	  *destP++ = im1_5;					   \
	  *destP++ = im1_14;					   \
	  ILOAD(iPtr, curI, iAlignPerm);			   \
	  GEN_ROW5(curI, m4, m3, m2, m1, md, im2_14, im2_5,	   \
		   im3_14, im3_5, im4_14, im4_5, im5_14, im5_5,	   \
		   im1_14, im1_5, 4, rotatePerm);		   \
	  *destP++ = im2_5;					   \
	  *destP++ = im2_14;					   \
	  m1 = m2;						   \
	  m2 = m3;						   \
	  m3 = m4;						   \
	} /* End of: else { MHGT == 4 */			   \
	iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);   \
	ILOAD(iPtr, curI, iAlignPerm);				   \
	GEN_ROW5(curI, m3, m2, m1, md, m4, im3_14, im3_5, im4_14,  \
		 im4_5, im5_14, im5_5, im1_14, im1_5, im2_14,	   \
		 im2_5, 3, rotatePerm);				   \
	*destP++ = im3_5;					   \
	*destP++ = im3_14;					   \
	m1 = m2;						   \
	m2 = m3;						   \
      } /* End of: else { MHGT > 2 */				   \
      iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
      ILOAD(iPtr, curI, iAlignPerm);				   \
      GEN_ROW5(curI, m2, m1, md, m4, m3, im4_14, im4_5, im5_14,	   \
	       im5_5, im1_14, im1_5, im2_14, im2_5, im3_14,	   \
	       im3_5, 2, rotatePerm);				   \
      *destP++ = im4_5;						   \
      *destP++ = im4_14;					   \
      m1 = m2;							   \
    } /* End of: else { MHGT > 1 */				   \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);	   \
    ILOAD(iPtr, curI, iAlignPerm);				   \
    GEN_ROW5(curI, m1, md, m4, m3, m2, im5_14, im5_5, im1_14,	   \
	     im1_5, im2_14, im2_5, im3_14, im3_5, im4_14,	   \
	     im4_5, 1, rotatePerm);				   \
    *destP++ = im5_5;						   \
    *destP++ = im5_14;						   \
  }

/* GEN_COL3 - For the given image line and set of model lines, generate
 * cross products for one column position per line.
 */
#define GEN_COL3(I, M1, M2, M3, IM1, IM2, IM3,	 \
		 MCNT, RPERM, DORPERM, SHIFT) {	 \
    rmAssert((MCNT) && ((MCNT) < 4));		 \
    T0_UI = vec_msum((I), (M1), V0_UI);		 \
    if (DORPERM) {				 \
      T0_SI = vec_sums(T0_SI, (IM1));		 \
      (IM1) = vec_perm(T0_SI, (IM1), (RPERM));	 \
    }						 \
    else					 \
      (IM1) = vec_sums(T0_SI, (IM1));		 \
    if ((MCNT) > 1) {				 \
      T0_UI = vec_msum((I), (M2), V0_UI);	 \
      if (DORPERM) {				 \
	T0_SI = vec_sums(T0_SI, (IM2));		 \
	(IM2) = vec_perm(T0_SI, (IM2), (RPERM)); \
      }						 \
      else					 \
	(IM2) = vec_sums(T0_SI, (IM2));		 \
    }						 \
    if ((MCNT) > 2) {				 \
      T0_UI = vec_msum((I), (M3), V0_UI);	 \
      if (DORPERM) {				 \
	T0_SI = vec_sums(T0_SI, (IM3));		 \
	(IM3) = vec_perm(T0_SI, (IM3), (RPERM)); \
      }						 \
      else					 \
	(IM3) = vec_sums(T0_SI, (IM3));		 \
    }						 \
    if (SHIFT)					 \
      (I) = vec_sld((I), V0_UC, 1);		 \
  }

/* GEN_ROW3 - For the given image line and set of model lines,
 * generate cross products for all columns in the row.
 */
#define GEN_ROW3(I, M1, M2, M3, IM1_13, IM2_13, IM3_13, MCNT, RPERM) { \
    GEN_COL3((I), (M1), (M2), (M3), (IM1_13), (IM2_13), (IM3_13),      \
	     (MCNT), (RPERM), true, true);			       \
    GEN_COL3((I), (M1), (M2), (M3), (IM1_13), (IM2_13), (IM3_13),      \
	     (MCNT), (RPERM), true, true);			       \
    GEN_COL3((I), (M1), (M2), (M3), (IM1_13), (IM2_13), (IM3_13),      \
	     (MCNT), (RPERM), true, false);			       \
    (IM1_13) = vec_perm(V0_SI, (IM1_13), (RPERM));		       \
    (IM2_13) = vec_perm(V0_SI, (IM2_13), (RPERM));		       \
    (IM3_13) = vec_perm(V0_SI, (IM3_13), (RPERM));		       \
  }

#define GEN_LRG_3X3_SPACE(ILOAD, MLOAD, MHGT, LBL) {			\
    vector signed int im1_13, im2_13, im3_13;				\
    vector unsigned char curI, m1, m2, m3;				\
    im1_13 = vec_splat_s32(0);						\
    im2_13 = vec_splat_s32(0);						\
    im3_13 = vec_splat_s32(0);						\
    rmAssert((MHGT >= 3) && (MHGT <= 14));				\
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);				\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    GEN_ROW3(curI, m1, m3, m2, im1_13, im2_13, im3_13, 1, rotatePerm);	\
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    MLOAD(mPtr, m2, mModelPerm, mAlignPerm);				\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    GEN_ROW3(curI, m2, m1, m3, im1_13, im2_13, im3_13, 2, rotatePerm);	\
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    MLOAD(mPtr, m3, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m3, m2, m1, im1_13, im2_13, im3_13, 3, rotatePerm);	\
    switch (MHGT) { /* Permute model data and start processing */	\
    case 0:								\
    case 1:								\
    case 2:								\
    case 3:								\
      m1 = m2;								\
      m2 = m3;								\
      goto LBL##3x3procResults;						\
    case 4:								\
      m1 = m3;								\
      m3 = m2;								\
      goto LBL##3x3proc4;						\
    case 5:								\
      goto LBL##3x3proc5;						\
    case 6:								\
      m1 = m2;								\
      m2 = m3;								\
      goto LBL##3x3proc6;						\
    case 7:								\
      m1 = m3;								\
      m3 = m2;								\
      goto LBL##3x3proc7;						\
    case 8:								\
      goto LBL##3x3proc8;						\
    case 9:								\
      m1 = m2;								\
      m2 = m3;								\
      goto LBL##3x3proc9;						\
    case 10:								\
      m1 = m3;								\
      m3 = m2;								\
      goto LBL##3x3proc10;						\
    case 11:								\
      goto LBL##3x3proc11;						\
    case 12:								\
      m1 = m2;								\
      m2 = m3;								\
      goto LBL##3x3proc12;						\
    case 13:								\
      m1 = m3;								\
      m3 = m2;								\
      goto LBL##3x3proc13;						\
    case 14:								\
      goto LBL##3x3proc14;						\
    }									\
LBL##3x3proc14:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m1, m3, m2, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc13:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m2, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m2, m1, m3, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc12:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m3, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m3, m2, m1, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc11:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m1, m3, m2, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc10:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m2, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m2, m1, m3, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc9:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m3, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m3, m2, m1, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc8:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m1, m3, m2, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc7:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m2, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m2, m1, m3, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc6:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m3, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m3, m2, m1, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc5:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m1, m3, m2, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3proc4:						                \
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		\
    MLOAD(mPtr, m2, mModelPerm, mAlignPerm);				\
    GEN_ROW3(curI, m2, m1, m3, im1_13, im2_13, im3_13, 3, rotatePerm);	\
LBL##3x3procResults:					                \
    *destP++ = im1_13;							\
    ILOAD(iPtr, curI, iAlignPerm);					\
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		\
    GEN_ROW3(curI, m2, m1, m3, im2_13, im3_13, im1_13, 2, rotatePerm);	\
    *destP++ = im2_13;							\
    ILOAD(iPtr, curI, iAlignPerm);					\
    GEN_ROW3(curI, m2, m1, m3, im3_13, im1_13, im2_13, 1, rotatePerm);	\
    *destP++ = im3_13;							\
  }

#define GEN_SML_3X3_SPACE(ILOAD, MLOAD, MHGT) {				 \
    vector signed int im1_13, im2_13, im3_13;				 \
    vector unsigned char curI, m1, m2, md;				 \
    im1_13 = vec_splat_s32(0);						 \
    im2_13 = vec_splat_s32(0);						 \
    im3_13 = vec_splat_s32(0);						 \
    rmAssert((MHGT >= 1) && (MHGT <= 2));				 \
    ILOAD(iPtr, curI, iAlignPerm);					 \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		 \
    MLOAD(mPtr, m1, mModelPerm, mAlignPerm);				 \
    GEN_ROW3(curI, m1, md, m2, im1_13, im2_13, im3_13, 1, rotatePerm);	 \
    if (MHGT == 1) {							 \
      *destP++ = im1_13;						 \
      ILOAD(iPtr, curI, iAlignPerm);					 \
      GEN_ROW3(curI, m1, md, m2, im2_13, im3_13, im1_13, 1, rotatePerm); \
      *destP++ = im2_13;						 \
    } else {								 \
      ILOAD(iPtr, curI, iAlignPerm);					 \
      iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		 \
      mPtr = (vector unsigned char*)((int)mPtr + _mRowUpdate);		 \
      MLOAD(mPtr, m2, mModelPerm, mAlignPerm);				 \
      GEN_ROW3(curI, m2, m1, md, im1_13, im2_13, im3_13, 2, rotatePerm); \
      *destP++ = im1_13;						 \
      ILOAD(iPtr, curI, iAlignPerm);					 \
      GEN_ROW3(curI, m2, m1, md, im2_13, im3_13, im1_13, 2, rotatePerm); \
      *destP++ = im2_13;						 \
      m1 = m2;								 \
    }									 \
    iPtr = (vector unsigned char*)((int)iPtr + _iRowUpdate);		 \
    ILOAD(iPtr, curI, iAlignPerm);					 \
    GEN_ROW3(curI, m1, md, m2, im3_13, im1_13, im2_13, 1, rotatePerm);	 \
    *destP++ = im3_13;							 \
  }

#endif

static const int32 defDestSpcSz = 1024;

rcPointCorrelation::rcPointCorrelation() :
  _iRowBase(0), _mRowBase(0), _iRowUpdate(0), _mRowUpdate(0), _imgMom(0),
  _mdlMom(0), _destSpaceBase(0), _destSpacePtr(0), _destSpaceSz(defDestSpcSz)
{
  rmAssert(_destSpaceSz >= (int32)(5*8*sizeof(float) + 32 + 32 + 16));
  _destSpaceBase = (unsigned char*)malloc(_destSpaceSz);
  _vectorSumPtr = (uint32*)(((int)_destSpaceBase + 15) & 0xFFFFFFF0);
  _vectorSumSqPtr = _vectorSumPtr + 8;
  _resultPtr =  _vectorSumSqPtr + 8; 
  _destSpacePtr = _resultPtr + 4;
}

rcPointCorrelation::~rcPointCorrelation()
{
  if (_destSpaceBase) {
    free(_destSpaceBase);
    _destSpaceBase = 0;
  }
}

void rcPointCorrelation::update(rcWindow imgWin, rcWindow mdlWin,
				const rcMomentGenerator& imgMom,
				const rcMomentGenerator& mdlMom)
{
  rmAssert(imgMom._type == rcMomentGenerator::eMoment2D);
  rmAssert(mdlMom._type == rcMomentGenerator::eMoment2D);
  rmAssert(VALID_2D_MOM_INT_WIDTH(imgWin.width()) == imgMom._imgSq.width());
  rmAssert(VALID_2D_MOM_INT_HEIGHT(imgWin.height())  == imgMom._imgSq.height());
  rmAssert(VALID_2D_MOM_INT_WIDTH(mdlWin.width()) == mdlMom._imgSq.width());
  rmAssert(VALID_2D_MOM_INT_HEIGHT(mdlWin.height())  == mdlMom._imgSq.height());

  _imgWin = imgWin;
  _iRowBase = _imgWin.rowPointer(0);
  _iRowUpdate = _imgWin.rowUpdate();
  _mdlWin = mdlWin;
  _mRowBase = _mdlWin.rowPointer(0);
  _mRowUpdate = _mdlWin.rowUpdate();
  _imgMom = &imgMom;
  _mdlMom = &mdlMom;
}

void rcPointCorrelation::update(rcWindow imgWin, rcWindow mdlWin)
{
  rmAssert(imgWin.isBound());
  rmAssert((mdlWin.height()));

  _imgWin = imgWin;
  _iRowBase = _imgWin.rowPointer(0);
  _iRowUpdate = _imgWin.rowUpdate();
  _mdlWin = mdlWin;
  _mRowBase = _mdlWin.rowPointer(0);
  _mRowUpdate = _mdlWin.rowUpdate();
  _imgMom = _mdlMom = 0;
}

void rcPointCorrelation::gen3by3Space(const rcRect& srchSpace,
				      const rcIPair& modelOrigin, 
				      rsCorr3by3Point& res) const
{
#ifndef __ppc__
   	vector<vector<float> > results (3);
	for (uint32 i = 0; i < 3; i++)
			results[i].resize (3);

		genSpace(srchSpace, modelOrigin, results, 1);
		for (uint32 j = 0; j < 3; j++)
			for (uint32 i = 0; i < 3; i++)
			res.score[j][i] = results[j][i];
#else
	
  /* Validate the current state of the object and the inputs.
   * Calculate model and image position and size.
   */
  rmAssert(_imgMom);
  rmAssert(_mdlMom);

  const int32 xImgOrigin = srchSpace.x();
  const int32 yImgOrigin = srchSpace.y();
  const int32 imgWidth = srchSpace.width();
  const int32 imgHeight = srchSpace.height();
  rmAssert(imgWidth <= 16);
  rmAssert(imgHeight <= 16);

  const int32 xSpcBound = xImgOrigin + imgWidth;
  const int32 ySpcBound = yImgOrigin + imgHeight;
  rmAssert(xImgOrigin < xSpcBound);
  rmAssert(xSpcBound <= _imgWin.width());
  rmAssert(yImgOrigin < ySpcBound);
  rmAssert(ySpcBound <= _imgWin.height());

  /* For 3x3 correlation space, model dimensions are always (-2,-2)
   * of the search space dimensions.
   */
  const int32 mdlWidth = imgWidth - 2;
  const int32 mdlHeight = imgHeight - 2;
  rmAssert(mdlWidth > 0);
  rmAssert(mdlHeight > 0);
  const int32 xMdlOrigin = modelOrigin.x();
  const int32 yMdlOrigin = modelOrigin.y();
  const int32 xMdlBound = xMdlOrigin + mdlWidth;
  const int32 yMdlBound = yMdlOrigin + mdlHeight;
  rmAssert(xMdlOrigin < xMdlBound);
  rmAssert(xMdlBound <= _mdlWin.width());
  rmAssert(yMdlOrigin < yMdlBound);
  rmAssert(yMdlBound <= _mdlWin.height());

  /* Request is valid. Calculate IM values for all the locations
   * within the search space.
   */
  vector short t0, t1, v0 = vec_splat_s16(0);
  vector unsigned char mModelPerm, mAlignPerm, iAlignPerm, rotatePerm;

  unsigned char* img =
    (unsigned char*)(_iRowBase + _iRowUpdate*yImgOrigin + xImgOrigin);
  vector unsigned char* iPtr =  (vector unsigned char*)img;
  unsigned char* mdl =
    (unsigned char*)(_mRowBase + _mRowUpdate*yMdlOrigin + xMdlOrigin);
  vector unsigned char* mPtr = (vector unsigned char*)mdl;
  vector signed int* destP = (vector signed int*)_destSpacePtr;

  const bool icb = (((int)iPtr & 0xF) + imgWidth) > 16;
  const bool mcb = (((int)mPtr & 0xF) + mdlWidth) > 16;

  /* Note: In the following comments, sample values are given based on the
   * assumption that:
   *
   *    mdlWidth = 5
   *    For mcb == true case: Address of mdl == 12 MOD 16
   *    For mcb == false case: Address of mdl == 10 MOD 16
   */
  iAlignPerm = vec_lvsl(0, img);
  T0_UC = vec_splat_u8(3);               // T0= 03030303030303030303030303030303
  T0_UC = vec_sl(T0_UC, T0_UC);          // T0= 18181818181818181818181818181818
  T1_UC = vec_lvsr(mdlWidth, (int*)0);   // T1= 0B0C0D0E0F101112131415161718191A
  T0_UC = vec_perm(V0_UC, T0_UC, T1_UC); // T0= 00000000001818181818181818181818
  if (mcb) {
    mAlignPerm = vec_lvsl(0, mdl);       // AP= 0C0D0E0F101112131415161718191A1B
    T1_UC = vec_lvsl(0, (int*)0);        // T1= 000102030405060708090A0B0C0D0E0F
    mModelPerm = vec_or(T1_UC, T0_UC);   // MP= 00010203041X1X1X1X1X1X1X1X1X1X1X
  }
  else {
    mAlignPerm = vec_splat_u8(0);        // Quiet compiler
    T1_UC = vec_lvsl(0, mdl);            // T1= 0A0B0C0D0E0F10111213141516171819
    mModelPerm = vec_or(T1_UC, T0_UC);   // MP= 0A0B0C0D0E1X1X1X1X1X1X1X1X1X1X1X
  }

  rotatePerm = vec_lvsr(4,(int*)0);
  
  if (icb) {
    if (mcb) {
      if (mdlHeight > 2) {
	GEN_LRG_3X3_SPACE(LOADI_CB, LOADM_CB, mdlHeight, ICBMCB);
      }
      else {
	GEN_SML_3X3_SPACE(LOADI_CB, LOADM_CB, mdlHeight);
      }
    }
    else {
      if (mdlHeight > 2) {
	GEN_LRG_3X3_SPACE(LOADI_CB, LOADM, mdlHeight, ICBM);
      }
      else {
	GEN_SML_3X3_SPACE(LOADI_CB, LOADM, mdlHeight);
      }
    }
  }
  else {
    if (mcb) {
      if (mdlHeight > 2) {
	GEN_LRG_3X3_SPACE(LOADI, LOADM_CB, mdlHeight, IMCB);
      }
      else {
	GEN_SML_3X3_SPACE(LOADI, LOADM_CB, mdlHeight);
      }
    }
    else {
      if (mdlHeight > 2) {
	GEN_LRG_3X3_SPACE(LOADI, LOADM, mdlHeight, IM);
      }
      else {
	GEN_SML_3X3_SPACE(LOADI, LOADM, mdlHeight);
      }
    }
  } // End of: if (icb) ... else ...

  /* Now calculate correlation scores for model and image at all
   * locations within the search space.
   */
  {
    vector unsigned short N, sumM;
    vector float varM, varI, varIM, vf1;

    T0_UI = vec_splat_u32(1);
    vf1 = vec_ctf(T0_UI, 0);

    /* Calculate and store N in vector.
     */
    *(uint16*)_vectorSumPtr = (uint16)(mdlWidth*mdlHeight);
    N = *(vector unsigned short*)_vectorSumPtr;
    N = vec_splat(N, 0);

    /* Calculate and store sumM and varM in vectors.
     */
    {
      const int32 x0 = xMdlOrigin*3;
      const int32 y0 = yMdlOrigin;
      const int32 x1 = x0 + mdlWidth*3;
      const int32 y1 = y0 + mdlHeight;
      
      const uint16 iSumM =
	(uint16)(*(_mdlMom->imgSumPtr(x1, y1)) - *(_mdlMom->imgSumPtr(x0, y1)) +
		   *(_mdlMom->imgSumPtr(x0, y0)) - *(_mdlMom->imgSumPtr(x1, y0)));

      *(uint16*)_vectorSumPtr = iSumM;
      sumM = *(vector unsigned short*)_vectorSumPtr;
      sumM = vec_splat(sumM, 0);

      vector unsigned int sumMM, sumMSq;

      const uint32 iSumMM =
	*(_mdlMom->imgSumSqPtrLow(x1+1,y1)) - *(_mdlMom->imgSumSqPtrLow(x0+1,y1)) +
	*(_mdlMom->imgSumSqPtrLow(x0+1,y0)) - *(_mdlMom->imgSumSqPtrLow(x1+1,y0));

      *_vectorSumPtr = iSumMM;
      sumMM = *(vector unsigned int*)_vectorSumPtr;
      sumMM = vec_splat(sumMM, 0);
   
      /* Calculate (N*SumMM - SumM*SumM).
       *
       * Note that SumM always fits in 16 bits, so the multiply can be
       * done in a single instruction. SumMM can go over 16 bits, so
       * the multiply is done as a pair of 16 bit multiplies, a 16 bit
       * shift, and an add.
       */
      sumMSq = vec_mulo(sumM, sumM);
      T0_UI = vec_mule(N, *(vector unsigned short*)&sumMM);
      T0_UI = vec_sld(T0_UI, V0_UI, 2);
      sumMM = vec_mulo(N, *(vector unsigned short*)&sumMM);
      sumMM = vec_add(sumMM, T0_UI);

      T0_UI = vec_sub(sumMM, sumMSq); // T0 = (N*SumMM - SumM*SumM).
      varM = vec_ctf(T0_UI, 0);
    }

    /* At each location within the search space, calculate varI, varIM
     * and use these values to calculate the final correlation scores.
     */
    for (int32 yo = 0; yo < 3; yo++) {
      const int32 y0 = (yImgOrigin + yo);
      const int32 y1 = y0 + mdlHeight;
      for (int32 xo = 0; xo < 3; xo++) {
	const int32 x0 = (xImgOrigin + xo)*3;
	const int32 x1 = x0 + mdlWidth*3;
	
	const uint32 iSumI =
	  *(_imgMom->imgSumPtr(x1, y1)) - *(_imgMom->imgSumPtr(x0, y1)) +
	  *(_imgMom->imgSumPtr(x0, y0)) - *(_imgMom->imgSumPtr(x1, y0));
	
	_vectorSumPtr[3 - xo] = iSumI;
	
	const uint32 iSumII =
	  *(_imgMom->imgSumSqPtrLow(x1+1,y1))-*(_imgMom->imgSumSqPtrLow(x0+1,y1)) +
	  *(_imgMom->imgSumSqPtrLow(x0+1,y0))-*(_imgMom->imgSumSqPtrLow(x1+1,y0));
	
	_vectorSumSqPtr[3 - xo] = iSumII;
      } // End of: for (int32 xo = 0; xo < 3; xo++) {

      vector unsigned short sumI;
      
      /* Calculate (N*SumII - SumI*SumI).
       */
      {
	vector unsigned int sumII, sumISq;
	sumI = *(vector unsigned short*)_vectorSumPtr;
	sumII = *(vector unsigned int*)_vectorSumSqPtr;
	
	sumISq = vec_mulo(sumI, sumI);
	T0_UI = vec_mule(N, *(vector unsigned short*)&sumII);
	T0_UI = vec_sld(T0_UI, V0_UI, 2);
	sumII = vec_mulo(N, *(vector unsigned short*)&sumII);
	sumII = vec_add(sumII, T0_UI);
	
	T0_UI = vec_sub(sumII, sumISq);
	varI = vec_ctf(T0_UI, 0);
      }

      /* Calculate (N*SumIM - SumI*SumM).
       */
      {
	vector unsigned int sumIM, sumISumM;
	
	sumISumM = vec_mulo(sumI, sumM);
	sumIM = *(vector unsigned int*)(_destSpacePtr + 4*yo);
	T0_UI = vec_mule(N, *(vector unsigned short*)&sumIM);
	T0_UI = vec_sld(T0_UI, V0_UI, 2);
	sumIM = vec_mulo(N, *(vector unsigned short*)&sumIM);
	sumIM = vec_add(sumIM, T0_UI);
	
	T0_UI = vec_sub(sumIM, sumISumM);
	varIM = vec_ctf(T0_SI, 0);
      }

      vector bool int mask;

      /* Set T0_F = (N*sumII - (sumI*sumI)) * (N*sumMM - (sumM*sumM))
       *     T1_F = (N*sumIM - (sumI*sumM)) * (N*sumIM - (sumI*sumM))
       */
      T0_F = vec_madd(varI, varM, V0_F);
      mask = vec_cmpgt(T0_F, V0_F); // Used to mask out any division's by zero
      T1_F = vec_madd(varIM, varIM, V0_F);
      /* Now ready to perform division using reciprocal estimate
       * followed by "Newton-Raphson" refinement, as specified in
       * Altivec PEM section 4.2.2.1.1.
       */
      vector float f0, f1;
      f0 = vec_re(T0_F);
      f1 = vec_nmsub(f0, T0_F, vf1);
      f0 = vec_madd(f0, f1, f0);
      f1 = vec_nmsub(f0, T0_F, vf1);
      f1 = vec_madd(f0, f1, f0);
      f0 = vec_madd(T1_F, f1, V0_F);
      T1_F = vec_nmsub(T0_F, f0, T1_F);
      T0_F = vec_madd(T1_F, f1, f0);
      T0_F = vec_and(T0_F, mask); // If denominator was 0, return 0
      T0_F = vec_min(T0_F, vf1); // Clamp upper bound to <= 1.0

      *(vector float*)_vectorSumPtr = T0_F;

      res.score[yo][0] = *((float*)_vectorSumPtr + 3);
      res.score[yo][1] = *((float*)_vectorSumPtr + 2);
      res.score[yo][2] = *((float*)_vectorSumPtr + 1);
    } // End of: for (int32 yo = 0; yo < 3; yo++) {
  } // End of: "Now calculate correlation scores ..." block
#endif
}

void rcPointCorrelation::gen5by5Space(const rcRect& srchSpace,
				      const rcIPair& modelOrigin, 
				      rsCorr5by5Point& res) const
{

#ifndef __ppc__
   	vector<vector<float> > results (5);
	for (uint32 i = 0; i < 5; i++)
		results[i].resize (5);
		
	genSpace(srchSpace, modelOrigin, results, 2);
	for (uint32 j = 0; j < 5; j++)
		for (uint32 i = 0; i < 5; i++)
		res.score[j][i] = results[j][i];
#else
	
  /* Validate the current state of the object and the inputs.
   * Calculate model and image position and size.
   */
  rmAssert(_imgMom);
  rmAssert(_mdlMom);

  const int32 xImgOrigin = srchSpace.x();
  const int32 yImgOrigin = srchSpace.y();
  const int32 imgWidth = srchSpace.width();
  const int32 imgHeight = srchSpace.height();
  rmAssert(imgWidth <= 16);
  rmAssert(imgHeight <= 16);
  const int32 xSpcBound = xImgOrigin + imgWidth;
  const int32 ySpcBound = yImgOrigin + imgHeight;
  rmAssert(xImgOrigin < xSpcBound);
  rmAssert(xSpcBound <= _imgWin.width());
  rmAssert(yImgOrigin < ySpcBound);
  rmAssert(ySpcBound <= _imgWin.height());

  /* For 5x5 correlation space, model dimensions are always (-4,-4)
   * of the search space dimensions.
   */
  const int32 mdlWidth = imgWidth - 4;
  const int32 mdlHeight = imgHeight - 4;
  rmAssert(mdlWidth > 0);
  rmAssert(mdlHeight > 0);

  const int32 xMdlOrigin = modelOrigin.x();
  const int32 yMdlOrigin = modelOrigin.y();
  const int32 xMdlBound = xMdlOrigin + mdlWidth;
  const int32 yMdlBound = yMdlOrigin + mdlHeight;
  rmAssert(xMdlOrigin < xMdlBound);
  rmAssert(xMdlBound <= _mdlWin.width());
  rmAssert(yMdlOrigin < yMdlBound);
  rmAssert(yMdlBound <= _mdlWin.height());

  /* Request is valid. Calculate IM values for all the locations
   * within the search space.
   */
  vector short t0, t1, v0 = vec_splat_s16(0);
  vector unsigned char mModelPerm, mAlignPerm, iAlignPerm, rotatePerm;

  unsigned char* img =
    (unsigned char*)(_iRowBase + _iRowUpdate*yImgOrigin + xImgOrigin);
  vector unsigned char* iPtr =  (vector unsigned char*)img;
  unsigned char* mdl =
    (unsigned char*)(_mRowBase + _mRowUpdate*yMdlOrigin + xMdlOrigin);
  vector unsigned char* mPtr = (vector unsigned char*)mdl;
  vector signed int* destP = (vector signed int*)_destSpacePtr;

  const bool icb = (((int)iPtr & 0xF) + imgWidth) > 16;
  const bool mcb = (((int)mPtr & 0xF) + mdlWidth) > 16;

  /* Note: In the following comments, sample values are given based on the
   * assumption that:
   *
   *    mdlWidth = 5
   *    For mcb == true case: Address of mdl == 12 MOD 16
   *    For mcb == false case: Address of mdl == 10 MOD 16
   */
  iAlignPerm = vec_lvsl(0, img);
  T0_UC = vec_splat_u8(3);               // T0= 03030303030303030303030303030303
  T0_UC = vec_sl(T0_UC, T0_UC);          // T0= 18181818181818181818181818181818
  T1_UC = vec_lvsr(mdlWidth, (int*)0);   // T1= 0B0C0D0E0F101112131415161718191A
  T0_UC = vec_perm(V0_UC, T0_UC, T1_UC); // T0= 00000000001818181818181818181818
  if (mcb) {
    mAlignPerm = vec_lvsl(0, mdl);       // AP= 0C0D0E0F101112131415161718191A1B
    T1_UC = vec_lvsl(0, (int*)0);        // T1= 000102030405060708090A0B0C0D0E0F
    mModelPerm = vec_or(T1_UC, T0_UC);   // MP= 00010203041X1X1X1X1X1X1X1X1X1X1X
  }
  else {
    mAlignPerm = vec_splat_u8(0);        // Quiet compiler
    T1_UC = vec_lvsl(0, mdl);            // T1= 0A0B0C0D0E0F10111213141516171819
    mModelPerm = vec_or(T1_UC, T0_UC);   // MP= 0A0B0C0D0E1X1X1X1X1X1X1X1X1X1X1X
  }

  rotatePerm = vec_lvsr(4,(int*)0);
  
  if (icb) {
    if (mcb) {
      if (mdlHeight > 4) {
	GEN_LRG_5X5_SPACE(LOADI_CB, LOADM_CB, mdlHeight, ICBMCB);
      }
      else {
	GEN_SML_5X5_SPACE(LOADI_CB, LOADM_CB, mdlHeight);
      }
    }
    else {
      if (mdlHeight > 4) {
	GEN_LRG_5X5_SPACE(LOADI_CB, LOADM, mdlHeight, ICBM);
      }
      else {
	GEN_SML_5X5_SPACE(LOADI_CB, LOADM, mdlHeight);
      }
    }
  }
  else {
    if (mcb) {
      if (mdlHeight > 4) {
	GEN_LRG_5X5_SPACE(LOADI, LOADM_CB, mdlHeight, IMCB);
      }
      else {
	GEN_SML_5X5_SPACE(LOADI, LOADM_CB, mdlHeight);
      }
    }
    else {
      if (mdlHeight > 4) {
	GEN_LRG_5X5_SPACE(LOADI, LOADM, mdlHeight, IM);
      }
      else {
	GEN_SML_5X5_SPACE(LOADI, LOADM, mdlHeight);
      }
    }
  } // End of: if (icb) ... else ...

  /* Now calculate correlation scores for model and image at all
   * locations within the search space.
   */
  {
    vector unsigned short N, sumM;
    vector float varM, varI, varIM, vf1;

    T0_UI = vec_splat_u32(1);
    vf1 = vec_ctf(T0_UI, 0);

    /* Calculate and store N in vector.
     */
    *(uint16*)_vectorSumPtr = (uint16)(mdlWidth*mdlHeight);
    N = *(vector unsigned short*)_vectorSumPtr;
    N = vec_splat(N, 0);

    /* Calculate and store sumM and varM in vectors.
     */
    {
      const int32 x0 = xMdlOrigin*3;
      const int32 y0 = yMdlOrigin;
      const int32 x1 = x0 + mdlWidth*3;
      const int32 y1 = y0 + mdlHeight;
      
      const uint16 iSumM =
	(uint16)(*(_mdlMom->imgSumPtr(x1, y1)) - *(_mdlMom->imgSumPtr(x0, y1)) +
		   *(_mdlMom->imgSumPtr(x0, y0)) - *(_mdlMom->imgSumPtr(x1, y0)));

      *(uint16*)_vectorSumPtr = iSumM;
      sumM = *(vector unsigned short*)_vectorSumPtr;
      sumM = vec_splat(sumM, 0);

      vector unsigned int sumMM, sumMSq;

      const uint32 iSumMM =
	*(_mdlMom->imgSumSqPtrLow(x1+1,y1)) - *(_mdlMom->imgSumSqPtrLow(x0+1,y1)) +
	*(_mdlMom->imgSumSqPtrLow(x0+1,y0)) - *(_mdlMom->imgSumSqPtrLow(x1+1,y0));

      *_vectorSumPtr = iSumMM;
      sumMM = *(vector unsigned int*)_vectorSumPtr;
      sumMM = vec_splat(sumMM, 0);
      
      /* Calculate (N*SumMM - SumM*SumM).
       *
       * Note that SumM always fits in 16 bits, so the multiply can be
       * done in a single instruction. SumMM can go over 16 bits, so
       * the multiply is done as a pair of 16 bit multiplies, a 16 bit
       * shift, and an add.
       */
      sumMSq = vec_mulo(sumM, sumM);
      T0_UI = vec_mule(N, *(vector unsigned short*)&sumMM);
      T0_UI = vec_sld(T0_UI, V0_UI, 2);
      sumMM = vec_mulo(N, *(vector unsigned short*)&sumMM);
      sumMM = vec_add(sumMM, T0_UI);

      T0_UI = vec_sub(sumMM, sumMSq); // T0_UI = (N*SumMM - SumM*SumM).
      varM = vec_ctf(T0_UI, 0);
    }

  /* At each location within the search space, calculate varI, varIM
   * and use these values to calculate the final correlation scores.
   */
    const int32 x0Start = xImgOrigin*3;
    const int32 x1Start = x0Start + mdlWidth*3;
    for (int32 yo = 0; yo < 5; yo++) {
      const int32 y0 = (yImgOrigin + yo);
      const int32 y1 = y0 + mdlHeight;

      int32 x0Cur = x0Start;
      int32 x1Cur = x1Start;
      for (int32 xo = 0; xo < 5; xo++) {
	const int32 x0 = x0Cur, x1 = x1Cur;
 	const uint32 iSumI =
	  *(_imgMom->imgSumPtr(x1, y1)) - *(_imgMom->imgSumPtr(x0, y1)) +
	  *(_imgMom->imgSumPtr(x0, y0)) - *(_imgMom->imgSumPtr(x1, y0));
	
	_vectorSumPtr[7 - xo] = iSumI;
	
	const uint32 iSumII =
	  *(_imgMom->imgSumSqPtrLow(x1+1,y1))-*(_imgMom->imgSumSqPtrLow(x0+1,y1)) +
	  *(_imgMom->imgSumSqPtrLow(x0+1,y0))-*(_imgMom->imgSumSqPtrLow(x1+1,y0));
	
	_vectorSumSqPtr[7 - xo] = iSumII;
	x0Cur += 3;
	x1Cur += 3;
      } // End of: for (int32 xo = 0; xo < 5; xo++) {

      /* On the last 2 rows 2 set of results will be processed.
       * Otherwise the 5th I, II, IM on the row is saved for future
       * use and only one set of results is processed.
       */
      int32 loopCnt = 2;
      if (yo < 3) {
	loopCnt = 1;
	_vectorSumSqPtr[yo] = _vectorSumSqPtr[3];
	_vectorSumPtr[yo] = _vectorSumPtr[3];
	_destSpacePtr[3*8 + yo] = _destSpacePtr[yo*8 + 3];
      }

      int32 index = 4;
      for (int32 loop = 0; loop < loopCnt; loop++, (index = 0)) {
	vector unsigned short sumI;
	
	/* Calculate (N*SumII - SumI*SumI).
	 */
	{
	  vector unsigned int sumII, sumISq;
	  sumI = *(vector unsigned short*)(_vectorSumPtr + index);
	  sumII = *(vector unsigned int*)(_vectorSumSqPtr + index);
	
	  sumISq = vec_mulo(sumI, sumI);
	  T0_UI = vec_mule(N, *(vector unsigned short*)&sumII);
	  T0_UI = vec_sld(T0_UI, V0_UI, 2);
	  sumII = vec_mulo(N, *(vector unsigned short*)&sumII);
	  sumII = vec_add(sumII, T0_UI);
	
	  T0_UI = vec_sub(sumII, sumISq);
	  varI = vec_ctf(T0_UI, 0);
	}

	/* Calculate (N*SumIM - SumI*SumM).
	 */
	{
	  vector unsigned int sumIM, sumISumM;
	  
	  sumISumM = vec_mulo(sumI, sumM);
	  sumIM = *(vector unsigned int*)(_destSpacePtr + 8*yo + index);
	  T0_UI = vec_mule(N, *(vector unsigned short*)&sumIM);
	  T0_UI = vec_sld(T0_UI, V0_UI, 2);
	  sumIM = vec_mulo(N, *(vector unsigned short*)&sumIM);
	  sumIM = vec_add(sumIM, T0_UI);
	  
	  T0_UI = vec_sub(sumIM, sumISumM);
	  varIM = vec_ctf(T0_SI, 0);
	}
	
	vector bool int mask;

	/* Set T0_F = (N*sumII - (sumI*sumI)) * (N*sumMM - (sumM*sumM))
	 *     T1_F = (N*sumIM - (sumI*sumM)) * (N*sumIM - (sumI*sumM))
	 */
	T0_F = vec_madd(varI, varM, V0_F);
	mask = vec_cmpgt(T0_F, V0_F); // Used to mask out any division's by zero
	T1_F = vec_madd(varIM, varIM, V0_F);

	/* Now ready to perform division using reciprocal estimate
	 * followed by "Newton-Raphson" refinement, as specified in
	 * Altivec PEM section 4.2.2.1.1.
	 */
	vector float f0, f1;
	f0 = vec_re(T0_F);
	f1 = vec_nmsub(f0, T0_F, vf1);
	f0 = vec_madd(f0, f1, f0);
	f1 = vec_nmsub(f0, T0_F, vf1);
	f1 = vec_madd(f0, f1, f0);
	f0 = vec_madd(T1_F, f1, V0_F);
	T1_F = vec_nmsub(T0_F, f0, T1_F);
	T0_F = vec_madd(T1_F, f1, f0);
	T0_F = vec_and(T0_F, mask); // If denominator was 0, return 0
	T0_F = vec_min(T0_F, vf1); // Clamp upper bound to <= 1.0
	*(vector float*)_resultPtr = T0_F;
	
	if (loop == 0) {
	  res.score[yo][0] = *((float*)_resultPtr + 3);
	  res.score[yo][1] = *((float*)_resultPtr + 2);
	  res.score[yo][2] = *((float*)_resultPtr + 1);
	  res.score[yo][3] = *((float*)_resultPtr + 0);
	}
	else if (yo == 3) {
	  res.score[0][4] = *((float*)_resultPtr + 0);
	  res.score[1][4] = *((float*)_resultPtr + 1);
	  res.score[2][4] = *((float*)_resultPtr + 2);
	  res.score[3][4] = *((float*)_resultPtr + 3);
	}
	else
	  res.score[4][4] = *((float*)_resultPtr + 3);
      } // End of: for (int32 loop = 0; loop < loopCnt; loop++) {
    } // End Of: for (int32 yo = 0; yo < 5; yo++) {
  } // End of: "Now calculate correlation scores ..." block
#endif
}

void rcPointCorrelation::genSpace(const rcRect& srchSpace,
				  const rcIPair& modelOrigin,
				  vector<vector<float> >& res,
				  const int32 acRadius) const
{
  rmAssert(acRadius > 0);
  const int32 diameter = acRadius*2 + 1;

  const int32 xImgOrigin = srchSpace.x();
  const int32 yImgOrigin = srchSpace.y();

  rmAssert(srchSpace.width() > (diameter-1));
  rmAssert(srchSpace.height() > (diameter-1));
  const int32 imgWidth = srchSpace.width();
  const int32 imgHeight = srchSpace.height();

  const int32 xSpcBound = xImgOrigin + imgWidth;
  const int32 ySpcBound = yImgOrigin + imgHeight;
  rmAssert(xImgOrigin < xSpcBound);
  rmAssert(xSpcBound <= _imgWin.width());
  rmAssert(yImgOrigin < ySpcBound);
  rmAssert(ySpcBound <= _imgWin.height());

  const int32 xMdlOrigin = modelOrigin.x();
  const int32 yMdlOrigin = modelOrigin.y();

  /* For NxN correlation space, model dimensions are always
   * (diameter-1,diameter-1) of the search space dimensions.
   */
  const int32 mdlWidth = imgWidth - (diameter-1);
  const int32 mdlHeight = imgHeight - (diameter-1);

  const int32 xMdlBound = xMdlOrigin + mdlWidth;
  const int32 yMdlBound = yMdlOrigin + mdlHeight;
  rmAssert(xMdlOrigin < xMdlBound);
  rmAssert(xMdlBound <= _mdlWin.width());
  rmAssert(yMdlOrigin < yMdlBound);
  rmAssert(yMdlBound <= _mdlWin.height());

  rmAssert((int32)(res.size()) >= diameter);
  for (int32 i = 0; i < diameter; i++)
    rmAssert((int32)(res[i].size()) >= diameter);

  /* Request is valid. Calculate result space.
   */
  rsCorrParams cp;
  rcCorr cr;
  rcWindow model(_mdlWin, xMdlOrigin, yMdlOrigin, mdlWidth, mdlHeight);

  for (int32 y = 0; y < diameter; y++)
    for (int32 x = 0; x < diameter; x++) {
      rcWindow image(_imgWin, xImgOrigin + x, yImgOrigin + y,
		     mdlWidth, mdlHeight);
      rfCorrelate(image, model, cp, cr);
      res[y][x] = cr.r();
    }
}


