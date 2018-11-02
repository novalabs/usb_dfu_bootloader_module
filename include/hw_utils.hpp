/* COPYRIGHT (c) 2016-2018 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#pragma once

#include <stdint.h>

// The following overrides the watchdog - useful only during debugging!
//#define OVERRIDE_WATCHDOG 1

namespace hw {
typedef enum {
    HARDWARE, WATCHDOG, SOFTWARE, OTHER
} ResetSource;

ResetSource
getResetSource();

void
reset();

uint32_t
getNVR();

void
setNVR(
    uint32_t value
);


class Watchdog
{
public:
    static void
    freezeOnDebug();


    enum Reason : uint32_t {
        NO_APPLICATION   = 0xCAFEBABE,
        USER_REQUEST     = 0xB0BAFE77,
        BOOT_APPLICATION = 0xBAADF00D,
        TRANSPORT_FAIL   = 0xACABACAB
    };

    enum Period {
        _0_ms,
        _800_ms,
        _1600_ms,
        _3200_ms,
        _6400_ms
    };

    static void
    enable(
        Period period
    );

    static void
    reload();
};

typedef void (* pFunction)(
    void
);

int32_t
jumptoapp(
    uint32_t addr
);
}
