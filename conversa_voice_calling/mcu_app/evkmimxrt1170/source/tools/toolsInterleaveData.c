/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tools.h>

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsInterleaveData_32b													 */
/*                                                                                               */
/* DESCRIPTION:                                                                                  */
/*  Interleaved N input channel data to 1 output channel 							             */
/*    Example: 3 channel / 10 samples															 */
/*			   input is CH0 (D0 D1 D2 D3 D4 D5 D6 D7 D8 D9)										 */
/*			   input is CH1 (D0 D1 D2 D3 D4 D5 D6 D7 D8 D9)										 */
/*			   input is CH2 (D0 D1 D2 D3 D4 D5 D6 D7 D8 D9)										 */
/*             output is CH1D0 CH2D0 CH3D0 CH1D1 CH2D1 CH3D1 CH1D2 CH2D2 CH3D3 ... 				 */
/*                                                                                               */
/* PARAMETERS:                                                                                   */
/*  pIns                table of input buffers		                                             */
/*  pOut		        Output interleaved buffer						                         */
/*  NumSamples          Number of samples in each input buffers	 		 						 */
/*  NumChannels			Number of input channels to interleaved                                  */
/*************************************************************************************************/
status_t toolsInterleaveData_32b(	PL_INT32 **pIns,
						 	 	 	PL_INT32 *pOut,
								 	PL_INT16 NumSamples,
									PL_INT16 NumChannels)
{
	status_t retStatus = kStatus_Success;

	// process variable
	PL_INT16 offsetSampleOut 	= 0;  // sample offset in the output buffer

	// loop indice variable
	volatile PL_INT16 iCh 		= 0;
	volatile PL_INT16 iSpl 		= 0;

	/********************/
	/* CHECK PARAMETERS */

	// check for not null input channels to interleave
	for(iCh = NumChannels; iCh>0; iCh--)
	{
	  	if (pIns[iCh-1] == NULL)
	  	{
	  		return kStatus_NullPointer;
	  	}
	}

	// check for not null output buffer
	if (pOut ==NULL)
	{
		return kStatus_NullPointer;
	}

	/********************/
	/* INTERLEAVE DATA */
	// loop for channels
	for (iCh = 0; iCh < NumChannels; iCh++)
	{
		// loop for samples
		for(iSpl=0; iSpl<NumSamples; iSpl++)
    	{
			offsetSampleOut = iSpl*NumChannels;

			pOut[offsetSampleOut+iCh] = pIns[iCh][iSpl];
    	}
    }

    return retStatus;
}
