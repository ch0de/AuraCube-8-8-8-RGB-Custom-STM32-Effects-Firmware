#include "sys.h"
#include <stdint.h>

void WFI_SET(void)      { __ASM volatile ("wfi");     }
void INTX_DISABLE(void) { __ASM volatile ("cpsid i"); }
void INTX_ENABLE(void)  { __ASM volatile ("cpsie i"); }

/* Set Main Stack Pointer (MSP) to 'addr' */
void MSR_MSP(uint32_t addr)
{
    __ASM volatile ("msr msp, %0" :: "r"(addr) : );
}
