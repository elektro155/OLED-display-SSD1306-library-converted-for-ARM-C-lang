/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "SSD1306.h"


////////////////////////////// Systick (delay) ///////////////////////////////////

volatile uint32_t g_ul_ms_ticks = 0;

void SysTick_Handler(void)
{
	g_ul_ms_ticks++;
}

void mdelay(uint32_t ul_dly_ticks);

void mdelay(uint32_t ul_dly_ticks)
{
	uint32_t ul_cur_ticks;

	ul_cur_ticks = g_ul_ms_ticks;
	while ((g_ul_ms_ticks - ul_cur_ticks) < ul_dly_ticks);
}

/////////////////////////////// UART0 config and IRQ /////////////////////////////

static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.paritytype = CONF_UART_PARITY
	};
	
	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
	
	//receiving
	uart_enable_tx(CONSOLE_UART);
	uart_enable_rx(CONSOLE_UART);
	uart_enable_interrupt(CONSOLE_UART, US_IER_RXRDY);
	NVIC_EnableIRQ(UART0_IRQn);
}

void UART0_Handler(void)
{	
	char buffer [100];
	uint32_t counter = 0;
	
	uint32_t dw_status = uart_get_status(CONSOLE_UART);
	
	if (dw_status & US_CSR_RXRDY)
	{
		Disable_global_interrupt();
		
		//timeout variables
		const uint32_t TIMEOUT_VAL = 15000;
		uint32_t timeout_counter = 0;
		
		//read data
		while(1) 
		{
			uint8_t received_byte;
			if(uart_read(CONSOLE_UART, &received_byte))
			{
				//not read
				++timeout_counter;
				if (timeout_counter >= TIMEOUT_VAL) break;
				else continue;				
			}
			else
			{
				//byte read
				timeout_counter = 0;
			}
			buffer[counter] = received_byte;
			if (buffer[counter++] == '\n') break;
		}
		
		//do sth with data
		for(uint8_t a = 0; a < counter; a++)
		{
			while(uart_write(CONSOLE_UART, buffer[a]));
		}
		puts("Responded by IRQ\r");
		
		Enable_global_interrupt();
	}
		
}

////////////////////////////// initializations /////////////////////////////////

static void uc_init(void)
{
	//uc init
	sysclk_init();
	board_init();
	
	//systick init
	if (SysTick_Config(sysclk_get_cpu_hz() / 1000)) {
		while (1); /* Capture error */
	}
		
	//GPIO  init
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);
	
	//turn off all LEDs
	LED_Off(LED0_GPIO);
	LED_Off(LED1_GPIO);
	
	//I2C init
	pmc_enable_periph_clk(ID_TWI0);
	twi_master_enable(TWI0);
	
	twi_options_t opt;
	opt.master_clk = sysclk_get_peripheral_hz();
	opt.speed      = 400000;
	if (twi_master_init(TWI0, &opt) != TWI_SUCCESS)
	{
		while(1); //error
	}
	
	//console init
	configure_console();
	
	//end of uc init, turn on LED0
	LED_On(LED0_GPIO);
}

static void hw_init(void)
{
	//I2C init
	i2c_begin(); 
	i2c_clear(); 
	i2c_setCursor(0, 0);  //x,y
	
	//end of hw init, turn on LED1
	LED_On(LED1_GPIO);
}

int main (void)
{
	//initializations
	uc_init();
	hw_init();

	//i2c test
	for(uint8_t a = 1; a < 10; a++)
	{
		i2c_writeDigit(a);
	}
	
	i2c_print("    TEST");
	
	//uart test
	puts("-- PDC_UART Example --\r");
	
	mdelay(200);
	i2c_setCursor(0, 2);  //x,y
	
	char _char = 'A';
	for(uint8_t a = 0; a < 20; a++)
	{
		i2c_write_char(_char++);
		mdelay(200);
	}
	i2c_write_char('\n');
	
	static const uint8_t key_down [] = {
		0x00, 0x80, 0xE0, 0xF8, 0xFC, 0xFE, 0xFF, 0x3F, 0x1F, 0x0F, 0x0F, 0x1F, 0x3F, 0xFF, 0xFE, 0xFC,
		0xF8, 0xF0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFC, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
		0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
		0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0x00, 0x00, 0x00, 0x00,
		0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xF0, 0xFF,
		0xFF, 0xFF, 0xFF, 0x1F, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
		0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0x07, 0x07, 0x07, 0x07, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0x7E, 0xFC, 0xF8, 0xF8, 0xFC, 0x7E, 0x7F, 0x3F, 0x1F,
		0x0F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x07,
		0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x0F, 0x0F, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x00
	};
	
	i2c_draw(key_down,32,32,64,32);
	
	//flag for avoiding constant writing to display
	uint8_t cleared = 0;

	while(1)
	{
		if (!gpio_pin_is_high(GPIO_PUSH_BUTTON_1))
		{
			cleared  = 1;
			i2c_setCursor(0, 4);  //x,y
			i2c_print("Button pressed");
		}
		else
		{
			if(cleared != 0)
			{
				i2c_setCursor(0, 4);  //x,y
				i2c_print("              ");
				cleared = 0;
			}
		} //if pin
		
		//LED_On(LED1_GPIO);
		//mdelay(1000);
		//LED_Off(LED1_GPIO);
		//mdelay(1000);
	} //while
}
