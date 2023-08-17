/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TOOLS_H__
#define __TOOLS_H__

// Platform
#include "PL_platformTypes.h"
//common
#include "fsl_common.h"


/*******************************************************************************
 * Structure
 ******************************************************************************/
typedef struct
{
	PL_UINT8 * p_cbStart;    			// beginning of the circular buffer
	PL_UINT8 * p_cbStop;	  			// end of the circular buffer
	PL_UINT8 * p_cbRead;	  			// first readable data
	PL_UINT8 * p_cbWrite;	  			// first writable data
	PL_UINT32  cbSize;		  			// size of the circular buffer in bytes
	PL_UINT32  cbChannelNum;  			// number of channels in the circular buffer
	PL_UINT32  cbSampleWidth; 			// number of bytes per sample
	PL_UINT32  lowLimitInBytes;			// used space low limit in Bytes. Value used for feedback calculation. It represents 1/3 of the cbSize
	PL_UINT32  highLimitInBytes	;		// used space high limit in Bytes. Value used for feedback calculation. It represents 2/3 of the cbSize
	PL_UINT8    flagMidFull;	 		// flag to indicates that circular buffer is midfull
	PL_UINT8	flagOptional;			// this flag can be use to propagate an external state inside the CirBuff struct + env +code

} TOOLS_circularBuffer_st;

typedef struct
{
	PL_UINT8*  p_cbBuffer;				// Memory used for the circular buffer
	PL_UINT32  bBufferSizeInBytes;		// size of the memory used for the circular buffer
    PL_UINT32  cbChannelNum;			// number of channels interleaved inside of the circular buffer
    PL_UINT32  cbSampleWidth;			// size of each samples in bytes

} TOOLS_circularBuffer_param;

/*******************************************************************************
 * Variables
 ******************************************************************************/




/*****************************************************************************************
 * Function
 *****************************************************************************************/


/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferStructInit										     */
/*                                                                                               */
/* DESCRIPTION:    	    initialize a circular buffer structure with user parameters              */
/* PARAMETERS:          p_cirBuff           Circular buffer structure                            */
/*                      param				param used to init the structure					 */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferStructInit(TOOLS_circularBuffer_st*    p_cirBuff,
									   TOOLS_circularBuffer_param  param);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferRead_I_NI 										     */
/*                      (Interleaved Input- Non-interleaved output)                              */
/*                                                                                               */
/* DESCRIPTION:    	    read interleaved data from cirBuff and deinterleaved them to  pp_Outs    */
/* PARAMETERS:          p_cirBuff              		  Circular buffer                            */
/*                      pp_Outs				          output deinterleaved buffers               */
/*                      samplesPerChannelToRead       number of samples to read per channel      */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferRead_I_NI(TOOLS_circularBuffer_st* p_cirBuff,
							     	  PL_UINT8** 			   pp_Outs,
									  PL_UINT32                samplesPerChannelToRead );

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferRead_I_I 										     */
/*                      (Interleaved Input- Interleaved output)                                  */
/*                                                                                               */
/* DESCRIPTION:    	    read interleaved data from cirBuff and copy them interleaved to p_Out    */
/* PARAMETERS:          p_cirBuff              		  Circular buffer to read                    */
/*                      p_Out				          output interleaved buffer                  */
/*                      bytesToRead                   number of samples to read per channel      */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferRead_I_I(TOOLS_circularBuffer_st* p_cirBuff,
									 PL_UINT8* 			      p_Out,
									 PL_UINT32               bytesToRead );

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferWrite_NI_I										     */
/*                      (Non-Interleaved Input- Interleaved output)                              */
/*                                                                                               */
/* DESCRIPTION:    	    write data from pp_Ins  to cirBuff and interleaved them                  */
/* PARAMETERS:          p_cirBuff              		  Circular buffer to read                    */
/*                      pp_Ins				          input non-interleaved buffers              */
/*                      samplesPerChannelToWrite      number of samples to write per channel     */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferWrite_NI_I(TOOLS_circularBuffer_st* p_cirBuff,
							      	   PL_UINT8** 			    pp_Ins,
									   PL_UINT32                samplesPerChannelToWrite );
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferWrite_I_I										     */
/*                      (Interleaved Input- Interleaved output)                                  */
/*                                                                                               */
/* DESCRIPTION:    	    write data from  p_In  to cirBuff and interleaved them                   */
/* PARAMETERS:          p_cirBuff              		  Circular buffer                            */
/*                      p_In				          input interleaved buffer                   */
/*                      bytesToWrite                  number of bytes to write                   */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferWrite_I_I(TOOLS_circularBuffer_st*  p_cirBuff,
							      	  PL_UINT8* 			        p_In,
									  PL_UINT32                 bytesToWrite );


