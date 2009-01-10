/*
   Copyright (C) 2009 Todd Kirby (ffmpeg.php@gmail.com)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <usb.h>
#include "usb_utils.h"

struct usb_device* get_usb_device(int vendor_id, int device_id)
{
    struct usb_bus* bus = NULL;
    struct usb_device* device = NULL;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    for(bus = usb_get_busses(); bus; bus = bus->next) {
        for(device = bus->devices; device; device = device->next) {
            if (device->descriptor.idVendor == vendor_id &&
                    device->descriptor.idProduct == device_id) {
                return device;
            }
        }
    }

    return NULL;
}

int claim_usb_device(usb_dev_handle* device_handle, int interface)
{
#if LIBUSB_HAS_DETACH_KERNEL_DRIVER_NP
    // detach kernel hid driver.
    usb_detach_kernel_driver_np(device_handle, interface);
#endif

    // claim the device
    return usb_claim_interface(device_handle, interface);
}
