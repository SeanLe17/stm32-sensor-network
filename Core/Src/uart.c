#include "uart.h"
#include "stm32f446xx.h"
#include "stm32f4xx.h"
#define UART_RX_BUFFER_SIZE 16u

static volatile uint32_t uart2_rx_count = 0;
static volatile char rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint32_t rx_head = 0;
static volatile uint32_t rx_tail = 0;
static volatile uint32_t rx_dropped = 0;

static void uart2_rx_push(char c){
    uint32_t next_head = (rx_head + 1) % UART_RX_BUFFER_SIZE;
    if(next_head == rx_tail){
        rx_dropped++;
    }
    else{
        rx_buffer[rx_head] = c;
        rx_head = next_head;
    }
}

bool uart2_pop(char *out)
{
    if(rx_head == rx_tail)
    {
        return false;
    }
    *out = rx_buffer[rx_tail];
    rx_tail = (rx_tail +1) %UART_RX_BUFFER_SIZE;
    return true;
}
void uart2_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; //enable clock
    (void)RCC->APB1ENR; // read clock register (delay)

    //Disable synchronous
    USART2->CR1 = 0;
    USART2->CR2 = 0;
    USART2->CR3 = 0;

    USART2->BRR = 365u; // load baud rate
    USART2->CR1 |= USART_CR1_TE; // enable transmitter
    USART2->CR1 |= USART_CR1_RE; // enable reciever
    USART2->CR1 |= USART_CR1_UE; // enable opeartion
    NVIC_SetPriority(USART2_IRQn, 5);
    NVIC_EnableIRQ(USART2_IRQn);
    USART2->CR1 |= USART_CR1_RXNEIE;
}

void uart2_write_char(char c)
{
    while((USART2->SR & USART_SR_TXE) == 0) // polling
    {

    }
    USART2->DR = c;
}

void uart2_write_string(const char *text){
    while(*text!= '\0'){
        uart2_write_char(*text);
        text++;
    }
}



void USART2_IRQHandler(void)
{
    if(USART2->SR & USART_SR_RXNE)
    {
        char recieved = (char) USART2->DR;
        uart2_rx_count++;
        uart2_rx_push(recieved);

    }


}