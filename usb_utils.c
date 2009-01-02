#include <usb.h>

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


void usb_release_and_close_device(usb_dev_handle* device_handle, int interface)
{
    if (device_handle) {
        usb_release_interface(device_handle, interface);
        usb_close(device_handle);
    }
}


