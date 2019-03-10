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

#include "unused.h"
#include "matrix.h"


const uint8_t keymaps[][MATRIX_ROWS][MATRIX_COLS] PROGMEM = {
};

const action_t fn_actions[] PROGMEM = {};



#if 0
    KEYMAP( )

const action_t PROGMEM fn_actions[] = {
};

#endif

static bool matrix_is_mod = false;

bool matrix_is_modified(void) {
    return matrix_is_mod;
}

bool matrix_is_on(uint8_t row, uint8_t col) {
    return false;
}

matrix_row_t matrix_get_row(uint8_t row) {
    return 0;
}

void led_set(uint8_t usb_led)
{
}

uint8_t matrix_rows(void) { return MATRIX_ROWS; }
uint8_t matrix_cols(void) { return MATRIX_COLS; }
bool matrix_has_ghost(void) { return false; }
