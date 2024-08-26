/* #####################################################################################################################

               """          main.cpp
        -\-    _|__
         |\___/  . \        Created on 26 Aug. 2024 at 20:33
         \     /(((/        by hmelica
          \___/)))/         hmelica@student.42.fr

#####################################################################################################################
*/

#include "bsp/board.h"
#include "class/hid/hid.h" // HID_KEY_*
#include "class/hid/hid_device.h"
#include "codes.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define OUT_PIN 25

//
enum {
    BLINK_NOT_MOUNTED = 250,  // device not mounted
    BLINK_MOUNTED     = 1000, // device mounted
    BLINK_SUSPENDED   = 2500, // device is suspended
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

static bool stop = false;

int main() {
    stdio_init_all(); // init stdio with uart and stuff
    printf("hello\n");
    board_init();
    tusb_init();
    gpio_init(OUT_PIN);
    gpio_set_dir(OUT_PIN, GPIO_OUT);
    stop = false;
    while (!stop) {
        tud_task();
        led_blinking_task();
        hid_task();
    }
    return 0;
}

static void send_hid_report(uint16_t digit, bool nullreport) {
    // skip if hid is not ready yet
    uint8_t keycodes[6] = { 0 };
    uint8_t keycode;
    switch (digit) {
        case 0:
            keycode = HID_KEY_0;
            break;
        case 1:
            keycode = HID_KEY_1;
            break;
        case 2:
            keycode = HID_KEY_2;
            break;
        case 3:
            keycode = HID_KEY_3;
            break;
        case 4:
            keycode = HID_KEY_4;
            break;
        case 5:
            keycode = HID_KEY_5;
            break;
        case 6:
            keycode = HID_KEY_6;
            break;
        case 7:
            keycode = HID_KEY_7;
            break;
        case 8:
            keycode = HID_KEY_8;
            break;
        case 9:
            keycode = HID_KEY_9;
            break;
        case 10:
            keycode = HID_KEY_SPACE;
            break;
        default:
            printf("Number not in switch\n");
            nullreport = true;
    }
    keycodes[0] = keycode;
    // if (!tud_hid_ready()) {
    //     printf("HID not ready yet\n");
    //     return;
    // }

    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycodes);
    sleep_ms(50);
    tud_task();
    sleep_ms(50);
    // printf("n");
    for (uint8_t i = 0; i < 6; i++)
        keycodes[i] = 0x00;
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycodes);
    sleep_ms(50);
    tud_task();
    // tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
}

bool write_code(uint16_t code) {
    uint16_t current = 1000;
    uint16_t to_send;
    if (!tud_hid_ready()) {
        printf("HID not ready yet\n");
        return false;
    }
    printf("Sending... ");
    sleep_ms(3000);
    send_hid_report(10, false); // press space to enter digits;
    sleep_ms(3000);
    send_hid_report(10, false); // press space to enter digits;
    for (uint8_t i = 0; i < 4; i++) {
        if (current == 0)
            to_send = code;
        else
            to_send = code / current;
        code    %= current;
        current /= 10;
        printf("%u ", to_send);
        send_hid_report(to_send, false);
        sleep_ms(100);
    }
    printf("... sent.\n");
    return true;
}

// static const uint16_t codes[] = {
//     1234, 1235, 1233, 1232, 1231, 1230,
// };

// Every 10ms, we poll the pins and send a report
void hid_task(void) {
    static uint16_t index = 0;
    static uint32_t time  = (16 * 600) + 50;
    static bool first_time = true;
    // Poll every 10ms

    if (!tud_hid_ready()) {
        printf("Waiting for device to be ready\n");
        sleep_ms(100);
        return;
    }
    sleep_ms(100);
    if (first_time) {
        first_time = false;
        write_code(codes[index]);
    }
    time += 1; // time is in s
    if (time % 600 == 0)
        printf("ping! Next code : %04u (%u)\n", codes[index], index);
    if (time >= (16 * 600) + 50) {
        // if (time >= 50) {
        gpio_put(OUT_PIN, 1);
        // Remote wakeup
        if (tud_suspended()) {
            // Wake up host if we are in suspend mode
            // and REMOTE_WAKEUP feature is enabled by host
            tud_remote_wakeup();
        }
        if (!write_code(codes[index])) {
            printf("Could not write\n");
            return;
        }
        time = 0;
        index++;
        if (index >= 1000)
            stop = true;
        gpio_put(OUT_PIN, 0);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, const uint8_t *report, uint16_t len) {
    // not implemented, we only send REPORT_ID_KEYBOARD
    (void) instance;
    (void) len;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(
    uint8_t           instance,
    uint8_t           report_id,
    hid_report_type_t report_type,
    uint8_t          *buffer,
    uint16_t          reqlen
) {
    // TODO not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(
    uint8_t           instance,
    uint8_t           report_id,
    hid_report_type_t report_type,
    const uint8_t    *buffer,
    uint16_t          bufsize
) {
    (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD) {
            // bufsize should be (at least) 1
            if (bufsize < 1)
                return;

            const uint8_t kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
                // Capslock On: disable blink, turn led on
                blink_interval_ms = 0;
                board_led_write(true);
            } else {
                // Caplocks Off: back to normal blink
                board_led_write(false);
                blink_interval_ms = BLINK_MOUNTED;
            }
        }
    }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
    blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
    static uint32_t start_ms  = 0;
    static bool     led_state = false;

    // blink is disabled
    if (!blink_interval_ms)
        return;

    // Blink every interval ms
    if (board_millis() - start_ms < blink_interval_ms)
        return; // not enough time
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}
