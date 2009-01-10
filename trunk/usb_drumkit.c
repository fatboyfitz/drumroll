
#define USB_VENDOR_ID_DREAM_CHEEKY 0x1941
#define USB_DEVICE_ID_ROLL_UP_DRUMKIT 0x8021
#define USB_INTERFACE_NUMBER 0x00
#define USB_ENDPOINT 0x81

#include <stdio.h>
#include <errno.h>

#include <usb.h>
#include "usb_drumkit.h"
#include "usb_utils.h"

static usb_dev_handle* drumkit_handle = NULL;
struct usb_device* usb_drumkit_device = NULL;

int usb_drumkit_open()
{
    usb_drumkit_device = get_usb_device(USB_VENDOR_ID_DREAM_CHEEKY, USB_DEVICE_ID_ROLL_UP_DRUMKIT);

    if (usb_drumkit_device  == NULL) {
        fprintf(stderr, "ERROR: couldn't find drumkit\n");
        return 1;
    }
    
    drumkit_handle = usb_open(usb_drumkit_device);

    if (drumkit_handle == NULL) {
        fprintf(stderr, "ERROR: opening drumkit\n");
        return 2; 
    }

    if (claim_usb_device(drumkit_handle, 0x00)) {
        fprintf(stderr, "ERROR: claiming drumkit\n");
        return 3;
    }
}


int usb_drumkit_close()
{
    usb_release_interface(drumkit_handle, USB_INTERFACE_NUMBER);
    usb_close(drumkit_handle);
}


/*
 * Drumkit event loop
 */
int usb_drumkit_process_events(void (*callback)(int))
{
    char drum_state;
    int pad_num;
    static char last_drum_state = 0;

    // read pad status from device
    while (usb_bulk_read(drumkit_handle, USB_ENDPOINT, &drum_state, 1, 0) >= 0) {
        if (drum_state == last_drum_state) {
            continue;
        }

        for (pad_num = 0; pad_num < USB_DRUMKIT_NUM_PADS; pad_num++) {
            if (((drum_state ^ last_drum_state) & drum_state) & (1 << pad_num)) {
                callback(pad_num);
            }
        }

        last_drum_state = drum_state;
    }

    fprintf(stderr, "ERROR: reading from usb device.\n Reason: %s\n", strerror(errno));
    return -1;
}
