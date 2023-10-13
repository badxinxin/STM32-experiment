#ifndef USART_H
#define USART_H

#include "stm32f10x.h"

void usart_init(uint32_t baud_rate);
void USART1_IRQHandler(void);
void ProcessAndSendData(void);

#endif  // USART_H
