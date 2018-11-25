#include "LPC8xx.h"
#include "LPC8xx_uart.h"
#include "lpc8xx_syscon.h"
#include "lpc8xx_swm.h"
#include "utilities.h"


// Implementation of sendchar, hard-coded to UART0 (used by printf)
// This is for Keil projects.
int sendchar (int ch) {
  while (!((LPC_USART0->STAT) & TXRDY));   // Wait for TX Ready
  return (LPC_USART0->TXDAT  = ch);        // Write one character to TX data register
}


// Implementation of MyLowLevelPutchar, hard-coded to UART0 (used by printf)
// This is for IAR projects. Must include locally modified __write in the project.
int MyLowLevelPutchar(int ch) {
  while (!((LPC_USART0->STAT) & TXRDY));   // Wait for TX Ready
  return (LPC_USART0->TXDAT  = ch);        // Write one character to TX data register
}


// Implementation of getkey, hard-coded to UART0 (used by scanf)
// This is for Keil projects.
int getkey (void) {
  while (!((LPC_USART0->STAT) & RXRDY));   // Wait for RX Ready
  return (LPC_USART0->RXDAT );             // Read one character from RX data register
}


// Implementation of MyLowLevelGetchar, hard-coded to UART0 (used by scanf)
// This is for IAR projects. Must include locally modified __read in the project.
int MyLowLevelGetchar(void){
  while (!((LPC_USART0->STAT) & RXRDY));   // Wait for RX Ready
  return (LPC_USART0->RXDAT );             // Read one character from RX data register
}


// setup_debug_uart, hard coded to UART0, desired_baudrate/8/N/1
void setup_debug_uart() {

  // Select the baud rate
  const uint32_t desired_baud = 9600;
	
	// Turn on relevant clocks
  LPC_SYSCON->SYSAHBCLKCTRL |= (UART0 | SWM);

  // Connect UART0 TXD, RXD signals to port pins
  //ConfigSWM(U0_TXD, P0_4);       // Use with USB-to-RS232 break-out cable
  //ConfigSWM(U0_RXD, P0_0);       // Use with USB-to-RS232 break-out cable
  ConfigSWM(U0_TXD, TARGET_TX);  // For MBED serial port (requires board mod.)
  ConfigSWM(U0_RXD, TARGET_RX);  // For MBED serial port (requires board mod.)
	
  // UART BRG calculation:
  // For asynchronous mode (UART mode) the BRG formula is:
  // (BRG + 1) * (1 + (m/256)) * (UARTCLKDIV) * (16 * baudrate Hz.) = MainClock Hz.
  // As long as UARTCLKDIV = AHBCLKDIV, and FRG = 1, the System Clock and the UARTn_PCLKs will be the same.
  // For this example, we set m = 0 (so FRG = 1), and UARTCLKDIV = AHBCLKDIV.
	// Then, we can use the SystemCoreClock variable, as set by the function SystemCoreClockUpdate(),
	// in our BRG calculation as follows:
	// BRG = (SystemCoreClock Hz. / (16 * desired_baud Hz.)) - 1

  // Configure the UARTCLKDIV, default for calculation below is same as AHBCLKDIV
	LPC_SYSCON->UARTCLKDIV = LPC_SYSCON->SYSAHBCLKDIV;

  // Configure the FRG (default for calculation below is divide-by-1)
  LPC_SYSCON->UARTFRGMULT = 0;
  LPC_SYSCON->UARTFRGDIV = 255;

  // Give USART0 a reset
  LPC_SYSCON->PRESETCTRL &= (UART0_RST_N);
  LPC_SYSCON->PRESETCTRL |= ~(UART0_RST_N);

	// Get the System Clock frequency for the BRG calculation.
	SystemCoreClockUpdate();
	
	// Write calculation result to BRG register
	LPC_USART0->BRG = (SystemCoreClock / (16 * desired_baud)) - 1;

  // Configure the USART0 CFG register:
  // 8 data bits, no parity, one stop bit, no flow control, asynchronous mode
  LPC_USART0->CFG = DATA_LENG_8|PARITY_NONE|STOP_BIT_1;

  // Configure the USART0 CTL register (nothing to be done here)
  // No continuous break, no address detect, no Tx disable, no CC, no CLRCC
  LPC_USART0->CTL = 0;

  // Clear any pending flags (Just to be safe, isn't necessary after the peripheral reset)
  LPC_USART0->STAT = 0xFFFF;

  // Enable the USART0 RX Ready Interrupt, only if this project assumes an interrupt-driven use case
  //LPC_USART0->INTENSET = RXRDY;
  //NVIC_EnableIRQ(UART0_IRQn);

  // Enable USART0
  LPC_USART0->CFG |= UART_EN;
	
}
