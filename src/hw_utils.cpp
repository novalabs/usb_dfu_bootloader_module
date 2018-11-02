/* COPYRIGHT (c) 2016-2018 Nova Labs SRL
 *
 * All rights reserved. All use of this software and documentation is
 * subject to the License Agreement located in the file LICENSE.
 */

#include <hw_utils.hpp>

#include <nil.h>
#include <hal.h>

#ifdef OVERRIDE_WATCHDOG
#warning "OVERRIDE_WATCHDOG is DEFINED - DO NOT USE THIS IN PRODUCTION"
#endif

#if defined(STM32F091xC)
uint32_t VectorTable[48] __attribute__((section(".vector_table")));
#endif

namespace hw {
ResetSource
getResetSource()
{
    static const uint32_t CSR = RCC->CSR; // save it for later...

    if ((CSR & RCC_CSR_IWDGRSTF) == RCC_CSR_IWDGRSTF) {
        RCC->CSR |= RCC_CSR_RMVF;
        return ResetSource::WATCHDOG;
    } else if ((CSR & RCC_CSR_SFTRSTF) == RCC_CSR_SFTRSTF) {
        RCC->CSR |= RCC_CSR_RMVF;
        return ResetSource::SOFTWARE;
    } else {
        RCC->CSR |= RCC_CSR_RMVF;
        return ResetSource::HARDWARE;
    }
}

void
reset()
{
    chThdSleepMilliseconds(100);

    NVIC_SystemReset();

    while (1) {}
} // reset

uint32_t
getNVR()
{
    return RTC->BKP0R;
}

void
setNVR(
    uint32_t value
)
{
    RTC->BKP0R = value;
}

void
Watchdog::freezeOnDebug()
{
    DBGMCU->APB1FZ |= (DBGMCU_APB1_FZ_DBG_WWDG_STOP);
}

void
Watchdog::enable(
    Period period
)
{
#ifndef OVERRIDE_WATCHDOG
    switch (period) {
      case Period::_0_ms:
          IWDG->KR  = 0x5555; // enable access
          IWDG->PR  = 1;      // /8
          IWDG->RLR = 0x1;    // maximum (circa 0 ms)
          IWDG->KR  = 0xCCCC; // start watchdog
          break;
      case Period::_800_ms:
          IWDG->KR  = 0x5555; // enable access
          IWDG->PR  = 1;      // /8
          IWDG->RLR = 0xFFF;  // maximum (circa 800 ms)
          IWDG->KR  = 0xCCCC; // start watchdog
          break;
      case Period::_1600_ms:
          IWDG->KR  = 0x5555; // enable access
          IWDG->PR  = 2;      // /16
          IWDG->RLR = 0xFFF;  // maximum (circa 1600 ms)
          IWDG->KR  = 0xCCCC; // start watchdog
          break;
      case Period::_3200_ms:
          IWDG->KR  = 0x5555; // enable access
          IWDG->PR  = 3;      // /32
          IWDG->RLR = 0xFFF;  // maximum (circa 3200 ms)
          IWDG->KR  = 0xCCCC; // start watchdog
          break;
      default:
          IWDG->KR  = 0x5555; // enable access
          IWDG->PR  = 4;      // /64
          IWDG->RLR = 0xFFF;  // maximum (circa 6400 ms)
          IWDG->KR  = 0xCCCC; // start watchdog
          break;
    } // switch
#endif // ifndef OVERRIDE_WATCHDOG
} // watchdogEnable

void
Watchdog::reload()
{
#ifndef OVERRIDE_WATCHDOG
    IWDG->KR = 0xAAAA; // reload
#endif
}

int32_t
jumptoapp(
    uint32_t addr
)
{
#if defined(STM32F091xC)
    // Copy vector table from image to RAM
    for (std::size_t i = 0; i < 48; i++) {
        VectorTable[i] = *(uint32_t*)(addr + (i << 2));
    }

    // Enable the SYSCFG peripheral clock
    rccEnableAPB2(RCC_APB2ENR_SYSCFGCOMPEN, FALSE);
    // Remap SRAM at 0x00000000
    SYSCFG->CFGR1 &= ~SYSCFG_CFGR1_MEM_MODE;
    SYSCFG->CFGR1 |= SYSCFG_CFGR1_MEM_MODE;
#endif

    pFunction JumpToApp;
    uint32_t  JumpAddress;

    // The second entry of the vector table contains the reset_handler function
    JumpAddress = *(uint32_t*)(addr + 4);

    // Assign the function pointer
    JumpToApp = (pFunction)JumpAddress;

    // Initialize user application's Stack Pointer
    __set_MSP(*(uint32_t*)addr);

    chSysDisable();

    // Jump!
    JumpToApp();

    return 0;
} // jumptoapp
}
