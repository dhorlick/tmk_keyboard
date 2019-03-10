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
bool attempt_report_protocol_enable = true;
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
        dprintf("initialization failed.\n");
}

/**
 * Transmits button state, pointer positioning and (potentially) scroll state of the 
 * scanned mouse-like device.
 */
uint8_t remix_loop(void) {
    static uint16_t last_time_stamp_mse_1 = 0;
    
    if (mse_parser.time_stamp != last_time_stamp_mse_1)
    {
        host_mouse_send(&mse_parser.report);
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
