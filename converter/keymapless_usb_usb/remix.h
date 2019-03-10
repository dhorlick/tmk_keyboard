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

#include "Arduino.h"
#include "hidboot.h"

#include "report.h"

extern "C"
{
    void remix_setup(void);
    
    /**
     * Remix inputs from host device in interesting ways and transmits.
     */
    uint8_t remix_loop(void);
}

/**
 * Parses input from hosted mouse-like devices and canonicalizes to tmk_keyboard's
 * report_mouse_t (not usb_host_shield_2.0's MOUSEINFO).
 * 
 * If use_custom_mappings is false, parses host mouse input buffers by copying them
 * more-or-less directly to report_mouse_t.
 * 
 * This works OK for most mice-like devices, but typically requires giving up scroll
 * support.
 * 
 * If use_custom_mappings is true, attempts to determine the device and vendor, then
 * parse inputs according by custom mini-drivers. Not many devices are supported at this
 * time and scrolling has been prioritized over positioning.
 * 
 * For custom mappings to work, register with the USB host infrastructure by invoking
 * USB.RegisterDeviceClass(this) from usb_host_shield_2.0.
 *
 * Since report_mouse_t uses 8-bit integers, one limitation is that extra fine detail
 * from 16-bit mice must be discarded.
 *
 * Not to be confused with usb_host_shield_2.0's "MouseReportParser". 
 */
class MSEReportParser : public HIDReportParser, USBDeviceConfig
{
    public:
        report_mouse_t report;
        uint16_t time_stamp;
        
        /**
         * If false, assumes a standardized Boot Protocol and copies input buffer to
         * report_mouse_t with no or minimal modification.
         * 
         * Use true for Report Protocol or an idiosyncratic Boot Protocol.
         * 
         * If this flag is incorrect, expect misbehavior.
         * 
         * When in doubt, keep the default false value.
         */
        bool use_custom_mappings;
        
        /**
         * Not recommended.
         * 
         * Most mouse boot protocols don't (and indeed may not be allowed to) provide usable scroll
         * v and h inputs.
         * 
         * Therefore, it is generally NOT safe to relay the output report_mouse_t to host_mouse_send()
         * without zeroing-out scroll v and h first! We do this by default, but this flag lets to 
         * opt to relay them, instead.
         */
        bool relay_scroll_inputs_with_standard_mappings;
        
        uint16_t vendor_id;

        /**
         Also known as Device ID
         */
        uint16_t product_id;
        
        uint8_t device_class;
        
        uint8_t device_subclass;

        virtual void print_mouse_input(USBHID *hid,  bool is_rpt_id, uint8_t len, uint8_t *buf);
        virtual void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);

        /**
         * This is a hack to get at & record vendor_id and product_id
         */
        virtual bool VIDPIDOK(uint16_t vid, uint16_t pid)
        {
            vendor_id = vid;
            product_id = pid;
            return false;
        };
        
        virtual bool DEVCLASSOK(uint8_t klass)
        {
            device_class = klass;
            return false;
        }
        
        virtual bool DEVSUBCLASSOK(uint8_t subklass)
        {
            device_subclass = subklass;
            return true;
        };
        
        virtual void print_binary(uint8_t value);
        virtual void print_more_mouse_input(uint8_t len, uint8_t *buf);
};
