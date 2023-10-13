#include "usart.h"
#include <string.h> 

void usart_init(uint32_t baud_rate) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);  // Enable USART1 and GPIOA clocks

    // Configure USART1 Tx (PA9) as alternate function push-pull
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure USART1 Rx (PA10) as input floating
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = baud_rate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  // Enable the USART1 Receive interrupt

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

#define HEADER_BYTE 0xAA
#define NEW_HEADER "\xBB\xBB\xBB"
#define HEADER_SIZE 3

uint8_t rxBuffer[64 + HEADER_SIZE];
uint8_t rxDataSize = 0;
uint8_t headerCount = 0;
enum { WAIT_FOR_HEADER, RECEIVING_DATA } state = WAIT_FOR_HEADER;

void USART1_IRQHandler(void) {
    // Keep processing bytes as long as data is available
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET) {
        uint8_t data = USART_ReceiveData(USART1);
        switch (state) {
            case WAIT_FOR_HEADER:
                if (data == HEADER_BYTE) {
                    headerCount++;
                    if (headerCount == HEADER_SIZE) {
                        state = RECEIVING_DATA;
                        memcpy(rxBuffer, NEW_HEADER, HEADER_SIZE);
                        rxDataSize = HEADER_SIZE;
                    }
                } else {
                    // Reset if byte doesn't match expected header byte
                    headerCount = 0;
                }
                break;
            case RECEIVING_DATA:
                if (rxDataSize < sizeof(rxBuffer)) {
                    rxBuffer[rxDataSize++] = data;
                }
                break;
        }
    }
}

void ProcessAndSendData(void) {
    if (state == RECEIVING_DATA && rxDataSize > HEADER_SIZE) {
        // 发送数据
        for (uint8_t i = 0; i < rxDataSize; ++i) {
            USART_SendData(USART1, rxBuffer[i]);
            while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        }
        // 重置变量以准备接收下一个消息
        rxDataSize = 0;
        headerCount = 0;
        state = WAIT_FOR_HEADER;
    }
}