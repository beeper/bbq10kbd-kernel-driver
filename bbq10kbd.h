#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/time.h>

#include "bbq10kbd_i2c.h"
#include "bbq10kbd_keycodes.h"

#define DEVICE_NAME           "bbq10kbd"

// BBQ10 Compatible
#define REG_VER        0x01    // FW Version Register
#define REG_CFG        0x02    // Configuration Register
#define REG_INT        0x03    // Interrupt status register
#define REG_KEY        0x04    // Key Status Register
#define REG_BKL        0x05    // Backlight control register
#define REG_DEB        0x06    // Debounce control Register (not implemented)
#define REG_FRQ        0x07    // Polling frequency register (not implemented)
#define REG_RST        0x08    // Chip Reset Register 
#define REG_FIF        0x09    // Key Press FIFO Register
                               //
// BBQ20 Compatible
#define REG_CF2       0x14    // Key Press FIFO Register
#define REG_TOX        0x15    // Key Press FIFO Register
#define REG_TOY        0x16    // Key Press FIFO Register


// Interrupt status register (REG_INT = 0x03)
//
//Bit	Name	Description
//7	N/A	Currently not implemented.
//6	INT_TOUCH	The interrupt was generated by a trackpad motion.
//5	INT_GPIO	The interrupt was generated by a input GPIO changing level.
//4	INT_PANIC	Currently not implemented.
//3	INT_KEY	The interrupt was generated by a key press.
//2	INT_NUMLOCK	The interrupt was generated by Num Lock.
//1	INT_CAPSLOCK	The interrupt was generated by Caps Lock.
//0	INT_OVERFLOW	The interrupt was generated by FIFO overflow.
#define MASK_INT_TOUCH 0b1100000
#define MASK_INT_KEY 0b00001111


// Configuration Register 
//Bit	Name	            Description
//7	    CFG_USE_MODS	    Should Alt, Sym and the Shift keys modify the keys being reported.
//6	    CFG_REPORT_MODS	    Should Alt, Sym and the Shift keys be reported as well.
//5	    CFG_PANIC_INT	    Currently not implemented.
//4	    CFG_KEY_INT	        Should an interrupt be generated when a key is pressed.
//3	    CFG_NUMLOCK_INT	    Should an interrupt be generated when Num Lock is toggled.
//2	    CFG_CAPSLOCK_INT	Should an interrupt be generated when Caps Lock is toggled.
//1	    CFG_OVERFLOW_INT	Should an interrupt be generated when a FIFO overflow happens.
//0	    CFG_OVERFLOW_ON	    When a FIFO overflow happens, should the new entry still be pushed, overwriting the oldest one. If 0 then new entry is lost.
#define BBQ10_CFG_BITS 0b01011110


// CF2_TOUCH_INT (trackpad generates interrupts), but not USB keyboard /
// mouse support (we definitely don't want the Beeper echoing to the host
// console)
#define BBQ10_CF2_BITS 0b00000001

#define KEY_PRESSED  1
#define KEY_RELEASED 3


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Billy Lindeman <billylindeman@gmail.com>");
MODULE_DESCRIPTION("bbq10kbd i2c keyboard driver");
MODULE_VERSION("0.1");

#define BBQ10KBD_NS_PER_MS 1000000L
#define BBQ10KBD_GESTURE_TIMEOUT (400 * BBQ10KBD_NS_PER_MS)
#define BBQ10KBD_GESTURE_HORIZ_START_THRESH 30
#define BBQ10KBD_GESTURE_HORIZ_STEP 15
#define BBQ10KBD_GESTURE_VERT_START_THRESH 70
#define BBQ10KBD_GESTURE_VERT_STEP 30

typedef enum bbq10kbd_gesture_dir {
  BBQ10KBD_GESTURE_NONE = 0,
  BBQ10KBD_GESTURE_HORIZ,
  BBQ10KBD_GESTURE_VERT
} bbq10kbd_gesture_dir;

struct bbq10kbd_keypad {
  struct i2c_client *i2c;
  struct input_dev *input_keyboard;
  uint64_t gesture_last_ns;
  int gesture_acc_x;
  int gesture_acc_y;
  bbq10kbd_gesture_dir gesture_dir;
};

static struct of_device_id bbq10kbd_ids[] = {
    {.compatible = DEVICE_NAME},
    {}
};

static const struct i2c_device_id bbq10kbd_id[] = { 
    {DEVICE_NAME, 0}, 
    {}
};
MODULE_DEVICE_TABLE(i2c, bbq10kbd_id);



