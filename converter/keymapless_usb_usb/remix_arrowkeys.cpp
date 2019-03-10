/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdint.h>
#include <stdbool.h>

#include "Usb.h"
#include "usbhid.h"
#include "hidboot.h"
#include "parser.h"

#include "remix.h"

#include "keycode.h"
#include "util.h"
#include "print.h"
#include "debug.h"
#include "timer.h"
#include "host.h"


USB usb_host;
HIDBoot<USB_HID_PROTOCOL_MOUSE> mse1(&usb_host);
MSEReportParser mse_parser1;
report_keyboard_t output_keyboard_report = {};


void remix_setup(void)
{
    debug_enable = true;
    int usb_host_init_result;
    usb_host_init_result = usb_host.Init();
    dprintf("usb_host.Init() result: %d", usb_host_init_result);
    mse1.SetReportParser(0, (HIDReportParser*)&mse_parser1);
    for (int i=0; i<KEYBOARD_REPORT_KEYS; i++) 
    {
        output_keyboard_report.keys[i] = KC_NO;
    }
}

/**
 * Polls input mouse-like device, maps position to arrow keycodes, and transmits.
 */
uint8_t remix_loop(void)
{
    static uint16_t last_time_stamp_mse_1 = 0;
    const int MARGIN = 3;
    
    if (mse_parser1.time_stamp != last_time_stamp_mse_1)
    {
        int i = 0;
        if (mse_parser1.report.y > MARGIN) 
        {
            output_keyboard_report.keys[i++] = KC_DOWN;
        }
        else if (mse_parser1.report.y < -MARGIN) 
        {
            output_keyboard_report.keys[i++] = KC_UP;
        }
        if (mse_parser1.report.x < -MARGIN)
        {
            output_keyboard_report.keys[i++] = KC_LEFT;
        }
        else if (mse_parser1.report.x > MARGIN) 
        {
            output_keyboard_report.keys[i++] = KC_RIGHT;
        }
        if (mse_parser1.report.buttons == MOUSE_BTN1) 
        {
            output_keyboard_report.keys[i++] = KC_SPACE;
        }
            
        host_keyboard_send(&output_keyboard_report);
        
        for (int j=i; j>=0; j--)
        {
            output_keyboard_report.keys[j] = KC_NO;
        }
        host_keyboard_send(&output_keyboard_report);
    
        last_time_stamp_mse_1 = mse_parser1.time_stamp;
    }

    uint16_t timer;
    timer = timer_read();
    usb_host.Task();
    timer = timer_elapsed(timer);
    if (timer > 100) {
        dprintf("host.Task: %d\n", timer);
    }
    return 1;
}
