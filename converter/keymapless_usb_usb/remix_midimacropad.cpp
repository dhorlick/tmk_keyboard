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

#include "usbh_midi.h"
#include "host.h"


USB usb_host;
USBH_MIDI Midi(&usb_host);
report_keyboard_t output_keyboard_report = {};


void remix_setup(void)
{
    debug_enable = true;
    int usb_host_init_result;
    usb_host_init_result = usb_host.Init();
    dprintf("usb_host.Init() result: %d\n", usb_host_init_result);
}

/**
 * Polls hosted MIDI controller, maps to keyboard keycodes, then transmits.
 */
uint8_t remix_loop(void)
{
    uint8_t content[ 3 ];
    uint8_t size;
    static uint8_t last_note = 0u;
    
    do
    {
        if ( (size = Midi.RecvData(content)) > 0 )
        {
            dprintf("got %d content bytes", size);
            // is first byte a Note On message with a MIDI channel?
            uint8_t note_on_command_midi_channel = 1 + content[0] - 0x90;
            dprintf("note_on_command_midi_channel = %u", note_on_command_midi_channel);
            if (note_on_command_midi_channel >= 1 && note_on_command_midi_channel <= 15)
            {
                // second byte is the MIDI note
                uint8_t next_keypress = KC_NO;
                if (last_note != content[1])
                {
                    dprintf("MIDI note = %d\n", content[1]);
                    switch (content[1])
                    {
                        case 53: // F3
                            next_keypress = KC_F1;
                            break;
                        case 54: // F#3
                            next_keypress = KC_MAIL;
                            break;
                        case 55: // G3
                            next_keypress = KC_F2;
                            break;
                        case 56: // G#3
                            next_keypress = KC_CALC;
                            break;
                        case 57: // A3
                            next_keypress = KC_F3;
                            break;
                        case 58: // A#3
                            next_keypress = KC_MY_COMPUTER;
                            break;
                        case 59: // B3
                            next_keypress = KC_F4;
                            break;
                        case 60: // C4
                            next_keypress = KC_F5;
                            break;
                        case 61: // C#4
                            next_keypress = KC_WWW_SEARCH;
                            break;
                        case 62: // D4
                            next_keypress = KC_F6;
                            break;
                        case 63: // D#4
                            next_keypress = KC_WWW_HOME;
                            break;
                        case 64: // E4
                            next_keypress = KC_F7;
                            break;
                        case 65: // F4
                            next_keypress = KC_F8;
                            break;
                        case 66: // F#4
                            next_keypress = KC_WWW_BACK;
                            break;
                        case 67: // G4
                            next_keypress = KC_F9;
                            break;
                        case 68: // G#4
                            next_keypress = KC_WWW_FORWARD;
                            break;
                        case 69: // A4
                            next_keypress = KC_F10;
                            break;
                        case 70: // A#4
                            next_keypress = KC_WWW_STOP;
                            break;
                        case 71: // B4
                            next_keypress = KC_F11;
                            break;
                        case 72: // C5
                            next_keypress = KC_F12;
                            break;
                        case 73: // C#5
                            next_keypress = KC_WWW_REFRESH;
                            break;
                        case 74: // D5
                            next_keypress = KC_F13;
                            break;
                        case 75: // D#5
                            next_keypress = KC_WWW_FAVORITES;
                            break;
                        case 76: // E5
                            next_keypress = KC_F14;
                            break;
                        case 77: // F5
                            next_keypress = KC_F15;
                            break;
                        case 78: // F#5
                            next_keypress = KC_EXECUTE;
                            break;
                        case 79: // G5
                            next_keypress = KC_F16;
                            break;
                        case 80: // G#5
                            next_keypress = KC_HELP;
                            break;
                        case 81: // A5
                            next_keypress = KC_F17;
                            break;
                        case 82: // A#5
                            next_keypress = KC_FIND;
                            break;
                        case 83: // B5
                            next_keypress = KC_F18;
                            break;
                        case 84: // C6
                            next_keypress = KC_F19;
                            break;
                        default:
                            dprintf("Unmapped note./n");
                    }
                    
                    if (next_keypress != KC_NO)
                    {
                        output_keyboard_report.keys[0] = next_keypress;
                        host_keyboard_send(&output_keyboard_report);
                        output_keyboard_report.keys[0] = KC_NO;
                        host_keyboard_send(&output_keyboard_report);
                        last_note = content[1];
                    }
                }
            }
        }
        else
            last_note = 0u;
    } while (size > 0);

    usb_host.Task();
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
