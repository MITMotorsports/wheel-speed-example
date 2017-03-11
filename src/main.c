#include "chip.h"
#include "board.h"

#define BAUDRATE 57600

const uint32_t OscRateIn = 12000000;

volatile uint32_t msTicks;

/****************************************************************************************
 * Private functions
 ***************************************************************************************/

static void GPIO_Config(void) {
	Chip_GPIO_Init(LPC_GPIO);

}

void SysTick_Handler(void) {
	msTicks++;
}

void TIMER32_0_IRQHandler(void) {
	if (Chip_TIMER_MatchPending(LPC_TIMER32_0, 1)) {
		Chip_TIMER_ClearMatch(LPC_TIMER32_0, 1);
		//LED_On();
	}
}

int main(void) {

	// Update system core clock rate
	SystemCoreClockUpdate();

	// Initialize and start system tick timer
	if (SysTick_Config(SystemCoreClock / 1000)) {
		Board_Println("Failed SysTick_Config");
		//Error
		while(1);
	}

	/* UART setup */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_6, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* RXD */
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_7, (IOCON_FUNC1 | IOCON_MODE_INACT)); /* TXD */
	Chip_UART_Init(LPC_USART);
	Chip_UART_SetBaud(LPC_USART, BAUDRATE);
	// Configure data width, parity, and stop bits
	Chip_UART_ConfigData(LPC_USART, (UART_LCR_WLEN8 | UART_LCR_SBS_1BIT | UART_LCR_PARITY_DIS));
	Chip_UART_SetupFIFOS(LPC_USART, (UART_FCR_FIFO_EN | UART_FCR_TRG_LEV2));
	Chip_UART_TXEnable(LPC_USART);

	// Initialize 32 bit timer
	Chip_TIMER_Init(LPC_TIMER32_0);

	/* Timer setup */
	// Reset timer terminal and prescale counts to zero
	Chip_TIMER_Reset(LPC_TIMER32_0);
	// Start the timer
	Chip_TIMER_Enable(LPC_TIMER32_0);

	// Clear pending bit of an external interrupt
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);
	// Enable timer interrupt
	NVIC_EnableIRQ(TIMER_32_0_IRQn);

	Board_Println("Started up");
										
	while(1){
		// Wait for interrupt
		__WFI();
	}

	return 0;
}
