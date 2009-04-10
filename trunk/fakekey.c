#include <fakekey/fakekey.h>

#define ENTER_KEY   0xff0d 
#define SPACE_KEY   0x20
#define UP_KEY      0xff52
#define DOWN_KEY    0xff54
#define LEFT_KEY    0xff51
#define RIGHT_KEY   0xff53

static FakeKey *fk;
static unsigned int num_keys;

static unsigned int pad_keys[] = { 
    ENTER_KEY, SPACE_KEY, UP_KEY, DOWN_KEY, LEFT_KEY, RIGHT_KEY 
};

    int fakekey_setup(unsigned int nkeys) {
        Display *dpy = XOpenDisplay(NULL);
        num_keys = nkeys;

        if (dpy == NULL) {
            return 1;
        }

        fk = fakekey_init(dpy);
        return 0;
    }


void fakekey_send(unsigned int pad_num)
{
    if (pad_num > num_keys) { 
        return;
    }

    fakekey_press_keysym(fk, pad_keys[pad_num], 1);
    fakekey_release(fk);
}
