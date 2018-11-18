/*
===============================================================================
 Name        : my2ndprj.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/
#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
/* Transmit and receive ring buffers */
STATIC RINGBUFF_T txring, rxring;

/* Transmit and receive ring buffer sizes */
#define UART_SRB_SIZE 128	/* Send */
#define UART_RRB_SIZE 32	/* Receive */

/* Transmit and receive buffers */
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];

const char inst1[] = "APC11xx UART example using ring buffers\r\n";
const char inst2[] = "Press a key to echo it back or ESC to quit\r\n";

/**
 * @brief	UART interrupt handler using ring buffers
 * @return	Nothing
 */
void UART_IRQHandler(void)
{
	/* Use default ring buffer handler. Override this with your own
	   code if you need more capability. */
	Chip_UART_IRQRBHandler(LPC_USART, &rxring, &txring);
}

int main(void) {
	uint8_t key;
	int bytes;
	int ph=0;
	int pl=0;
#if defined (__USE_LPCOPEN)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif
    Board_Debug_Init();
    // TODO: insert code here
	/* Before using the ring buffers, initialize them using the ring
	   buffer init function */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);

	/* Enable receive data and line status interrupt */
	Chip_UART_IntEnable(LPC_USART, (UART_IER_RBRINT | UART_IER_RLSINT));

	/* preemption = 1, sub-priority = 1  it  is able to run without below code */
	NVIC_SetPriority(UART0_IRQn, 1);
	NVIC_EnableIRQ(UART0_IRQn);

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
	/* Send initial messages */
	Chip_UART_SendRB(LPC_USART, &txring, inst1, sizeof(inst1) - 1);
	//Chip_UART_SendRB(LPC_USART, &txring, inst2, sizeof(inst2) - 1);
	/* Poll the receive ring buffer for the ESC (ASCII 27) key */
	key = 0;
	while (key != 27) {
		bytes = Chip_UART_ReadRB(LPC_USART, &rxring, &key, 1);
		if (bytes > 0) {
			/* Wrap value back around */
			if (Chip_UART_SendRB(LPC_USART, &txring, (const uint8_t *) &key, 1) != 1) {
				Board_LED_Toggle(0);/* Toggle LED if the TX FIFO is full */
			}
		}
        i++ ;
        if (i>20000) {
        	Board_LED_Set(0, false);
        	if (i>30000) {
        		i=0;
        		//Board_UARTPutChar('L');
        	}
        } else if (i<20000) {
        	Board_LED_Set(0, true);
        }
	}

	/* DeInitialize UART0 peripheral */
	NVIC_DisableIRQ(UART0_IRQn);
	Chip_UART_DeInit(LPC_USART);

    // Enter an infinite loop, just incrementing a counter
    while(0) {
        i++ ;
        if (i>20000) {
        	Board_LED_Set(0, false);
        	if(pl==0) {
        		//Board_UARTPutChar('L');
        		//Board_UARTPutSTR("LOW");
        		//Chip_UART_SendBlocking(LPC_USART, "low", 3);
        		Chip_UART_SendRB(LPC_USART, &txring,  "LOW", 3);
        		pl=1;
        	}
        	if (i>30000) {
        		i=0;
        		pl=0;
        		ph=0;
        	}
        }
        if (i<20000) {
        	Board_LED_Set(0, true);
        	if (ph==0){
        		//Board_UARTPutChar('H');
        		//Board_UARTPutSTR("HIGH");
        		//Chip_UART_SendBlocking(LPC_USART, "high", 4);
        		Chip_UART_SendRB(LPC_USART, &txring,  "HIGH", 4);
        		ph=1;
        	}
        }
    }
    return 0 ;
}
