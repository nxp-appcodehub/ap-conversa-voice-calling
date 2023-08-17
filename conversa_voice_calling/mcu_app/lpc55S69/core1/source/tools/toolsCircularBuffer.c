/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tools.h"



/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferStructInit										     */
/*                                                                                               */
/* DESCRIPTION:    	    initialize a circular buffer structure with user parameters              */
/* PARAMETERS:          p_cirBuff           Circular buffer structure                            */
/*                      p_buffer			memory allocated buffer for the circular buffer      */
/*                      bufferSizeInBytes   size of memory (in bytes) allocated for p_Buffer     */
/*                      channelNum			number of channels in the circular buffer            */
/*                      sampleWidth         size in bytes of each samples in the circular buffer */
/*                                                                                               */
/*************************************************************************************************/
status_t toolsCircularBufferStructInit(TOOLS_circularBuffer_st*    p_cirBuff,
									   TOOLS_circularBuffer_param  param)
{
	status_t retStatus = kStatus_Success;
	PL_FLOAT temp_value;


	// check for non-null pointers.
	if    ( p_cirBuff == PL_NULL )
	{
		retStatus=  kStatus_NullPointer;
	}
	//initialize structure with user parameters
	p_cirBuff->cbSize 		= param.bBufferSizeInBytes;
	p_cirBuff->cbChannelNum = param.cbChannelNum;
	p_cirBuff->cbSampleWidth= param.cbSampleWidth;
	// initialize pointer to the beginning and the end of the memory allocated buffer
	p_cirBuff->p_cbStart = &param.p_cbBuffer[0];
	p_cirBuff->p_cbStop  = p_cirBuff->p_cbStart +p_cirBuff->cbSize ;
	// initialize read/write pointers to the beginning of the memory allocated buffer
	p_cirBuff->p_cbRead  = &param.p_cbBuffer[0];
	p_cirBuff->p_cbWrite = &param.p_cbBuffer[0];

	temp_value = (PL_FLOAT)(p_cirBuff->cbSize) / 3.0;
	p_cirBuff->lowLimitInBytes = (PL_UINT32)temp_value ;  // The low limit represent 1/3 of the full cbSize in bytes
	temp_value = (PL_FLOAT)(p_cirBuff->cbSize<<1) / 3.0;
	p_cirBuff->highLimitInBytes = (PL_UINT32)temp_value ;  // The High limit represent 2/3 of the full cbSize in bytes
	// Buffer is empty => midFull flag is false
	p_cirBuff->flagMidFull = 0;
	p_cirBuff->flagOptional = 1;							// by default the optional flag is set to 1 to avoid issue in case we don't use it

	return retStatus;

}


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
									  PL_UINT32                samplesPerChannelToRead )

