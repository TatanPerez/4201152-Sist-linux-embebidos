#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*
 * Some FreeRTOS ports expect vPortCleanUpTCB to be provided by the port.
 * If it's missing in the current port implementation, provide a no-op
 * stub here to satisfy the linker.
 */
void vPortCleanUpTCB(void *pxTCB)
{
    (void) pxTCB;
}
