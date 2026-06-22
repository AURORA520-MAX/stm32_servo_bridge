#include "stm32f10x_it.h"

void NMI_Handler(void)         { while (1); }
void HardFault_Handler(void)   { while (1); }
void MemManage_Handler(void)   { while (1); }
void BusFault_Handler(void)    { while (1); }
void UsageFault_Handler(void)  { while (1); }
void SVC_Handler(void)         {}
void DebugMon_Handler(void)    {}
void PendSV_Handler(void)      {}

void SysTick_Handler(void)
{
    // delay 库使用 SysTick，这里留空，delay_init 会自己处理
}