{
	status_t retStatus = kStatus_Success;
	// To address different buffer format
	PL_UINT16** pp_Outs_16b 	  		   = (PL_UINT16**) pp_Outs; 			  					    // 16 bit audio data as output
	PL_UINT32** pp_Outs_32b 	  		   = (PL_UINT32**) pp_Outs;    			  					// 32 bit audio data as output
	PL_UINT16*  p_cbStart_16b 	  		   = (PL_UINT16*) p_cirBuff->p_cbStart ;  					// circular buffer start pointer 16bits
	PL_UINT32*  p_cbStart_32b 	  		   = (PL_UINT32*) p_cirBuff->p_cbStart;   					// circular buffer start pointer 32bits
	PL_UINT16*  p_cbRead_16b 	  		   = (PL_UINT16*) p_cirBuff->p_cbRead; 	  					// circular buffer read  pointer 16bits
	PL_UINT32*  p_cbRead_32b 	  	       = (PL_UINT32*) p_cirBuff->p_cbRead;    					// circular buffer read  pointer 32bits
	PL_UINT32  SamplePerChannelUntilCbStop = 0; 								  					// How many samples can be read until the end of the circular buffer.
	PL_UINT32  readableSamplesPerChannel   =  (toolsCircularBufferReadableData(p_cirBuff)        )
										     /(p_cirBuff->cbSampleWidth * p_cirBuff->cbChannelNum);	// number of samples readable
	volatile PL_INT32 iChOut 			   = 0;								  					    // loop for channels

	/********************/
	/* CHECK PARAMETERS */

	// check for non-null output channels to interleave
	for(iChOut = ((p_cirBuff->cbChannelNum)-1); iChOut >= 0; iChOut--)
	{
		if (pp_Outs[iChOut] == PL_NULL)
		{
			return kStatus_NullPointer;
		}
	}
	if (p_cirBuff->flagMidFull == 1)
	{
		/* IF enough sample in the circular buffer to be read */
		if (readableSamplesPerChannel>= samplesPerChannelToRead )
		{
			/********************/
			/* GET THE NUMBER OF READABLE SAMPLES UNTIL THE END OF THE CIRCULAR BUFFER */
			if (p_cirBuff->p_cbStop != p_cirBuff->p_cbRead)
			{
				SamplePerChannelUntilCbStop =   (p_cirBuff->p_cbStop - p_cirBuff->p_cbRead) 			// Bytes available between cbStop address and cbRead address
											   /(p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth);   // Bytes available / (number of channels * sampleWidth) = samples number until the end of the circular buffer
			}
			else
			{
				SamplePerChannelUntilCbStop = 0;
			}
			/********************/
			/* READ DATA FROM CIRCULAR BUFFER + DEINTERLEAVED THEM */

			/* 16 BITS data, the case we are in 16 bits of sample width */
			if (p_cirBuff->cbSampleWidth == 2)
			{
				// First case : enough data to read until we reach the end of the circular buff =>  continuous read
				if (samplesPerChannelToRead < SamplePerChannelUntilCbStop)
				{

					// Here we can de-inteleave data from p_cirBuff->p_cbRead + fill out buffers | from  pp_Outs_16b[0]                        [0]
					//                                                    						 | to    pp_Outs_16b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_16b(p_cbRead_16b,
															pp_Outs_16b,
															samplesPerChannelToRead,
															p_cirBuff->cbChannelNum,
															0);
					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbRead + samplesPerChannelToRead*p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;
				}
				// second case : enough data but need to reload cbWrite at cbStart
				else if (samplesPerChannelToRead == SamplePerChannelUntilCbStop)
				{

					// Here we can de-inteleave data from p_cirBuff->p_cbRead + fill out buffers | from  pp_Outs_16b[0]                        [0]
					//                                                    						 | to    pp_Outs_16b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_16b(p_cbRead_16b,
															pp_Outs_16b,
															samplesPerChannelToRead,
															p_cirBuff->cbChannelNum,
															0);
					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbStart;
				}
				// Third case : not enough data to read  until we reach the end of the circular buff =>  discontinuous read
				//               we will first read SamplePerChannelUntilCbStop samples from p_cirBuff->p_cbRead to p_cirBuff->p_cbStop
				//               then we read (samplesPerChannelToRead-SamplePerChannelUntilCbStop) from p_cirBuff->p_cbStart
				else if (samplesPerChannelToRead > SamplePerChannelUntilCbStop)
				{
					// First operation
					// Here we can de-inteleave data from p_cirBuff->p_cbRead and fill out buffers | from  pp_Outs_16b[0]                        [0]
					//                                                         					   | to    pp_Outs_16b[p_cirBuff->cbChannelNum-1][SamplePerChannelUntilCbStop]
					toolsCircularBufferDeInterleaveData_16b(p_cbRead_16b,
															pp_Outs_16b,
															SamplePerChannelUntilCbStop,
															p_cirBuff->cbChannelNum,
															0);
					// Second operation
					// Here we can de-inteleave data from p_cirBuff->p_cbStart and fill out buffers | from  pp_Outs_16b[p_cirBuff->cbChannelNum-1][SamplePerChannelUntilCbStop]
					//                                                         					    | to    pp_Outs_16b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_16b(p_cbStart_16b,
															pp_Outs_16b,
															(samplesPerChannelToRead-SamplePerChannelUntilCbStop),
															p_cirBuff->cbChannelNum,
															SamplePerChannelUntilCbStop);
					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbStart + (samplesPerChannelToRead-SamplePerChannelUntilCbStop) *p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;
				}
				// last case : error out of range
				else
				{
					retStatus = kStatus_OutOfRange;
				}
			}

			/* 32 BITS data, the case we are in 32 bits of sample width */
			else if (p_cirBuff->cbSampleWidth == 4)
			{
				// First case : enough data to read until we reach the end of the circular buff => continuous read
				if (samplesPerChannelToRead < SamplePerChannelUntilCbStop)
				{

					// Here we can de-inteleave data from p_cirBuff->p_cbRead + fill out buffers | from  pp_Outs_32b[0]                        [0]
					//                                                    						 | to    pp_Outs_32b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_32b(p_cbRead_32b,
															pp_Outs_32b,
															samplesPerChannelToRead,
															p_cirBuff->cbChannelNum,
															0);

					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbRead + samplesPerChannelToRead*p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;
				}
				// second case : enough data but need to reload cbWrite at cbStart
				else if (samplesPerChannelToRead == SamplePerChannelUntilCbStop)
				{

					// Here we can de-inteleave data from p_cirBuff->p_cbRead + fill out buffers | from  pp_Outs_32b[0]                        [0]
					//                                                    						 | to    pp_Outs_32b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_32b(p_cbRead_32b,
															pp_Outs_32b,
															samplesPerChannelToRead,
															p_cirBuff->cbChannelNum,
															0);

					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbStart;
				}
				// Second case : not enough data to read  until we reach the end of the circular buff =>  discontinuous read
				//               we will first read x samples from p_cirBuff->p_cbRead to p_cirBuff->p_cbStop
				//               then we read (samplesPerChannelToRead-x) from p_cirBuff->p_cbStart
				else if (samplesPerChannelToRead > SamplePerChannelUntilCbStop)
				{
					// First operation
					// Here we can de-inteleave data from p_cirBuff->p_cbRead and fill out buffers | from  pp_Outs_32b[0]                        [0]
					//                                                         					   | to    pp_Outs_32b[p_cirBuff->cbChannelNum-1][SamplePerChannelUntilCbStop]
					toolsCircularBufferDeInterleaveData_32b(p_cbRead_32b,
															pp_Outs_32b,
															SamplePerChannelUntilCbStop,
															p_cirBuff->cbChannelNum,
															0);
					// Second operation
					// Here we can de-inteleave data from p_cirBuff->p_cbStart and fill out buffers | from  pp_Outs_32b[p_cirBuff->cbChannelNum-1][SamplePerChannelUntilCbStop]
					//                                                         					    | to    pp_Outs_32b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_32b(p_cbStart_32b,
															pp_Outs_32b,
															(samplesPerChannelToRead-SamplePerChannelUntilCbStop),
															p_cirBuff->cbChannelNum,
															SamplePerChannelUntilCbStop);
					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbStart + (samplesPerChannelToRead-SamplePerChannelUntilCbStop) *p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;

				}/*end if (samplesPerChannelToRead > SamplePerChannelUntilCbStop)*/
				// last case : error out of range
				else
				{
					retStatus = kStatus_OutOfRange;
				}
			}/*end if (p_cirBuff->cbSampleWidth == 4)*/

			/* 24 BITS data, the case we are in 24 bits of sample width */
			else if (p_cirBuff->cbSampleWidth == 3)
			{
				// First case : enough data to read until we reach the end of the circular buff => continuous read
				if (samplesPerChannelToRead < SamplePerChannelUntilCbStop)
				{

					// Here we can de-inteleave data from p_cirBuff->p_cbRead + fill out buffers | from  pp_Outs_32b[0]                        [0]
					//                                                    						 | to    pp_Outs_32b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_24b_for32bContainer(p_cirBuff->p_cbRead,
																			pp_Outs,
																			samplesPerChannelToRead,
																			p_cirBuff->cbChannelNum,
																			0);

					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbRead + samplesPerChannelToRead*p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;
				}
				// second case : enough data but need to reload cbWrite at cbStart
				else if (samplesPerChannelToRead == SamplePerChannelUntilCbStop)
				{

					// Here we can de-inteleave data from p_cirBuff->p_cbRead + fill out buffers | from  pp_Outs_32b[0]                        [0]
					//                                                    						 | to    pp_Outs_32b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_24b_for32bContainer(p_cirBuff->p_cbRead,
																			pp_Outs,
																			samplesPerChannelToRead,
																			p_cirBuff->cbChannelNum,
																			0);
					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbStart;
				}
				// Second case : not enough data to read  until we reach the end of the circular buff =>  discontinuous read
				//               we will first read x samples from p_cirBuff->p_cbRead to p_cirBuff->p_cbStop
				//               then we read (samplesPerChannelToRead-x) from p_cirBuff->p_cbStart
				else if (samplesPerChannelToRead > SamplePerChannelUntilCbStop)
				{
					// First operation
					// Here we can de-inteleave data from p_cirBuff->p_cbRead and fill out buffers | from  pp_Outs_32b[0]                        [0]
					//                                                         					   | to    pp_Outs_32b[p_cirBuff->cbChannelNum-1][SamplePerChannelUntilCbStop]
					toolsCircularBufferDeInterleaveData_24b_for32bContainer(p_cirBuff->p_cbRead,
																			pp_Outs,
																			SamplePerChannelUntilCbStop,
																			p_cirBuff->cbChannelNum,
																			0);
					// Second operation
					// Here we can de-inteleave data from p_cirBuff->p_cbStart and fill out buffers | from  pp_Outs_32b[p_cirBuff->cbChannelNum-1][SamplePerChannelUntilCbStop]
					//                                                         					    | to    pp_Outs_32b[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead]
					toolsCircularBufferDeInterleaveData_24b_for32bContainer(p_cirBuff->p_cbStart,
																			pp_Outs,
																			(samplesPerChannelToRead-SamplePerChannelUntilCbStop),
																			p_cirBuff->cbChannelNum,
																			SamplePerChannelUntilCbStop);

					// Update of the read pointer
					p_cirBuff->p_cbRead = p_cirBuff->p_cbStart + (samplesPerChannelToRead-SamplePerChannelUntilCbStop) *p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;

				}/*end if (samplesPerChannelToRead > SamplePerChannelUntilCbStop)*/
				// last case : error out of range
				else
				{
					retStatus = kStatus_OutOfRange;
				}
			}/*end if (p_cirBuff->cbSampleWidth == 4)*/
			else
			{
				retStatus = kStatus_NotCompatible;
			}
		} /*end if (readableSamplesPerChannel> samplesPerChannelToRead )*/

		/* ELSE NOT enough sample in the circular buffer to be read => Buffer is empty */
		else
		{
			for(iChOut = ((p_cirBuff->cbChannelNum)-1); iChOut >= 0; iChOut--)
			{
				memset(pp_Outs[iChOut], 0x0,samplesPerChannelToRead *4/*p_cirBuff->cbSampleWidth*/  );
			}
			retStatus = kStatus_cbEmpty;

			p_cirBuff->flagMidFull = 0;		// reset the mid full flag to force wait mid full data to be write beforre authorize the read
		}
	}/* end if (p_cirBuff->flagMidFull == 1) */
	else
	{
		for(iChOut = ((p_cirBuff->cbChannelNum)-1); iChOut >= 0; iChOut--)
		{
			memset(pp_Outs[iChOut], 0x0,samplesPerChannelToRead*4/*p_cirBuff->cbSampleWidth*/  );
		}
	}

	return retStatus;
}
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
									  PL_UINT32               bytesToRead )

