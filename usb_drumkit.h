#ifndef USB_DRUMKIT_H
#define USB_DRUMKIT_H

#define USB_DRUMKIT_NUM_PADS 6

int usb_drumkit_open();
int usb_drumkit_process_events(void (*callback)(int));
void usb_drumkit_close();

#endif
