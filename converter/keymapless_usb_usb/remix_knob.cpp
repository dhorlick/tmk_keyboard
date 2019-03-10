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

// USB HID host
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
#include "matrix.h"
#include "led.h"
#include "host.h"
#include "keyboard.h"



USB usb_host;
report_keyboard_t output_keyboard_report = {};
bool attempt_report_protocol_enable = true;
    // Check MSEReportParser::Parse in shared.cpp to make sure your input device is
    // custom-supported for scrolling.
HIDBoot<USB_HID_PROTOCOL_MOUSE> mse1(&usb_host, attempt_report_protocol_enable);
MSEReportParser mse_parser;


void remix_setup(void)
{
    debug_enable = true;
    int usb_host_init_result;
    usb_host_init_result = usb_host.Init();
    dprintf("usb_host.Init() result: %d\n", usb_host_init_result);
    if (!usb_host_init_result)
    {
        usb_host.RegisterDeviceClass((USBDeviceConfig*) &mse_parser);
        mse_parser.use_custom_mappings = attempt_report_protocol_enable;
        mse1.SetReportParser(0, (HIDReportParser*)&mse_parser);
    }
    else
        dprintf("initialization failed; %d", usb_host_init_result);
}

/**
 * Polls hosted mouse-like device, attempts to map scroll to volume up and volume down
 * keycodes, and transmits if able.
 */
uint8_t remix_loop(void)
{
    static uint16_t last_time_stamp_mse_1 = 0;
    
    const int MARGIN = 1;
    
    if (mse_parser.time_stamp != last_time_stamp_mse_1)
    {
        dprintf("mse_parser.report.v = %d\n", mse_parser.report.v);
        switch (mse_parser.report.buttons) 
        {
            case MOUSE_BTN1:
                output_keyboard_report.keys[0] = KC__MUTE;
                break;
            case MOUSE_BTN2:
                output_keyboard_report.keys[0] = KC__MUTE;
                break;
            default:
                if (mse_parser.report.v >= MARGIN) 
                {
                    dprintf("Turning volume up!\n");
                    output_keyboard_report.keys[0] = KC__VOLUP;
                }
                else if (mse_parser.report.v <= -MARGIN) 
                {
                    dprintf("Turning volume down!\n");
                    output_keyboard_report.keys[0] = KC__VOLDOWN;                   
                }
        }
        
        if (output_keyboard_report.keys[0] != KC_NO) 
        {
            dprintf("Issuing keypress to host...\n");
            host_keyboard_send(&output_keyboard_report);
            output_keyboard_report.keys[0] = KC_NO;
            host_keyboard_send(&output_keyboard_report);
        }
        else
        {
            dprintf("NOT Issuing keypress to host\n");
        }
        
        last_time_stamp_mse_1 = mse_parser.time_stamp;
    }

    uint16_t timer;
    timer = timer_read();
    usb_host.Task();
    timer = timer_elapsed(timer);
    if (timer > 100)
    {
        dprintf("host.Task: %d\n", timer);
    }
    return 1;
}
