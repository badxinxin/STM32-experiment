#include "stm32f10x.h"
#include "usart.h"

int main(void)
{
	  SystemInit();
    usart_init(115200);  // Initialize USART1, baud rate 115200

    while (1)
    {   
      ProcessAndSendData();  
    }		
}
