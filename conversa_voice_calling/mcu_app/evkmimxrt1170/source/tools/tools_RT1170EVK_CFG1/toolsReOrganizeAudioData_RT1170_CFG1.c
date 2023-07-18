/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tools.h"

#include "appGlobal.h"

// memory section replacement
#include <cr_section_macros.h>


#if (APP_PLATFORM == APP_PL_RT1170EVK)

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsReOrganizeAudioData									 			 */
/*                                                                                               */
/* DESCRIPTION:                                                                                  */
/*  Format receive buffer to individual buffer 							 					 	 */
/*																								 */
/* PDM INPUT:																					 */
/* 	PDM are 8 channels so input data are formatted like this									 */
/*		Sample0 PDM0 PDM1 PDM2 PDM3 PDM4 PDM6 PDM7 / Sample1 PDM0 PDM1 ... 						 */
/* 	Even if mic is not present the PDM data are formated like this because the EDMA read the 	 */
/*  8 PDM channel even if they are disable.													 	 */
/* 																								 */
/*	AUDIO_SRC_SQUARE_TRILLIUM_4DMIC:																	     */
/*	 	PDM 2 3 6 7 8 are not connected to a microphone so datas = 0							 */
/* 		output data buffer Mic 0: PDM0 Spl0 Spl1 Spl2 ...										 */
/* 		output data buffer Mic 1: PDM1 Spl0 Spl1 Spl2 ...										 */
/* 		output data buffer Mic 3: PDM4 Spl0 Spl1 Spl2 ...										 */
/* 		output data buffer Mic 4: PDM5 Spl0 Spl1 Spl2 ...										 */
/*																								 */
/* PARAMETERS:                                                                                   */
/*  p_inOut                     input buffer		                                       			 */
/*  pp_inOut		            table of output buffers					                         	 */
/*  p_configParam           path parameters     												 */
/*************************************************************************************************/
__RAMFUNC(RAM) // force to put the code instruction of this function in RAM (FLEXRAMRAM_ITC)
status_t toolsReOrganizeAudioData(	PL_INT8*			  p_inOut,
								 	PL_INT8**			  pp_inOut,
									AUDIO_configParam_st* p_configParam,
									AUDIO_direction_en	  audioDirection)
{
	status_t retStatus = kStatus_Success;

	/* indice */
	volatile PL_INT16 iChIn		 = 0;		// loop for in channel
	volatile PL_INT16 iChOut 	 = 0;	  	// loop for out channel
	volatile PL_INT16 iSpl 		 = 0;  	  	// loop for sample in the frame

	/* path parameter */
	PL_INT8  channelNumberPDMIn		   = APP_AUDIO_PDM_CHANNEL_NUMBER_MAX; 	// number of PDM channel enabled read by the edma
	PL_INT8  channelNumberOut		   = p_configParam->channelNumber; 		// number of PDM channel enabled read by the edma
	PL_INT16 samplePerFrame			   = p_configParam->samplePerFrame; 	// number of channel enabled
	PL_INT16 bytesPerSample 		   = p_configParam->bitPerSample/8; 	// number of channel enabled
	AUDIO_sourceElement_en audioSource = p_configParam->source; 			// audio source type

	/* process variables */
	PL_INT16 offsetSplIn 	= APP_AUDIO_PDM_CHANNEL_NUMBER_MAX ;	// offset between 2 samples of the same channel inside the In Buffer in bytes

	// To address different buffer format and different direction in o rout in the function
	PL_INT8*   p_in_8b 		= (PL_INT8*)    p_inOut; 		// audio data as input
	PL_INT8*   p_out_8b 	= (PL_INT8*)    p_inOut;		// audio data as output
	PL_INT8**  pp_in_8b 	= (PL_INT8**)  pp_inOut;        // audio data as input
	PL_INT8**  pp_out_8b 	= (PL_INT8**)  pp_inOut;        // audio data as output
	PL_INT16*  p_in_16b 	= (PL_INT16*)   p_inOut; 		// 16 bit audio data as input
	PL_INT16*  p_out_16b 	= (PL_INT16*)   p_inOut;		// 16 bit audio data as output
	PL_INT16** pp_in_16b 	= (PL_INT16**) pp_inOut;        // 16 bit audio data as input
	PL_INT16** pp_out_16b 	= (PL_INT16**) pp_inOut;        // 16 bit audio data as output
	PL_INT32*  p_in_32b 	= (PL_INT32*)   p_inOut;		// 32 bit audio data as input
	PL_INT32*  p_out_32b 	= (PL_INT32*)   p_inOut;		// 32 bit audio data as output
	PL_INT32** pp_in_32b 	= (PL_INT32**) pp_inOut;		// 32 bit audio data as input
	PL_INT32** pp_out_32b 	= (PL_INT32**) pp_inOut;		// 32 bit audio data as output

	/********************/
	/* CHECK PARAMETERS */

	// check for not null output channels to interleave
	for(iChOut = (channelNumberOut-1); iChOut >= 0; iChOut--)
	{
	  	if (pp_inOut[iChOut] == NULL)
	  	{
	  		return kStatus_NullPointer;
	  	}
	}
	// check for not null Input buffer
	if (p_inOut == NULL)
	{
		return kStatus_NullPointer;
	}

	/*************************************************/
	/* RE ORGANIZE AUDIO DATA WHICH COME FROM SOURCE */
	/*************************************************/

	if ( audioDirection == AUDIO_DIRECTION_FROM_SOURCE)
	{
		if (audioSource == AUDIO_SRC_OMNI_1DMIC)
		{
			// process channel in which get real data, others are fill with 0 value because EDMA read PDM buffer in continuous (enabled or not) and the PDM hardware mics are not mapped continuously
			if (p_configParam->bitPerSample == 16)
			{
				for(iSpl=0; iSpl < samplePerFrame; iSpl++) 					// loop for sample
				{
					offsetSplIn = iSpl * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX;	// offset sample In
					pp_out_16b [0] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn] >> 16);	// mic 0 = PDM channel 0 << 16 because PDM are always 32 bits
				}
			}
			else if (p_configParam->bitPerSample == 32)
			{
				for(iSpl=0; iSpl < samplePerFrame; iSpl++) 					// loop for sample
				{
					offsetSplIn = iSpl * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX;	// offset sample In
					pp_out_32b [0] [iSpl] = p_in_32b [offsetSplIn]; 		// mic 0 = PDM channel 0
				}
			}
			else
			{
				return kStatus_OutOfRange;
			} // end bitPerSample selection
		}
		else if (audioSource == AUDIO_SRC_LINEAR_2DMIC)
		{
			if (p_configParam->bitPerSample == 16)
			{
				for(iSpl=0; iSpl < samplePerFrame; iSpl++) 					// loop for sample
				{
					offsetSplIn = iSpl * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX;	// offset sample In
					pp_out_16b [0] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn] 	   >> 16); 		// mic 0 = PDM channel 0
					pp_out_16b [1] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn + 1] >> 16); 	    // mic 1 = PDM channel 1
				}
			}
			else if (p_configParam->bitPerSample == 32)
			{
				for(iSpl=0; iSpl < samplePerFrame; iSpl++) 					// loop for sample
				{
					offsetSplIn = iSpl * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX;	// offset sample In
					pp_out_32b [0] [iSpl] = p_in_32b [offsetSplIn]; 		// mic 0 = PDM channel 0
					pp_out_32b [1] [iSpl] = p_in_32b [offsetSplIn + 1]; 	// mic 1 = PDM channel 1
				}
			}
			else
			{
				return kStatus_OutOfRange;
			} // end bitPerSample selection
		}
		else if (audioSource == AUDIO_SRC_TRIANGULAR_3DMIC)
		{
			// process channel in which get real data, others are fill with 0 value because EDMA read PDM buffer in continuous (enabled or not) and the PDM hardware mics are not mapped continuously
			if (p_configParam->bitPerSample == 16)
			{
				// default mapping
				for(iSpl=0; iSpl < samplePerFrame; iSpl++) 					// loop for sample
				{
					offsetSplIn = iSpl * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX;	// offset sample In
					pp_out_16b [0] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn] 	   >> 16); 		// mic 0 = PDM channel 0
					pp_out_16b [1] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn + 1] >> 16); 	    // mic 1 = PDM channel 1
					pp_out_16b [2] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn + 4] >> 16); 	    // mic 2 = PDM channel 4
				}
			}
			else if (p_configParam->bitPerSample == 32)
			{
				for(iSpl=0; iSpl < samplePerFrame; iSpl++) 								// loop for sample
				{
					// default mapping
					offsetSplIn = iSpl * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX;	// offset sample In
					pp_out_32b [0] [iSpl] = p_in_32b [offsetSplIn]; 		// mic 0 = PDM channel 0
					pp_out_32b [1] [iSpl] = p_in_32b [offsetSplIn + 1]; 	// mic 1 = PDM channel 1
					pp_out_32b [2] [iSpl] = p_in_32b [offsetSplIn + 4]; 	// mic 2 = PDM channel 4
				}
			}
			else
			{
				return kStatus_OutOfRange;
			} // end bitPerSample selection
		}
		else if (audioSource == AUDIO_SRC_SQUARE_TRILLIUM_4DMIC)
		{
			// process channel in which get real data, others are fill with 0 value because EDMA read PDM buffer in continuous (enabled or not) and the PDM hardware mics are not mapped continuously
			if (p_configParam->bitPerSample == 16)
			{
				for(iSpl=0; iSpl < samplePerFrame; iSpl++) 					// loop for sample
				{
					offsetSplIn = iSpl * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX;	// offset sample In
					pp_out_16b [0] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn] 	   >> 16); 	// mic 0 = PDM channel 0
					pp_out_16b [1] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn + 1] >> 16); 	// mic 1 = PDM channel 1
					pp_out_16b [2] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn + 4] >> 16); 	// mic 2 = PDM channel 4
					pp_out_16b [3] [iSpl] = (PL_INT16) (p_in_32b [offsetSplIn + 5] >> 16); 	// mic 3 = PDM channel 5
				}

			}
			else if (p_configParam->bitPerSample == 32)
			{
				for(iSpl=0; iSpl < samplePerFrame; iSpl++) 					// loop for sample
				{
					offsetSplIn = iSpl * APP_AUDIO_PDM_CHANNEL_NUMBER_MAX;	// offset sample In
					pp_out_32b [0] [iSpl] = p_in_32b [offsetSplIn]; 		// mic 0 = PDM channel 0
					pp_out_32b [1] [iSpl] = p_in_32b [offsetSplIn + 1]; 	// mic 1 = PDM channel 1
					pp_out_32b [2] [iSpl] = p_in_32b [offsetSplIn + 4]; 	// mic 2 = PDM channel 4
					pp_out_32b [3] [iSpl] = p_in_32b [offsetSplIn + 5]; 	// mic 3 = PDM channel 5
				}
			}
			else
			{
				return kStatus_OutOfRange;
			} // end bitPerSample selection
		}
		else // audio source not managed
		{
			return kStatus_OutOfRange;
		} // end  audiosource selection
	} //end audioDirection selection

	/*************************************************/
	/* RE ORGANIZE AUDIO DATAS WHICH GO TO SINK      */
	/*************************************************/
	/* Copy data to the output sink buffer */
	else if ( audioDirection == AUDIO_DIRECTION_TO_SINK)
	{
		// 1 speaker => just copy buffer
		if ( p_configParam->sink == AUDIO_SINK_SPEAKER1 ) 							// if Audio Rx path send data to speaker mono
		{
			if (p_configParam->bitPerSample == 16)									// data are 16 bits
			{
				for (iSpl = 0 ;iSpl < (p_configParam->samplePerFrame) ; iSpl++) 	// store data interleaved into ping pong buffer
				{
					p_out_16b[ p_configParam->channelNumber * iSpl ]     = pp_in_16b[0][iSpl];
					p_out_16b[ p_configParam->channelNumber * iSpl + 1 ] = 0;		// force to 0 because 1 speaker. 2 channels are required because audio codec is using I2S
				}
			}
			else if (p_configParam->bitPerSample == 32)									// data are 32 bits
			{
				for (iSpl = 0 ;iSpl < (p_configParam->samplePerFrame) ; iSpl++)  		// store data interleaved into ping pong buffer
				{
					p_out_32b[ p_configParam->channelNumber * iSpl ]     = pp_in_32b[0][iSpl];
					p_out_32b[ p_configParam->channelNumber * iSpl + 1 ] = 0;		// force to 0 because 1 speaker. 2 channels are required because audio codec is using I2S
				}
			}
			else
			{
				retStatus = kStatus_OutOfRange;
			}
		} // end speaker 1*/

		// 1 differential speaker => non interleave to interleave buffer + negate channel 2
		else if ( p_configParam->sink == AUDIO_SINK_SPEAKER1DIFF ) 				   		// if Audio Rx path send data to speaker stereo
		{
			if (p_configParam->bitPerSample == 16)									// data are 16 bits
			{
				for (iSpl = 0 ;iSpl < (p_configParam->samplePerFrame) ; iSpl++) 	// store data interleaved into ping pong buffer
				{
					p_out_16b[ p_configParam->channelNumber * iSpl ]     =  pp_in_16b[0][iSpl];
					p_out_16b[ p_configParam->channelNumber * iSpl + 1 ] = -pp_in_16b[0][iSpl];
				}
			}
			else if (p_configParam->bitPerSample == 32)									// data are 32 bits
			{
				for (iSpl = 0 ;iSpl < (p_configParam->samplePerFrame) ; iSpl++)  		// store data interleaved into ping pong buffer
				{
					p_out_32b[ p_configParam->channelNumber * iSpl ]     =  (pp_in_32b[0][iSpl]);
					// compute negate value to create differential with saturation protection
					if (pp_in_32b[0][iSpl]== PL_INT32_MIN)
					{
						p_out_32b[ p_configParam->channelNumber * iSpl + 1 ] = PL_INT32_MAX;
					}
					else
					{
						p_out_32b[ p_configParam->channelNumber * iSpl + 1 ] = -(pp_in_32b[0][iSpl]);
					}
				}
			}
		}

		// 2 speakers => non interleave to interleave buffer
		else if ( p_configParam->sink == AUDIO_SINK_SPEAKER2 ) 				   		// if Audio Rx path send data to speaker stereo
		{
			if (p_configParam->bitPerSample == 16)									// data are 16 bits
			{
				for (iSpl = 0 ;iSpl < (p_configParam->samplePerFrame) ; iSpl++) 	// store data interleaved into ping pong buffer
				{
					p_out_16b[ p_configParam->channelNumber * iSpl ]     = pp_in_16b[0][iSpl];
					p_out_16b[ p_configParam->channelNumber * iSpl + 1 ] = pp_in_16b[1][iSpl];
				}
			}
			else if (p_configParam->bitPerSample == 32)									// data are 32 bits
			{
				for (iSpl = 0 ;iSpl < (p_configParam->samplePerFrame) ; iSpl++)  		// store data interleaved into ping pong buffer
				{
					p_out_32b[ p_configParam->channelNumber * iSpl ]     = pp_in_32b[0][iSpl];
					p_out_32b[ p_configParam->channelNumber * iSpl + 1 ] = pp_in_32b[1][iSpl];
				}
			}
			else
			{
				retStatus = kStatus_OutOfRange;
			}
		} // end speaker 2
		else
		{
			return kStatus_OutOfRange;
		} // end sink selection
	}

	else // audio direction not managed
	{
		return kStatus_OutOfRange;
	} // end  audiosource selection


    return retStatus;
}

#endif