/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferReadableData									 		 */
/*                                                                                               */
/* DESCRIPTION:    	    return readable amount of data     										 */
/*                                                             									 */
/* PARAMETERS:          p_cirBuff       circular buffer                     			         */
/*                                                                                               */
/*************************************************************************************************/
PL_UINT32 toolsCircularBufferReadableData(TOOLS_circularBuffer_st*  p_cirBuff);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferWritableData									 		 */
/*                                                                                               */
/* DESCRIPTION:    	    return writable amount of data     										 */
/*                                                             									 */
/* PARAMETERS:          p_cirBuff       circular buffer                     			         */
/*                                                                                               */
/*************************************************************************************************/
PL_UINT32 toolsCircularBufferWritableData(TOOLS_circularBuffer_st*  p_cirBuff);
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferDeInterleaveData_16b									 */
/*                                                                                               */
/* DESCRIPTION:    	    de-interleave  numSamples from p_in16b + copy  in pp_out16b starting     */
/*                      from index pp_out16b[0][OutIndex]                                        */
/* PARAMETERS:          p_in16b        input data to deinterleave                      			 */
/*                      pp_out16b	   Output deinterleaved buffers                              */
/*                      numSamples     Number of 16bits samples per channel to de-interleave     */
/*                      numChannels    Number of channels to de-interleave                       */
/*                      OutIndex       pp_out16b sample index where to begin de-int operation    */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferDeInterleaveData_16b(PL_UINT16*  p_in16b,
											     PL_UINT16** pp_out16b,
											     PL_UINT32  numSamples,
												 PL_UINT32  numChannels,
											     PL_UINT32  OutIndex);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferDeInterleaveData_32b									 */
/*                                                                                               */
/* DESCRIPTION:    	    de-interleave  numSamples from p_in16b + copy  in pp_out32b starting     */
/*                      from index pp_out32b[0][OutIndex]                                        */
/* PARAMETERS:          p_in32b        input data to deinterleave                      			 */
/*                      pp_out32b	   Output deinterleaved buffers                              */
/*                      numSamples     Number of 16bits samples per channel to de-interleave     */
/*                      numChannels    Number of channels to de-interleave                       */
/*                      OutIndex       pp_out16b sample index where to begin de-int operation    */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferDeInterleaveData_32b(PL_UINT32 * p_in32b,
											     PL_UINT32 ** pp_out32b,
											     PL_UINT32 numSamples,
												 PL_UINT32 numChannels,
											     PL_UINT32 OutIndex);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferInterleaveData_16b									 */
/*                                                                                               */
/* DESCRIPTION:    	    interleave  numSamples from pp_in16b + copy  in p_out16b starting        */
/*                      from index pp_in16b[0][OutIndex]                                         */
/* PARAMETERS:          pp_in16b       input data to interleave                      		     */
/*                      p_out16b	   Output interleaved buffers                                */
/*                      numSamples     Number of 16bits samples per channel to interleave        */
/*                      numChannels    Number of channels to interleave                          */
/*                      InIndex        pp_in16b sample index where to begin int operation        */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferInterleaveData_16b(PL_UINT16** pp_in16b,
											   PL_UINT16*  p_out16b,
											   PL_UINT32  numSamples,
											   PL_UINT32  numChannels,
											   PL_UINT32  InIndex);
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferInterleaveData_32b									 */
/*                                                                                               */
/* DESCRIPTION:    	    interleave  numSamples from pp_in32b + copy  in p_out32b starting        */
/*                      from index pp_in32b[0][OutIndex]                                         */
/* PARAMETERS:          pp_in32b       input data to interleave                      		     */
/*                      p_out32b	   Output interleaved buffers                                */
/*                      numSamples     Number of 32bits samples per channel to interleave        */
/*                      numChannels    Number of channels to interleave                          */
/*                      InIndex        pp_in32b sample index where to begin int operation        */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferInterleaveData_32b(PL_UINT32** pp_in32b,
											   PL_UINT32*  p_out32b,
											   PL_UINT32  numSamples,
											   PL_UINT32  numChannels,
											   PL_UINT32  InIndex);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferDeInterleaveData_24b_for32bContainer					 */
/*                                                                                               */
/* DESCRIPTION:    	    de-interleave  numSamples from p_in8b + copy  in pp_out8b starting       */
/*                      from index pp_out32b[0][OutIndex]                                        */
/* PARAMETERS:          p_in32b        input data to deinterleave                      			 */
/*                      pp_out32b	   Output deinterleaved buffers                              */
/*                      numSamples     Number of 16bits samples per channel to de-interleave     */
/*                      numChannels    Number of channels to de-interleave                       */
/*                      OutIndex       pp_out16b sample index where to begin de-int operation    */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferDeInterleaveData_24b_for32bContainer(PL_UINT8 * p_in8b,
																 PL_UINT8 ** pp_out8b,
																 PL_UINT32 numSamples,
																 PL_UINT32 numChannels,
																 PL_UINT32 OutIndex);
/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferAdjustRdPos									         */
/*                                                                                               */
/* DESCRIPTION:    	    adjust read position with an offset                                      */
/*                      								                                         */
/* PARAMETERS:          p_cirBuff       circular buffer			                      		     */
/*                      offsetInBytes	   position offset in bytes                              */
/*                                                                                               */
/*************************************************************************************************/

status_t toolsCircularBufferAdjustRdPos(TOOLS_circularBuffer_st*  p_cirBuff,
							            PL_UINT32 			      offsetInBytes);


/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            ResetCrc32Seed									                         */
/*                                                                                               */
/* DESCRIPTION:    	    Reset CRC seed                                                           */
/*                      								                                         */
/* PARAMETERS:          		                      		     								 */
/*                                                                                               */
/*                                                                                               */
/*************************************************************************************************/
void ResetCrc32Seed(void);

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            Init_CrcEngine									                         */
/*                                                                                               */
/* DESCRIPTION:    	    Init CRC engine                                                          */
/*                      								                                         */
/* PARAMETERS:          		                      		     								 */
/*                                                                                               */
/*                                                                                               */
/*************************************************************************************************/
void Init_CrcEngine(void);
#endif /*__TOOLS_H__*/
