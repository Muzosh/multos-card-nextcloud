/**
 *
 *  Copyright (C) 2020  Raul Casanova Marques
 *
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma attribute("aid", "f0 00 00 01")
#pragma attribute("dir", "61 0d 4f 04 f0 00 00 01 50 05 68 77 61 70 70")

// ISO codes
#include <ISO7816.h>
// SmartDeck comms support
#include <multoscomms.h>
// SmartDeck crypto support
#include <multoscrypto.h>

// Standard libraries
#include <stdint.h>
#include <string.h>

// ECC support
#include "ecc/multosecc.h"

#include "helpers/mem_helper.h"
#include "helpers/random_helper.h"

#include "models/user.h"

#include "apdu.h"
#include "types.h"

/// Global values - RAM (Public memory)
#pragma melpublic
uint8_t apdu_data[APDU_L_MAX];

/// Session values - RAM (Dynamic memory)
#pragma melsession
// user identifier and attributes
user_identifier_t user_identifier = {
        0x02, 0x0F, 0x84, 0x31, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0x1A, 0x2B, 0x3C, 0x4D,
        0x5E, 0x6F, 0x00, 0x40, 0x00, 0xFF, 0x01
};

/// Static values - EEPROM (Static memory)
#pragma melstatic
uint8_t elliptic_curve_base_point_affine = {0x0F};
elliptic_curve_domain_t elliptic_curve_domain = {
        0x00, // Format of domain params
        0x20, // Prime length in bytes
        0x25, 0x23, 0x64, 0x82, 0x40, 0x00, 0x00, 0x01, 0xba, 0x34, 0x4d, 0x80,
        0x00, 0x00, 0x00, 0x08, 0x61, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13,
        0xa7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, // p
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // a
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, // b
        0x25, 0x23, 0x64, 0x82, 0x40, 0x00, 0x00, 0x01, 0xBA, 0x34, 0x4D, 0x80,
        0x00, 0x00, 0x00, 0x08, 0x61, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13,
        0xA7, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, //Gx
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, //Gy
        0x25, 0x23, 0x64, 0x82, 0x40, 0x00, 0x00, 0x01, 0xba, 0x34, 0x4d, 0x80,
        0x00, 0x00, 0x00, 0x07, 0xff, 0x9f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x10,
        0xa1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, // N
        0x01 // H
}; // mcl 256bit

void main(void)
{
    if (CLA != CLA_APPLICATION)
    {
        ExitSW(ISO7816_SW_CLA_NOT_SUPPORTED);
    }

    switch (INS)
    {
        case INS_HELLO_WORLD:
        {
            if (!CheckCase(2))
            {
                ExitSW(ISO7816_SW_CLA_NOT_SUPPORTED);
                break;
            }

            // copy user identifier
            memcpy(apdu_data, (const void *) &user_identifier, USER_MAX_ID_LENGTH);

            ExitSWLa(ISO7816_SW_NO_ERROR, USER_MAX_ID_LENGTH);
            break;
        }
        default:
        {
            ExitSW(ISO7816_SW_INS_NOT_SUPPORTED);
            break;
        }
    }
}

/**
 * Writes random bytes to address by cryptographically secure
 * pseudo random number generator.
 *
 * @param address pointer to where the random number will be written
 */
void set_by_csprng(unsigned char *address)
{
    __set_by_csprng(address);
    __modular_reduction(address, EC_SIZE, (uint8_t *) &elliptic_curve_domain.N, EC_SIZE);
}