{
	status_t retStatus = kStatus_Success;
	// To address different buffer format and different direction in or out in the function

	PL_UINT32  BytesUntilcbStop 			= (p_cirBuff->p_cbStop - p_cirBuff->p_cbRead);		      // Bytes available between cbStop address and cbRead address
	/********************/
	/* CHECK PARAMETERS */

	// check for non-null output buffer
	if (p_Out == PL_NULL)
	{
		return kStatus_NullPointer;
	}

	/********************/
	/* READ DATA FROM CIRCULAR BUFFER + COPY THEM INTERLEAVED */
	// First case : enough data to read until we reach the end of the circular buff =>  continuous read
	if (bytesToRead < BytesUntilcbStop)
	{
		// copy the amount of data from cirBuff to p_Out
		memcpy(p_Out,p_cirBuff->p_cbRead,bytesToRead );
		// Update of the read pointer
		p_cirBuff->p_cbRead = p_cirBuff->p_cbRead + bytesToRead;
	}
	// second case= enough data but need to reload the pointer at the beginning
	else if (bytesToRead == BytesUntilcbStop)
	{
		// copy the amount of data from cirBuff to p_Out
		memcpy(p_Out,p_cirBuff->p_cbRead,bytesToRead );
		// Update of the read pointer
		p_cirBuff->p_cbRead = p_cirBuff->p_cbStart;
	}
	// third case : not enough data to read  until we reach the end of the circular buff =>  discontinuous read
	//               we will first read BytesUntilcbStop from p_cirBuff->p_cbRead to p_cirBuff->p_cbStop
	//               then we read (bytesToRead-BytesUntilcbStop) from p_cirBuff->p_cbStart
	else if (bytesToRead > BytesUntilcbStop)
	{
		// First operation
		// we will first read BytesUntilcbStop samples from p_cirBuff->p_cbRead to p_cirBuff->p_cbStop
		memcpy(&p_Out[0],p_cirBuff->p_cbRead,BytesUntilcbStop );
		// Second operation
		// then we read (bytesToRead-BytesUntilcbStop) from p_cirBuff->p_cbStart
		memcpy(&p_Out[BytesUntilcbStop],p_cirBuff->p_cbStart,bytesToRead-BytesUntilcbStop );
		// Update of the read pointer
		p_cirBuff->p_cbRead = p_cirBuff->p_cbStart + (bytesToRead-BytesUntilcbStop);
	}
	// last case : error out of range
	else
	{
		retStatus = kStatus_OutOfRange;
	}
	return retStatus;
}
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
									   PL_UINT32                samplesPerChannelToWrite )

