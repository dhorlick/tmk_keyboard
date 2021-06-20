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

#include "remix.h"
#include "Arduino.h"
#include "matrix.h"

#include "usbhid.h"
#include "report.h"
#include "debug.h"

/**
 * Doesn't really initialize a matrix here.
 */
void matrix_init(void) 
{
    remix_setup();
}

/**
 * Doesn't really scan a matrix here.
 */
uint8_t matrix_scan(void) 
{
    return remix_loop();
}

void MSEReportParser::print_mouse_input(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
{
    dprintf("mouse input %d:", hid->GetAddress());
    for (uint8_t i = 0; i < len; i++) 
        dprintf(" %02X", buf[i]);
    if (is_rpt_id) 
        dprintf(" is_rpt_id!");  // (whatever that means)
    dprint("\n");
}

void MSEReportParser::print_binary(uint8_t value)
{
    for (int i=7; i>=0; i--)
    {
        uint8_t shifted = (value >> i) & 1;
        dprintf("%u", shifted);
    }
}

void MSEReportParser::print_more_mouse_input(uint8_t len, uint8_t *buf)
{
    for (uint8_t i = 0; i < len; i++) 
    {
        print_binary(buf[i]);
        if (i < len)
            dprint(" ");
    }
    dprint("\n");    
}

void MSEReportParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf)
{
    print_mouse_input(hid, is_rpt_id, len, buf);
    time_stamp = millis();
    
    if (!use_custom_mappings)
    {
        ::memcpy(&report, buf, sizeof(report_mouse_t));
        if (!relay_scroll_inputs_with_standard_mappings)
        {
            report.v = 0;
            report.h = 0;
        }
    }
    else
    {
        switch (vendor_id)
        {
            case 0x045e: // Microsoft
                switch (product_id) 
                {
                    case 0x0823: // Microsoft Classic Intellimouse (2018)
                        report.buttons = buf[1];
                        report.x = (int8_t) buf[2];
                        report.y = (int8_t) buf[4];
                        report.v = (int8_t) buf[6];
                        report.h = 0;
                        return;
                }
                break;
                
            case 0x0F62: // Targus
                switch (product_id)
                {
                    case 0x1001: // Dual-Mode Mini Trackball Optical Mouse
                        report.buttons = buf[0];
                        report.x = buf[1];
                        report.y = buf[2];
                        switch (buf[3])
                        {
                            case 0xFF: // scroll down
                                report.v = 0xFF;
                                report.h = 0;
                                break;
                            case 0x07: // scroll left
                                report.v = 0;
                                report.h = 0xFF;
                                break;
                            case 0xF9: // scroll right
                                report.v = 0;
                                report.h = 0x01;
                                break;
                            case 0x01: // scroll up
                                report.v = 0x01;
                                report.h = 0;
                                break;
                        }
                        return;
                }
                
                break;                
            
            // remaining drivers support scroll and buttons, only
                
            case 0x1BCF: // Kensington
                switch (product_id)
                {
                    case 0x0002: // Pro Fit Full-Size Mouse
                        report.buttons = buf[0];
                        report.x = 0; // TODO ?
                        report.y = 0; // TODO ?
                        report.v = (int8_t) buf[4];
                        report.h = 0;
                        return;
                }
                break;
            
            case 0x413C: // Dell
                switch (product_id)
                {
                    case 0x3012: // Optical Wheel Mouse
                        report.buttons = buf[0];
                        report.x = 0; // TODO ?
                        report.y = 0 ; // TODO ?
                        report.v = (int8_t) buf[3];
                        report.h = 0;
                        return;
                }
                break;
                
            // TODO support other vendors
        }
        
        dprintf("No report protocol support for input device: vendor ID = %04X, product ID = %04X\n", vendor_id, product_id);
        
        report.buttons = 0;
        report.x = 0;
        report.y = 0;
        report.v = 0;
        report.h = 0;
        
        // Help implementors of new custom device mappings
        dprintf("--> ");
        print_more_mouse_input(len, buf);
    }
}
