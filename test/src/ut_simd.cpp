/*
 *  ut_simd.cpp
 *  vf
 *
 *  Created by arman on 1/27/09.
 *  Copyright 2009 Reify Corporation. All rights reserved.
 *
 */
#include <mmintrin.h>
#include <rc_window.h>
#include "ut_simd.h"


int testMMXCorr ();
int gtruth ();

int ut_simd (std::string& foo)
{
	uint32 ne = 0;
#if defined  (__i386__)
	ne += (testMMXCorr () != gtruth ());
	assert (ne == 0);
#endif
	return ne;
}

int gtruth ()
{
    vector<uint16> vs1 (32);
    vector<uint16> vs2 (32);
    for (uint32 i = 0; i < 32; i++) vs1[i] = i*i;
    for (uint32 i = 0; i < 32; i++) vs2[i] = 2*i*i+i;
	int init = 0;
	uint32 gt = inner_product(vs1.begin(),vs1.end(),vs2.begin(),init);
	return gt;
}

	
#ifdef __i386__
long crossCorr(const short *pV1, const short *pV2, uint32 overlapLength);
int testMMXCorr ()
{	
    rcWindow s1 (32, 1, rcPixel16);
    rcWindow s2 (32, 1, rcPixel16);
    for (uint32 i = 0; i < 32; i++) s1.setPixel (i,0,i*i);
    for (uint32 i = 0; i < 32; i++) s2.setPixel (i,0,2*i*i+i);
    long l = crossCorr ((const short* ) s1.rowPointer (0), (const short *) s2.rowPointer(0), 32);
    return l;
}


long crossCorr(const short *pV1, const short *pV2, uint32 overlapLength)
{
    const __m64 *pVec1, *pVec2;
    __m64 shifter;
    __m64 smi, sii, smm, si, sm;
    long corr;
    uint i;
  //  short ones[4] = {1, 1, 1, };
	
    pVec1 = (__m64*)pV1;
    pVec2 = (__m64*)pV2;
	
    uint32 overlapDividerBits = 0;
    shifter = _m_from_int(overlapDividerBits);
    smi  = sii = smm = si = sm = _mm_setzero_si64();
	
    // Process 4 parallel sets of 2 * stereo samples each during each
    // round to improve CPU-level parallellization.
    for (i = 0; i < overlapLength / 8; i ++)
    {
        __m64 temp;
		
        // dictionary of instructions:
        // _m_pmaddwd   : 4*16bit multiply-add, resulting two 32bits = [a0*b0+a1*b1 ; a2*b2+a3*b3]
        // _mm_add_pi32 : 2*32bit add
        // _m_psrad     : 32bit right-shift
		
        temp = _mm_add_pi32(_mm_madd_pi16(pVec1[0], pVec2[0]),
                            _mm_madd_pi16(pVec1[1], pVec2[1]));
        smi = _mm_add_pi32(smi, _mm_sra_pi32(temp, shifter));
		
        temp = _mm_add_pi32(_mm_madd_pi16(pVec1[2], pVec2[2]),
                            _mm_madd_pi16(pVec1[3], pVec2[3]));
        smi = _mm_add_pi32(smi, _mm_sra_pi32(temp, shifter));
		
        pVec1 += 4;
        pVec2 += 4;
    }
	
    // copy hi-dword of mm0 to lo-dword of mm1, then sum mmo+mm1
    // and finally store the result into the variable "corr"
	
    smi = _mm_add_pi32(smi, _mm_srli_si64(smi, 32));
    corr = _m_to_int(smi);
	
    // Clear MMS state
    _m_empty();
	
    return corr;
    // Note: Warning about the missing EMMS instruction is harmless
    // as it'll be called elsewhere.
}
#endif