{
	status_t retStatus = kStatus_Success;
	// To address different buffer format
	PL_UINT16** pp_Ins_16b 	  			= (PL_UINT16**) pp_Ins; 			      					// 16 bit audio data as input
	PL_UINT32** pp_Ins_32b 	  			= (PL_UINT32**) pp_Ins;    			  						// 32 bit audio data as input
	PL_UINT16*  p_cbStart_16b 	  		= (PL_UINT16*) p_cirBuff->p_cbStart ;  						// circular buffer start pointer 16bits
	PL_UINT32*  p_cbStart_32b 	  		= (PL_UINT32*) p_cirBuff->p_cbStart;   						// circular buffer start pointer 32bits
	PL_UINT16*  p_cbWrite_16b 	  		= (PL_UINT16*) p_cirBuff->p_cbWrite;   						// circular buffer write pointer 16bits
	PL_UINT32*  p_cbWrite_32b 	  	    = (PL_UINT32*) p_cirBuff->p_cbWrite;   						// circular buffer write pointer 32bits
	PL_UINT32  SampleMemoryUntilCbStop  = 0; 								  						// How many samples can be read until the end of the circular buffer.
	PL_UINT32  writableSamplesPerChannel=  (toolsCircularBufferWritableData(p_cirBuff)        )     // number of samples writable
										  /(p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth);
	volatile PL_INT32 iChIn 			= 0;								  						// loop for channels


	/********************/
	/* CHECK PARAMETERS */

	// check for non-null output channels to interleave
	for(iChIn = ((p_cirBuff->cbChannelNum)-1); iChIn >= 0; iChIn--)
	{
		if (pp_Ins[iChIn] == PL_NULL)
		{
			return kStatus_NullPointer;
		}
	}
	/********************/
	/* GET THE NUMBER OF SAMPLES WRITABLE UNTIL THE END OF THE CIRCULAR BUFFER */
	if (p_cirBuff->p_cbStop != p_cirBuff->p_cbWrite)
	{
		SampleMemoryUntilCbStop =   (p_cirBuff->p_cbStop - p_cirBuff->p_cbWrite) 			// Bytes available between cbStop address and cbRead address
									/(p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth) ; // Bytes available / (number of channels * sampleWidth) = samples number until the end of the circular buffer
	}
	else
	{
		SampleMemoryUntilCbStop = 0;
	}
	if (toolsCircularBufferReadableData(p_cirBuff) >= (p_cirBuff->cbSize/2))
	{
		p_cirBuff->flagMidFull = 1;
	}

	/********************/
	/* WRITE DATA TO CIRCULAR BUFFER + INTERLEAVED THEM  */
	// not enough writable memory => buffer is full

	// the case we are in 16 bits of sample width
	if (p_cirBuff->cbSampleWidth == 2)
	{
		// First case : enough memory to write until we reach the end of the circular buff =>  continuous write
		if (samplesPerChannelToWrite < SampleMemoryUntilCbStop)
		{

			// Here we can  copy data   | from  pp_Ins[0]                        [0]						|     and  inteleave them to p_cirBuff->p_cbWrite
			//                          | to    pp_Ins[p_cirBuff->cbChannelNum-1][samplesPerChannelToWrite] |
			toolsCircularBufferInterleaveData_16b(pp_Ins_16b,
												  p_cbWrite_16b,
												  samplesPerChannelToWrite,
												  p_cirBuff->cbChannelNum,
												  0);
			// Update of the read pointer
			p_cirBuff->p_cbWrite = p_cirBuff->p_cbWrite + samplesPerChannelToWrite*p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;

		}
		else if (samplesPerChannelToWrite == SampleMemoryUntilCbStop)
		{

			// Here we can  copy data   | from  pp_Ins[0]                        [0]						|     and  inteleave them to p_cirBuff->p_cbWrite
			//                          | to    pp_Ins[p_cirBuff->cbChannelNum-1][samplesPerChannelToWrite] |
			toolsCircularBufferInterleaveData_16b(pp_Ins_16b,
												  p_cbWrite_16b,
												  samplesPerChannelToWrite,
												  p_cirBuff->cbChannelNum,
												  0);
			// Update of the read pointer
			p_cirBuff->p_cbWrite =p_cirBuff->p_cbStart ;

		}
		// Second case : not enough memory to write  until we reach the end of the circular buff =>  discontinuous write
		//               we will first write SampleMemoryUntilCbStop samples between p_cirBuff->p_cbWrite to p_cirBuff->p_cbStop
		//               then we write (samplesPerChannelToWrite-SampleMemoryUntilCbStop) at p_cirBuff->p_cbStart
		else if (samplesPerChannelToWrite > SampleMemoryUntilCbStop)
		{
			// First operation
			// Here we can  copy data   | from  pp_Ins[0]                        [0]						|     and  interleave them to p_cirBuff->p_cbWrite
			//                          | to    pp_Ins[p_cirBuff->cbChannelNum-1][SampleMemoryUntilCbStop]  |
			toolsCircularBufferInterleaveData_16b(pp_Ins_16b ,
												  p_cbWrite_16b,
												  SampleMemoryUntilCbStop,
												  p_cirBuff->cbChannelNum,
												  0);
			// Second operation
			// Here we can  copy data   | from  pp_Ins[p_cirBuff->cbChannelNum-1][SampleMemoryUntilCbStop] |   and  interleave them to p_cirBuff->p_cbStart
			//                          | to    pp_Ins[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead ] |
			toolsCircularBufferInterleaveData_16b(pp_Ins_16b ,
												  p_cbStart_16b,
												  (samplesPerChannelToWrite-SampleMemoryUntilCbStop),
												  p_cirBuff->cbChannelNum,
												  SampleMemoryUntilCbStop);
			// Update of the write pointer
			p_cirBuff->p_cbWrite = p_cirBuff->p_cbStart + (samplesPerChannelToWrite-SampleMemoryUntilCbStop) * p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;
		}
		// last case : error out of range
		else
		{
			retStatus = kStatus_OutOfRange;
		}
	}/*(p_cirBuff->cbSampleWidth == 2)*/
	// the case we are in 32 bits of sample width
	else if (p_cirBuff->cbSampleWidth == 4)
	{
		// First case : enough memory to write until we reach the end of the circular buff =>  continuous write
				if (samplesPerChannelToWrite < SampleMemoryUntilCbStop)
				{

					// Here we can inteleave data to p_cirBuff->p_cbWrite + copy buffers   | from  pp_Ins[0]                        [0]
					//                                                    				   | to    pp_Ins[p_cirBuff->cbChannelNum-1][samplesPerChannelToWrite]
					toolsCircularBufferInterleaveData_32b(pp_Ins_32b ,
														  p_cbWrite_32b,
														  samplesPerChannelToWrite,
														  p_cirBuff->cbChannelNum,
														  0);
					// Update of the read pointer
					p_cirBuff->p_cbWrite = p_cirBuff->p_cbWrite + samplesPerChannelToWrite*p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;
				}
				else if (samplesPerChannelToWrite == SampleMemoryUntilCbStop)
				{

					// Here we can inteleave data to p_cirBuff->p_cbWrite + copy buffers   | from  pp_Ins[0]                        [0]
					//                                                    				   | to    pp_Ins[p_cirBuff->cbChannelNum-1][samplesPerChannelToWrite]
					toolsCircularBufferInterleaveData_32b(pp_Ins_32b ,
														  p_cbWrite_32b,
														  samplesPerChannelToWrite,
														  p_cirBuff->cbChannelNum,
														  0);
					// Update of the read pointer
					p_cirBuff->p_cbWrite = p_cirBuff->p_cbStart;
				}
				// third case : not enough memory to write  until we reach the end of the circular buff =>  discontinuous write
				//               we will first write SampleMemoryUntilCbStop samples from p_cirBuff->p_cbWrite to p_cirBuff->p_cbStop
				//               then we write (samplesPerChannelToWrite-SampleMemoryUntilCbStop) from p_cirBuff->p_cbStart
				else if (samplesPerChannelToWrite > SampleMemoryUntilCbStop)
				{
					// First operation
					// Here we can  copy data   | from  pp_Ins[0]                        [0]						|     and  inteleave them to p_cirBuff->p_cbWrite
					//                          | to    pp_Ins[p_cirBuff->cbChannelNum-1][SampleMemoryUntilCbStop] |
					toolsCircularBufferInterleaveData_32b(pp_Ins_32b ,
														  p_cbWrite_32b,
														  SampleMemoryUntilCbStop,
														  p_cirBuff->cbChannelNum,
														  0);
					// Second operation
					// Here we can  copy data   | from  pp_Ins[p_cirBuff->cbChannelNum-1][SampleMemoryUntilCbStop] |   and  inteleave them to p_cirBuff->p_cbStart
					//                          | to    pp_Ins[p_cirBuff->cbChannelNum-1][samplesPerChannelToRead ] |
					toolsCircularBufferInterleaveData_32b(pp_Ins_32b,
														  p_cbStart_32b,
														  (samplesPerChannelToWrite-SampleMemoryUntilCbStop),
														  p_cirBuff->cbChannelNum,
														  SampleMemoryUntilCbStop);

					// Update of the read pointer
					p_cirBuff->p_cbWrite = p_cirBuff->p_cbStart + (samplesPerChannelToWrite-SampleMemoryUntilCbStop) * p_cirBuff->cbChannelNum * p_cirBuff->cbSampleWidth;
				}
				// last case : error out of range
				else
				{
					retStatus = kStatus_OutOfRange;
				}
	}
	else
	{
		retStatus = kStatus_NotCompatible;
	}

	if (   (samplesPerChannelToWrite >= writableSamplesPerChannel )
		&& (p_cirBuff->flagMidFull==1)
		&& (p_cirBuff->flagOptional ==1))                               // if we don't use optional flag, the value is set to 1 by default
	{
		// but we continue to write data normally
		retStatus = kStatus_cbFull;
		p_cirBuff->flagMidFull = 0;
	}
	return retStatus;
}

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
									  PL_UINT32                 bytesToWrite )

