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

#include <math.h>


USB usb_host;
bool attempt_report_protocol_enable = false;
HIDBoot<USB_HID_PROTOCOL_MOUSE> mse1(&usb_host, attempt_report_protocol_enable);
MSEReportParser mse_parser;

const uint8_t BREATH_MAGNITUDE = 10;
const uint8_t BREATH_PERIOD_IN_TICKS = 100;
#define PI 3.14159265


void remix_setup(void) 
{
    debug_enable = true;
    int usb_host_init_result;
    usb_host_init_result = usb_host.Init();
    dprintf("usb_host.Init() result: %d", usb_host_init_result);
    if (!usb_host_init_result)
    {
        usb_host.RegisterDeviceClass((USBDeviceConfig*) &mse_parser);
        mse_parser.use_custom_mappings = attempt_report_protocol_enable;
        mse1.SetReportParser(0, (HIDReportParser*)&mse_parser);
    }
    else
        dprintf("Initialization failed.\n");
}

/**
 * Transmits button state, pointer positioning and (potentially) scroll state of the 
 * scanned mouse-like device.
 * 
 * After a period of inactivity applies animation to the pointer so it appears to be
 * asleep. New inputs back-out that animation and prompt resumption of the initial
 * pass-thru state.
 */
uint8_t remix_loop(void)
{
    static uint16_t last_time_stamp_mse_1 = 0; // millis();
    static uint16_t animation_ticks;

    static bool mouse_is_asleep = false;
    static int8_t breathing_ticks = 0;
    static int8_t previous_y = 0;
    static uint8_t note_index = 0;
    
    animation_ticks++;
    
    if (mse_parser.time_stamp != last_time_stamp_mse_1)
    {
        host_mouse_send(&mse_parser.report);
        last_time_stamp_mse_1 = mse_parser.time_stamp;
        if (mouse_is_asleep)
        {
            mouse_is_asleep = false;
            dprintf("Mouse wakes up!\n");
            animation_ticks = 0;
            // reset to waking position
            mse_parser.report.y = previous_y;
            mse_parser.report.buttons = 0;
            mse_parser.report.v = 0;
            mse_parser.report.h = 0;
            host_mouse_send(&mse_parser.report);
        }
    }
    else if (!mouse_is_asleep && animation_ticks > 60000)
    {
        mouse_is_asleep = true;
        dprintf("Mouse falls asleep!\n");
        breathing_ticks = 0;
        animation_ticks = 0;
        previous_y = 0;
    }
    
    if (mouse_is_asleep && animation_ticks > 180)
    {
        breathing_ticks++;
        
        if (breathing_ticks >= BREATH_PERIOD_IN_TICKS)
            breathing_ticks = 0;
        
        animation_ticks = 0;
        mse_parser.report.x = 0;
        
        float breath_fraction = (float)breathing_ticks / (float)BREATH_PERIOD_IN_TICKS;
        int8_t y = BREATH_MAGNITUDE * sin(2.0 * PI * breath_fraction);
        int8_t delta_y = y - previous_y;
        previous_y = y;
        
        mse_parser.report.y = -delta_y;
        mse_parser.report.buttons = 0;
        mse_parser.report.v = 0;
        mse_parser.report.h = 0;
        dprintf("breathing_ticks = %d\n", breathing_ticks);
        host_mouse_send(&mse_parser.report);
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