#ifndef SERVO_H
#define SERVO_H

#include "stm32f10x.h"

void servo_init(uint32_t baudrate);
void servo_forward_frame(uint8_t *frame, uint8_t len);

#endif