{
	status_t retStatus = kStatus_Success;
	PL_UINT32  MemoryUntilCbStop         = 0; 								  	 // How many samples can be read until the end of the circular buffer.
	/********************/
	/* CHECK PARAMETERS */

	// check for non-null input buffer
	if (p_In == PL_NULL)
	{
		return kStatus_NullPointer;
	}

	/********************/
	/* GET THE NUMBER OF BYTES WRITABLE UNTIL THE END OF THE CIRCULAR BUFFER */
	MemoryUntilCbStop =   p_cirBuff->p_cbStop - p_cirBuff->p_cbWrite ;			 // Bytes available between cbStop address and cbWrite address

	/********************/
	/* WRITE DATA TO CIRCULAR BUFFER */

	// First case : enough data to write until we reach the end of the circular buff =>  continuous write
	if (bytesToWrite < MemoryUntilCbStop)
	{
		// copy the amount of data from p_In to cirBuff
		memcpy(p_cirBuff->p_cbWrite,p_In,bytesToWrite );
		// Update of the write pointer
		p_cirBuff->p_cbWrite = p_cirBuff->p_cbWrite + bytesToWrite;
	}
	// second case : reload pointer to cbStart
	else if (bytesToWrite == MemoryUntilCbStop)
	{
		// copy the amount of data from p_In to cirBuff
		memcpy(p_cirBuff->p_cbWrite,p_In,bytesToWrite );
		// Update of the write pointer
		p_cirBuff->p_cbWrite = p_cirBuff->p_cbStart;
	}
	// third case : not enough data to read  until we reach the end of the circular buff =>  discontinuous read
	//               we will first write MemoryUntilCbStop between p_cirBuff->p_cbWrite and p_cirBuff->p_cbStop
	//               then we write (bytesToWrite-MemoryUntilCbStop) at p_cirBuff->p_cbStart
	else if (bytesToWrite > MemoryUntilCbStop)
	{
		// First operation
		//  we will first write MemoryUntilCbStop between p_cirBuff->p_cbWrite to p_cirBuff->p_cbStop
		memcpy(p_cirBuff->p_cbWrite,&p_In[0],MemoryUntilCbStop );
		// Second operation
		// then we write (bytesToWrite-MemoryUntilCbStop) from p_cirBuff->p_cbStart
		memcpy(p_cirBuff->p_cbStart,&p_In[MemoryUntilCbStop],bytesToWrite-MemoryUntilCbStop );
		// Update of the read pointer
		p_cirBuff->p_cbWrite = p_cirBuff->p_cbStart + (bytesToWrite-MemoryUntilCbStop);
		}
		// last case : error out of range
	else
	{
		retStatus = kStatus_OutOfRange;
	}
	return retStatus;
}

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferReadableData									 		 */
/*                                                                                               */
/* DESCRIPTION:    	    return readable amount of data     										 */
/*                                                             									 */
/* PARAMETERS:          p_cirBuff       circular buffer                     			         */
/*                                                                                               */
/*************************************************************************************************/

