#include <usb.h>

struct usb_device* get_usb_device(int vendor_id, int device_id);

int claim_device(usb_dev_handle* device_handle, int interface);
