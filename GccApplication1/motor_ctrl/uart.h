/*
 *     uart.h
 *
 *          Description:  UART for megaAVR, tinyAVR & AVR DA DD DB EA
 *          Author:       Hans-Henrik Fuxelius
 *          Date:         Uppsala, 2023-05-29
 *          License:      MIT
 *          Version:      RC1
 */
#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>



// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// DEFINE RING BUFFER SIZE; MUST BE 2, 4, 8, 16, 32, 64 or 128
#define RBUFFER_SIZE 64

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

#define USART0_ENABLE
#define USART1_ENABLE
#define UART1_CONSOLE


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
#define USART_BUFFER_OVERFLOW    1 << 8      // ==USART_BUFOVF_bm
#define USART_FRAME_ERROR        1 << 9      // ==USART_FERR_bm
#define USART_PARITY_ERROR       1 << 10      // ==USART_PERR_bm
#define USART_NO_DATA            1 << 11     

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
#define BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// RINGBUFFER STRUCT
typedef struct { 
    volatile char     buffer[RBUFFER_SIZE];
    volatile uint8_t  in;
    volatile uint8_t  out;
    volatile uint8_t  count;
} ringbuffer_t;

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART META STRUCT
typedef struct { 
    volatile ringbuffer_t rb_rx;    // Rx ringbuffer
    volatile ringbuffer_t rb_tx;    // Tx ringbuffer
    volatile uint16_t usart_error;   // Holds error from RXDATAH
} usart_meta_t;

void rbuffer_init(volatile ringbuffer_t* rb);
uint8_t rbuffer_count(volatile ringbuffer_t* rb);
bool rbuffer_full(volatile ringbuffer_t* rb);
bool rbuffer_empty(volatile ringbuffer_t* rb);
void rbuffer_insert(char data, volatile ringbuffer_t* rb);
char rbuffer_remove(volatile ringbuffer_t* rb);

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART FUNCTIONS
void	usart0_init(void);			//init at 9600 bps, FCPU = 8Mhz
void	usart0_send_char( char c);
void	usart0_send_string(const char* str);
void	usart0_send_array(const char* str, uint8_t	len);
void	usart0_send_string_P(const char* chr);
uint8_t usart0_rx_count(void);
uint16_t usart0_read_char(void);
void	usart0_close(void);
volatile ringbuffer_t * uart_get_uart0_rx_buffer_address(void);

void	usart1_init(void);			//init at 9600 bps, FCPU = 8Mhz
void	usart1_send_char( char c);
void	usart1_send_string(const char* str);
void	usart1_send_array(const char* str, uint8_t	len);
void	usart1_send_string_P(const char* chr);
uint8_t usart1_rx_count(void);
uint16_t usart1_read_char(void);
void	usart1_close(void);
volatile ringbuffer_t * uart_get_uart1_rx_buffer_address(void);


void UARTvprintf(const char *pcString, va_list vaArgP);
void UARTprintf(const char *pcString, ...);
#endif