PL_UINT32 toolsCircularBufferReadableData(TOOLS_circularBuffer_st*  p_cirBuff)
{
	if (p_cirBuff->p_cbWrite > p_cirBuff->p_cbRead)
	{
		return  (PL_UINT32) (p_cirBuff->p_cbWrite - p_cirBuff->p_cbRead - 1);

	}
	else if (p_cirBuff->p_cbWrite < p_cirBuff->p_cbRead)
	{
		return (  ( (PL_UINT32) (p_cirBuff->p_cbStop   -  p_cirBuff->p_cbRead) )
				+ ( (PL_UINT32) (p_cirBuff->p_cbWrite  - p_cirBuff->p_cbStart) )    );
	}
	else/* (p_cirBuff->p_cbWrite  = p_cirBuff->p_cbRead) */
	{
		return 0;
	}


}

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferWritableData									 		 */
/*                                                                                               */
/* DESCRIPTION:    	    return writable amount of data     										 */
/*                                                             									 */
/* PARAMETERS:          p_cirBuff       circular buffer                     			         */
/*                                                                                               */
/*************************************************************************************************/

PL_UINT32 toolsCircularBufferWritableData(TOOLS_circularBuffer_st*  p_cirBuff)
{
	if (p_cirBuff->p_cbWrite < p_cirBuff->p_cbRead)
	{
		return  (PL_UINT32) (p_cirBuff->p_cbRead - p_cirBuff->p_cbWrite - 1 );

	}
	else if (p_cirBuff->p_cbWrite > p_cirBuff->p_cbRead)
	{
		return (  ( (PL_UINT32) (p_cirBuff->p_cbStop   - p_cirBuff->p_cbWrite ) )
				+ ( (PL_UINT32) (p_cirBuff->p_cbRead   - p_cirBuff->p_cbStart ) )    );
	}
	else
		return 0;


}

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
											     PL_UINT32  OutIndex)
{
	status_t retStatus = kStatus_Success;

	// process variable
	PL_INT16 offsetSampleIn 	= 0;  		 // sample offset in the input buffer
	PL_UINT32 firstIndex 	    = OutIndex;  // pp_out16b sample Index where to begin the de-interleaving operation
	// loop indice variable
	volatile PL_INT16 iCh 		= 0;
	volatile PL_INT16 iSpl 		= 0;

	/********************/
	/* CHECK PARAMETERS */
	// check for not null output channels to interleave
	for(iCh = numChannels; iCh>0; iCh--)
	{
		if (pp_out16b[iCh-1] == NULL)
		{
			return kStatus_NullPointer;
		}
	}

	// check for not null input buffer
	if (p_in16b == NULL)
	{
		return kStatus_NullPointer;
	}

	/********************/
	/* DE-INTERLEAVE DATA */

	// loop for channels
	for (iCh = 0; iCh < numChannels; iCh++)
	{
		// loop for samples
		for(iSpl=0; iSpl<numSamples; iSpl++)
		{
			offsetSampleIn = iCh + iSpl*numChannels;
			pp_out16b[iCh][firstIndex+iSpl] = p_in16b[offsetSampleIn];
		}
	}

		return retStatus;
}

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
											     PL_UINT32 OutIndex)
{
	status_t retStatus = kStatus_Success;

	// process variable
	PL_INT16 offsetSampleIn 	= 0;  		 // sample offset in the input buffer
	PL_UINT32 firstIndex 	    = OutIndex;  // pp_out116b sample Index where to begin the de-interleaving operation
	// loop indice variable
	volatile PL_INT32 iCh 		= 0;
	volatile PL_INT32 iSpl 		= 0;

	/********************/
	/* CHECK PARAMETERS */
	// check for not null output channels to interleave
	for(iCh = numChannels; iCh>0; iCh--)
	{
		if (pp_out32b[iCh-1] == NULL)
		{
			return kStatus_NullPointer;
		}
	}

	// check for not null input buffer
	if (p_in32b == NULL)
	{
		return kStatus_NullPointer;
	}

	/********************/
	/* DE-INTERLEAVE DATA */

	// loop for channels
	for (iCh = 0; iCh < numChannels; iCh++)
	{
		// loop for samples
		for(iSpl=0; iSpl<numSamples; iSpl++)
		{
			offsetSampleIn = iCh + iSpl*numChannels;
			pp_out32b[iCh][firstIndex+iSpl] = p_in32b[offsetSampleIn];

		}
	}

		return retStatus;
}


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
											   PL_UINT32  InIndex)
{
	status_t retStatus = kStatus_Success;

	// process variable
	PL_INT16 offsetSampleOut 	= 0;  		 // sample offset in the output buffer
	PL_UINT32 firstIndex 	    = InIndex;  // pp_out16b sample Index where to begin the de-interleaving operation
	// loop indice variable
	volatile PL_INT16 iCh 		= 0;
	volatile PL_INT16 iSpl 		= 0;

	/********************/
	/* CHECK PARAMETERS */
	// check for not null input channels to interleave
	for(iCh = numChannels; iCh>0; iCh--)
	{
		if (pp_in16b[iCh-1] == NULL)
		{
			return kStatus_NullPointer;
		}
	}

	// check for not null output buffer
	if (p_out16b == NULL)
	{
		return kStatus_NullPointer;
	}

	/********************/
	/* INTERLEAVE DATA */

	// loop for channels
	for (iCh = 0; iCh < numChannels; iCh++)
	{
		// loop for samples
		for(iSpl=0; iSpl<numSamples; iSpl++)
		{
			offsetSampleOut = iCh + iSpl*numChannels;
			p_out16b[offsetSampleOut] = pp_in16b[iCh][firstIndex+iSpl];

		}
	}

		return retStatus;
}

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
											   PL_UINT32  InIndex)
{
	status_t retStatus = kStatus_Success;

	// process variable
	PL_INT32 offsetSampleOut 	= 0;  		 // sample offset in the output buffer
	PL_UINT32 firstIndex 	    = InIndex;  // pp_out32b sample Index where to begin the de-interleaving operation
	// loop indice variable
	volatile PL_INT16 iCh 		= 0;
	volatile PL_INT16 iSpl 		= 0;

	/********************/
	/* CHECK PARAMETERS */
	// check for not null input channels to interleave
	for(iCh = numChannels; iCh>0; iCh--)
	{
		if (pp_in32b[iCh-1] == NULL)
		{
			return kStatus_NullPointer;
		}
	}

	// check for not null output buffer
	if (p_out32b == NULL)
	{
		return kStatus_NullPointer;
	}

	/********************/
	/* INTERLEAVE DATA */

	// loop for channels
	for (iCh = 0; iCh < numChannels; iCh++)
	{
		// loop for samples
		for(iSpl=0; iSpl<numSamples; iSpl++)
		{
			offsetSampleOut = iCh + iSpl*numChannels;
			p_out32b[offsetSampleOut] = pp_in32b[iCh][firstIndex+iSpl];

		}
	}

		return retStatus;
}

