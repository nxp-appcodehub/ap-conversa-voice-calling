/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef DSP_IMAGE_COPY_TO_RAM
#define DSP_IMAGE_COPY_TO_RAM 1
#endif

		.end

#if DSP_IMAGE_COPY_TO_RAM==1

#if defined(__GNUC__) || defined(__ARMCC_VERSION)

        .section .Core1App , "ax" @progbits @preinit_array

        .global Core1_image_start
        .type Core1_image_start, %object
        .align 4
Core1_image_start:

		#ifdef DEBUG
	        .incbin "。。/Co re1_Debug.bin"
        #else
    	    .incbin "../Co re1_Release.bin"
        #endif

        .global Core1_image_end
        .type Core1_image_end, %object
Core1_image_end:




        .global Core1_image_size
        .type Core1_image_size, %object
        .align 4
Core1_image_size:
        .int  Core1_image_end - Core1_image_start

//        .end

#endif

#endif // DSP_IMAGE_COPY_TO_RAM==1


        .text
        .global ConversaParaBin_Beg
        .global ConversaParaBin_End
        .align 4
ConversaParaBin_Beg: // Include Conversa parameters here
//        .incbin "tunings/whisper_tuning2.bin"
ConversaParaBin_End:
		.byte 0,0,0,0

        .end
