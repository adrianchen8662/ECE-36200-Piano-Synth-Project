#include "fifo.h"
#include "tty.h"
#include "commands.h"
#include "diskio.h"
#include "ff.h"
#include "ffconf.h"
#include "lcd.h"
#include "tty.h"
#include "fifo.h"

//
#include "stm32f0xx.h"

int __io_putchar(int c)
{
    if(c == '\n')
    {
        while(!(USART5->ISR & USART_ISR_TXE)) { }
        USART5->TDR = '\r';
    }
    while(!(USART5->ISR & USART_ISR_TXE)) { }
    USART5->TDR = c;
    return c;
}

void enable_tty_interrupt()
{
    USART5->CR1 |= USART_CR1_RXNEIE; //Raise an interrupt every time the receive data register becomes not empty
    NVIC->ISER[0] |= 1<<USART3_8_IRQn; //Set the proper bit in the NVIC ISER
    USART5->CR3 |= USART_CR3_DMAR; //Trigger a DMA operation every time the receive data register becomes not empty

    //Enable the RCC clock for DMA Controller 2
    RCC->AHBENR |= RCC_AHBENR_DMA2EN;
    DMA2->RMPCR |= DMA_RMPCR2_CH2_USART5_RX;
    DMA2_Channel2->CCR &= ~DMA_CCR_EN; //First make sure DMA is turned off

    DMA2_Channel2->CMAR = serfifo; //CMAR should be set to the address of serfifo
    DMA2_Channel2->CPAR = &(USART5->RDR); //CPAR should be set to the address of the USART5 RDR
    DMA2_Channel2->CNDTR = FIFOSIZE; //CNDTR should be set to FIFOSIZE
    DMA2_Channel2->CCR &= ~DMA_CCR_DIR; //The DIRection of copying should be from peripheral to memory
    DMA2_Channel2->CCR &= ~DMA_CCR_TCIE; //Neither the total-completion nor the half-transfer interrupt should be enabled
    DMA2_Channel2->CCR &= ~DMA_CCR_HTIE; //Neither the total-completion nor the half-transfer interrupt should be enabled
    DMA2_Channel2->CCR &= ~DMA_CCR_MSIZE; //Both the MSIZE and the PSIZE should be set for 8 bits
    DMA2_Channel2->CCR &= ~DMA_CCR_PSIZE; //Both the MSIZE and the PSIZE should be set for 8 bits
    DMA2_Channel2->CCR &= ~DMA_CCR_PINC; //PINC should not be set so that CPAR always points at the USART5 RDR
    DMA2_Channel2->CCR |= DMA_CCR_MINC; //MINC should be set to increment the CMAR
    DMA2_Channel2->CCR |= DMA_CCR_CIRC; //Enable CIRCular transfers
    DMA2_Channel2->CCR &= ~DMA_CCR_MEM2MEM; //Do not enable MEM2MEM transfers
    DMA2_Channel2->CCR |= DMA_CCR_PL; //Set the Priority Level to highest
    DMA2_Channel2->CCR |= DMA_CCR_EN; //The channel is enabled for operation
}

int interrupt_getchar()
{
    while(fifo_newline(&input_fifo) == 0)
    {
        asm volatile ("wfi");
    }
    char ch = fifo_remove(&input_fifo);

    return ch;
}

int __io_getchar(void)
{
    return interrupt_getchar();
}

void USART3_4_5_6_7_8_IRQHandler(void)
{
    while(DMA2_Channel2->CNDTR != sizeof serfifo - seroffset)
    {
        if (!fifo_full(&input_fifo))
        {
            insert_echo_char(serfifo[seroffset]);
        }
        seroffset = (seroffset + 1) % sizeof serfifo;
    }
}

void init_spi1_slow()
{
    // Set the baud rate divisor to the maximum value to make the SPI baud rate as low as possible.
    // Set it to Master Mode.
    // Set the word size to 8-bit.
    // Configure "Software Slave Management" and "Internal Slave Select".
    // Set the "FIFO reception threshold" bit in CR2 so that the SPI channel immediately releases a received 8-bit value.
    // Enable the SPI channel.
}

void enable_sdcard()
{

}

void disable_sdcard()
{

}

void init_sdcard_io()
{
    init_spi1_slow();
    
    disable_sdcard();
}

void sdcard_io_high_speed()
{

}

void init_lcd_spi()
{

    init_spi1_slow();
    sdcard_io_high_speed();
}

int main()
{
	init_usart5();
	enable_tty_interrupt();
	setbuf(stdin,0);
	setbuf(stdout,0);
	setbuf(stderr,0);
	command_shell();
}
