#include "chip.h"
#include "board.h"

#define BAUDRATE 57600

#define TIME_BETWEEN_UART_MESSAGES 1000

const uint32_t OscRateIn = 12000000;

volatile uint32_t msTicks;

uint32_t clockCyclesBetweenTicks;
uint32_t lastUARTMessageTime;


void SysTick_Handler(void) {
	msTicks++;
}

void TIMER32_0_IRQHandler(void) {
	Chip_TIMER_Reset(LPC_TIMER32_0);		/* Reset the timer immediately */
	Chip_TIMER_ClearCapture(LPC_TIMER32_0, 0);	/* Clear the capture */
	clockCyclesBetweenTicks = Chip_TIMER_ReadCapture(LPC_TIMER32_0,0);
}

int main(void) {

	// Initialize variables
	clockCyclesBetweenTicks = 0;

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

	// Timer Initialization
	Chip_TIMER_Init(LPC_TIMER32_0);
	Chip_TIMER_Reset(LPC_TIMER32_0);		/* Reset the timer */
	Chip_TIMER_PrescaleSet(LPC_TIMER32_0, 0);	/* Set the prescaler to zero */
	LPC_TIMER32_0->CCR |= 5;			/* Set the first and third bits of
							   the capture control register to
							   1 */ 

	// Interrupt Setup
	Chip_IOCON_PinMuxSet(LPC_IOCON, IOCON_PIO1_5, (IOCON_FUNC2|IOCON_MODE_INACT));	
						/* Set PIO1_5 to the 32 bit timer capture 
						 * function */
	NVIC_SetPriority(SysTick_IRQn, 1);	/* Give the SysTick function a lower 
						   priority */
	NVIC_SetPriority(TIMER_32_0_IRQn, 0);	/* Give 32 bit timer capture 
						   interrupt the highest priority */
	NVIC_ClearPendingIRQ(TIMER_32_0_IRQn);	/* Clear pending interrupt */
	NVIC_EnableIRQ(TIMER_32_0_IRQn);	/* Enable timer interrupt */

	// Start the timer
	Chip_TIMER_Enable(LPC_TIMER32_0);

	Board_Println("Started up");
										
	while(1){

		if ( (msTicks - lastUARTMessageTime) > TIME_BETWEEN_UART_MESSAGES) {
			lastUARTMessageTime = msTicks;
			Board_Print("clockCyclesBetweenTicks: ");
			Board_Println_Int(clockCyclesBetweenTicks, 10);
		}

	}

	return 0;
}