/*************************************************************************************************/
/*                                                                                               */
/* FUNCTION:            toolsCircularBufferDeInterleaveData_24b_for32bContainer					 */
/*                                                                                               */
/* DESCRIPTION:    	    de-interleave  numSamples from p_in8b + copy  in pp_out8b starting       */
/*                      from index pp_out32b[0][OutIndex]                                        */
/* PARAMETERS:          p_in32b        input data to deinterleave                      			 */
/*                      pp_out32b	   Output deinterleaved buffers                              */
/*                      numSamples     Number of samples per channel to de-interleave            */
/*                      numChannels    Number of channels to de-interleave                       */
/*                      OutIndex       pp_out16b sample index where to begin de-int operation    */
/*                                                                                               */
/*************************************************************************************************/

status_t toolsCircularBufferDeInterleaveData_24b_for32bContainer(PL_UINT8 * p_in8b,
																 PL_UINT8 ** pp_out8b,
																 PL_UINT32 numSamples,
																 PL_UINT32 numChannels,
																 PL_UINT32 OutIndex)
{
	status_t retStatus = kStatus_Success;
	// process variable
	PL_INT16 offsetBytesIn 	= 0;  		 	 // sample offset in the input buffer
	PL_INT16 offsetBytesOut = 0;  		 	 // sample offset in the output buffer
	PL_UINT32 firstIndex_byte 	= OutIndex;  // pp_out116b sample Index where to begin the de-interleaving operation
	// loop indice variable
	volatile PL_INT32 iCh 		= 0;
	volatile PL_INT32 iSpl 		= 0;
	PL_UINT32 byte1;
	PL_UINT32 byte2;
	PL_UINT32 byte3;


	/********************/
	/* CHECK PARAMETERS */
	// check for not null output channels to interleave
	for(iCh = numChannels; iCh>0; iCh--)
	{
		if (pp_out8b[iCh-1] == NULL)
		{
			return kStatus_NullPointer;
		}
	}

	// check for not null input buffer
	if (p_in8b == NULL)
	{
		return kStatus_NullPointer;
	}

	/********************/
	/* DE-INTERLEAVE DATA */

	// loop for channels

	for (iCh = 0; iCh < numChannels; iCh++)
	{
		firstIndex_byte = OutIndex * 4; 	// *4 because OutIndex unit is sample and firstIndex unit is byte
		// loop for samples
		for(iSpl=0; iSpl<numSamples; iSpl++)
		{
			offsetBytesIn  = (iCh + iSpl*numChannels)*3;
			pp_out8b[iCh][firstIndex_byte++] = 0x00;
			pp_out8b[iCh][firstIndex_byte++] = (p_in8b[offsetBytesIn]);
			pp_out8b[iCh][firstIndex_byte++] = (p_in8b[offsetBytesIn+1]);
		    pp_out8b[iCh][firstIndex_byte++] = (p_in8b[offsetBytesIn+2]);
		}
	}
		return retStatus;
}


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
							            PL_UINT32 			      offsetInBytes)
{
	PL_UINT32  BytesUntilcbStop 			= (p_cirBuff->p_cbStop - p_cirBuff->p_cbRead);		      // Bytes available between cbStop address and cbRead address
	// check for not null input buffer
	if (p_cirBuff == NULL)
	{
		return kStatus_NullPointer;
	}
	// check offset size
	if (offsetInBytes>= p_cirBuff->cbSize)
	{
		return kStatus_OutOfRange;
	}
	if (BytesUntilcbStop > offsetInBytes)
	{
	p_cirBuff->p_cbRead = p_cirBuff->p_cbRead + offsetInBytes;
	}
	else if (BytesUntilcbStop == offsetInBytes)
	{
		p_cirBuff->p_cbRead = p_cirBuff->p_cbStart;
	}
	else if (BytesUntilcbStop < offsetInBytes)
	{
		p_cirBuff->p_cbRead = p_cirBuff->p_cbStart + (offsetInBytes - BytesUntilcbStop);
	}
	else
	{
		return kStatus_NotCompatible;
	}
	return kStatus_Success;


}





