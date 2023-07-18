/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef USB_INIT_H
#define USB_INIT_H


#include "appGlobal.h"
#include "usb_audio_unified.h"
#include "clock_config.h"

void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
status_t USB_Init(usb_device_composite_struct_t *p_usbCompositeStruct);
extern usb_status_t USB_DeviceCdcVcomInit(usb_device_composite_struct_t *deviceComposite);



#endif/*USB_INIT_H*/
