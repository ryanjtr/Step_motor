/*
 *     uart.c
 *       
 */
#include "uart.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/interrupt.h>

static const char * const g_pcHex = "0123456789abcdef";



#define USART0_ENABLE

#ifdef USART0_ENABLE
	usart_meta_t	UART0_meta;
	usart_meta_t	*p_UART0_meta = &UART0_meta;
#endif

#ifdef USART1_ENABLE
	usart_meta_t	UART1_meta;
	usart_meta_t	*p_UART1_meta = &UART1_meta;
#endif


#ifdef	UART0_CONSOLE
	#define CONSOLE_UART_WRITE(pdata, len)	usart0_send_array(pdata, len)
#elif   defined(UART1_CONSOLE)
	#define CONSOLE_UART_WRITE(pdata, len)	usart1_send_array(pdata, len)
#else
#define CONSOLE_UART_WRITE(data)
#endif 

#define USART_RX_ERROR_MASK ((1 << FE0) | (1 << DOR0) | (1 << UPE0))  

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// RINGBUFFER FUNCTIONS
void rbuffer_init(volatile ringbuffer_t* rb) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rb->in = 0;
        rb->out = 0;
        rb->count = 0;
    }
}

uint8_t rbuffer_count(volatile ringbuffer_t* rb) {
    return rb->count;
}

bool rbuffer_full(volatile ringbuffer_t* rb) {
    return (rb->count == (uint8_t)RBUFFER_SIZE);
}

bool rbuffer_empty(volatile ringbuffer_t* rb) {
    return (rb->count == 0);
}

void rbuffer_insert(char data, volatile ringbuffer_t* rb) {   
    *(rb->buffer + rb->in) = data;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rb->in = (rb->in + 1) & ((uint8_t)RBUFFER_SIZE - 1);
        rb->count++;
    }
}

char rbuffer_remove(volatile ringbuffer_t* rb) {
    char data = *(rb->buffer + rb->out);
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        rb->out = (rb->out + 1) & ((uint8_t)RBUFFER_SIZE - 1);
        rb->count--;
    }
    return data;
}

// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----


// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// USART FUNCTIONS




#ifdef USART0_ENABLE

void usart0_init(void) {
	DDRD &= ~(1<<PIND1);
	PORTD |= (1<<PIND1);
    rbuffer_init(&p_UART0_meta->rb_tx);                             // Init Rx buffer
    rbuffer_init(&p_UART0_meta->rb_rx);                             // Init Tx buffer
	UBRR0H=0;
	UBRR0L=51;
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);					//enable TX, RX, RX interrupt
	UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01); // 8bit
	//UCSR0A |= (1<<U2X0);
	sei();
}

void usart0_send_char( char c) {
//	if ((UCSR0B & (1 << TXC)) != (1 << TXC))
//	{
//		UDR0 = c;
//		 UCSR0B |= 1 << TXC; 
//		 return;
//	}
	
    while(rbuffer_full(&p_UART0_meta->rb_tx));
    rbuffer_insert(c, &p_UART0_meta->rb_tx);
    UCSR0B |= 1 << UDRE0;                   // Enable Tx buffer empty interrupt 
}

void usart0_send_string(const char* str) {
    while (*str) {
        usart0_send_char(*str++);
    }
}

void	usart0_send_array(const char* str, uint8_t	len)
{
	uint8_t	udx;
	for(udx = 0;udx < len; udx++)	 usart0_send_char(*str++);
}

void usart0_send_string_P(const char* chr) {
    char c;
    while ((c = pgm_read_byte(chr++))) {
        usart0_send_char(c);
    }
}

uint8_t usart0_rx_count(void) {
    return rbuffer_count(&p_UART0_meta->rb_rx);
}

uint16_t usart0_read_char(void) {
    if (!rbuffer_empty(&p_UART0_meta->rb_rx)) {
        return (((p_UART0_meta->usart_error & USART_RX_ERROR_MASK) << 8) | (uint16_t)rbuffer_remove(&p_UART0_meta->rb_rx));
    }
    else {
        return (((p_UART0_meta->usart_error & USART_RX_ERROR_MASK) << 8) | USART_NO_DATA);     // Empty ringbuffer
    }
}

void usart0_close() {
    while(!rbuffer_empty(&p_UART0_meta->rb_tx));                        // Wait for Tx to transmit ALL characters in ringbuffer
    while(!(UCSR0A & (1 << TXC0)));										// Wait for Tx unit to transmit the LAST character of ringbuffer

    _delay_ms(200);                                             // Extra safety for Tx to finish!

	UCSR0B &= ~( (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0) | (1<<UDRIE0));					//disable TX, RX, RX interrupt
	UCSR0C &= (1<<UCSZ01) | (1<<UCSZ00);
}

volatile ringbuffer_t * uart_get_uart0_rx_buffer_address(void)
{
	return &(p_UART0_meta->rb_rx);
}
#endif




// ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
// ISR FUNCTIONS
#ifdef USART0_ENABLE

ISR(USART_RX_vect) {
	
   char	data = UDR0;


    if(!rbuffer_full(&p_UART0_meta->rb_rx)) {
	    rbuffer_insert(data, &p_UART0_meta->rb_rx);
	    p_UART0_meta->usart_error = UCSR0A & USART_RX_ERROR_MASK ;

    }
    else {
	    p_UART0_meta->usart_error = ((UCSR0A & USART_RX_ERROR_MASK) | USART_BUFFER_OVERFLOW>>8);
    }   
}
ISR(USART_UDRE_vect) {
    if(!rbuffer_empty(&p_UART0_meta->rb_tx)) {
	    UDR0 = rbuffer_remove(&p_UART0_meta->rb_tx);
    }
    else {
	    UCSR0B &= ~(1 << UDRE0);                   // Enable Tx buffer empty interrupt 
    }
}
#endif



//*****************************************************************************
//
//! A simple UART based vprintf function supporting \%c, \%d, \%p, \%s, \%u,
//! \%x, and \%X.
//!
//! \param pcString is the format string.
//! \param vaArgP is a variable argument list pointer whose content will depend
//! upon the format string passed in \e pcString.
//!
//! This function is very similar to the C library <tt>vprintf()</tt> function.
//! All of its output will be sent to the UART.  Only the following formatting
//! characters are supported:
//!
//! - \%c to print a character
//! - \%d or \%i to print a decimal value
//! - \%s to print a string
//! - \%u to print an unsigned decimal value
//! - \%x to print a hexadecimal value using lower case letters
//! - \%X to print a hexadecimal value using lower case letters (not upper case
//! letters as would typically be used)
//! - \%p to print a pointer as a hexadecimal value
//! - \%\% to print out a \% character
//!
//! For \%s, \%d, \%i, \%u, \%p, \%x, and \%X, an optional number may reside
//! between the \% and the format character, which specifies the minimum number
//! of characters to use for that value; if preceded by a 0 then the extra
//! characters will be filled with zeros instead of spaces.  For example,
//! ``\%8d'' will use eight characters to print the decimal value with spaces
//! added to reach eight; ``\%08d'' will use eight characters as well but will
//! add zeroes instead of spaces.
//!
//! The type of the arguments in the variable arguments list must match the
//! requirements of the format string.  For example, if an integer was passed
//! where a string was expected, an error of some kind will most likely occur.
//!
//! \return None.
//
//*****************************************************************************
void
UARTvprintf(const char *pcString, va_list vaArgP)
{
   volatile uint32_t ui32Idx, ui32Value, ui32Pos, ui32Count, ui32Base, ui32Neg;
    char *pcStr, pcBuf[16], cFill;

    //
    // Loop while there are more characters in the string.
    //
    while(*pcString)
    {
        //
        // Find the first non-% character, or the end of the string.
        //
        for(ui32Idx = 0;
            (pcString[ui32Idx] != '%') && (pcString[ui32Idx] != '\0');
            ui32Idx++)
        {
        }

        //
        // Write this portion of the string.
        //
        CONSOLE_UART_WRITE(pcString, ui32Idx);

        //
        // Skip the portion of the string that was written.
        //
        pcString += ui32Idx;

        //
        // See if the next character is a %.
        //
        if(*pcString == '%')
        {
            //
            // Skip the %.
            //
            pcString++;

            //
            // Set the digit count to zero, and the fill character to space
            // (in other words, to the defaults).
            //
            ui32Count = 0;
            cFill = ' ';

            //
            // It may be necessary to get back here to process more characters.
            // Goto's aren't pretty, but effective.  I feel extremely dirty for
            // using not one but two of the beasts.
            //
again:

            //
            // Determine how to handle the next character.
            //
            switch(*pcString++)
            {
                //
                // Handle the digit characters.
                //
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                {
                    //
                    // If this is a zero, and it is the first digit, then the
                    // fill character is a zero instead of a space.
                    //
                    if((pcString[-1] == '0') && (ui32Count == 0))
                    {
                        cFill = '0';
                    }

                    //
                    // Update the digit count.
                    //
                    ui32Count *= 10;
                    ui32Count += pcString[-1] - '0';

                    //
                    // Get the next character.
                    //
                    goto again;
                }

                //
                // Handle the %c command.
                //
                case 'c':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32Value = va_arg(vaArgP, uint32_t);

                    //
                    // Print out the character.
                    //
                    CONSOLE_UART_WRITE((char *)&ui32Value, 1);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %d and %i commands.
                //
                case 'd':
                case 'i':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32Value = va_arg(vaArgP, uint16_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32Pos = 0;

                    //
                    // If the value is negative, make it positive and indicate
                    // that a minus sign is needed.
                    //
                    if((int32_t)ui32Value < 0)
                    {
                        //
                        // Make the value positive.
                        //
                        ui32Value = -(int32_t)ui32Value;

                        //
                        // Indicate that the value is negative.
                        //
                        ui32Neg = 1;
                    }
                    else
                    {
                        //
                        // Indicate that the value is positive so that a minus
                        // sign isn't inserted.
                        //
                        ui32Neg = 0;
                    }

                    //
                    // Set the base to 10.
                    //
                    ui32Base = 10;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %s command.
                //
                case 's':
                {
                    //
                    // Get the string pointer from the varargs.
                    //
                    pcStr = va_arg(vaArgP, char *);

                    //
                    // Determine the length of the string.
                    //
                    for(ui32Idx = 0; pcStr[ui32Idx] != '\0'; ui32Idx++)
                    {
                    }

                    //
                    // Write the string.
                    //
                    CONSOLE_UART_WRITE(pcStr, ui32Idx);

                    //
                    // Write any required padding spaces
                    //
                    if(ui32Count > ui32Idx)
                    {
                        ui32Count -= ui32Idx;
                        while(ui32Count--)
                        {
                            CONSOLE_UART_WRITE(" ", 1);
                        }
                    }

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %u command.
                //
                case 'u':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32Value = va_arg(vaArgP, uint16_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32Pos = 0;

                    //
                    // Set the base to 10.
                    //
                    ui32Base = 10;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ui32Neg = 0;

                    //
                    // Convert the value to ASCII.
                    //
                    goto convert;
                }

                //
                // Handle the %x and %X commands.  Note that they are treated
                // identically; in other words, %X will use lower case letters
                // for a-f instead of the upper case letters it should use.  We
                // also alias %p to %x.
                //
                case 'x':
                case 'X':
                case 'p':
                {
                    //
                    // Get the value from the varargs.
                    //
                    ui32Value = va_arg(vaArgP, uint16_t);

                    //
                    // Reset the buffer position.
                    //
                    ui32Pos = 0;

                    //
                    // Set the base to 16.
                    //
                    ui32Base = 16;

                    //
                    // Indicate that the value is positive so that a minus sign
                    // isn't inserted.
                    //
                    ui32Neg = 0;

                    //
                    // Determine the number of digits in the string version of
                    // the value.
                    //
convert:
                    for(ui32Idx = 1;
                        (((ui32Idx * ui32Base) <= ui32Value) &&
                         (((ui32Idx * ui32Base) / ui32Base) == ui32Idx));
                        ui32Idx *= ui32Base, ui32Count--)
                    {
                    }

                    //
                    // If the value is negative, reduce the count of padding
                    // characters needed.
                    //
                    if(ui32Neg)
                    {
                        ui32Count--;
                    }

                    //
                    // If the value is negative and the value is padded with
                    // zeros, then place the minus sign before the padding.
                    //
                    if(ui32Neg && (cFill == '0'))
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pcBuf[ui32Pos++] = '-';

                        //
                        // The minus sign has been placed, so turn off the
                        // negative flag.
                        //
                        ui32Neg = 0;
                    }

                    //
                    // Provide additional padding at the beginning of the
                    // string conversion if needed.
                    //
                    if((ui32Count > 1) && (ui32Count < 16))
                    {
                        for(ui32Count--; ui32Count; ui32Count--)
                        {
                            pcBuf[ui32Pos++] = cFill;
                        }
                    }

                    //
                    // If the value is negative, then place the minus sign
                    // before the number.
                    //
                    if(ui32Neg)
                    {
                        //
                        // Place the minus sign in the output buffer.
                        //
                        pcBuf[ui32Pos++] = '-';
                    }

                    //
                    // Convert the value into a string.
                    //
                    for(; ui32Idx; ui32Idx /= ui32Base)
                    {
                        pcBuf[ui32Pos++] =
                            g_pcHex[(ui32Value / ui32Idx) % ui32Base];
                    }

                    //
                    // Write the string.
                    //
                    CONSOLE_UART_WRITE(pcBuf, ui32Pos);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle the %% command.
                //
                case '%':
                {
                    //
                    // Simply write a single %.
                    //
                    CONSOLE_UART_WRITE(pcString - 1, 1);

                    //
                    // This command has been handled.
                    //
                    break;
                }

                //
                // Handle all other commands.
                //
                default:
                {
                    //
                    // Indicate an error.
                    //
                    CONSOLE_UART_WRITE("ERROR", 5);

                    //
                    // This command has been handled.
                    //
                    break;
                }
            }
        }
    }
}

//*****************************************************************************
//
//! A simple UART based printf function supporting \%c, \%d, \%p, \%s, \%u,
//! \%x, and \%X.
//!
//! \param pcString is the format string.
//! \param ... are the optional arguments, which depend on the contents of the
//! format string.
//!
//! This function is very similar to the C library <tt>fprintf()</tt> function.
//! All of its output will be sent to the UART.  Only the following formatting
//! characters are supported:
//!
//! - \%c to print a character
//! - \%d or \%i to print a decimal value
//! - \%s to print a string
//! - \%u to print an unsigned decimal value
//! - \%x to print a hexadecimal value using lower case letters
//! - \%X to print a hexadecimal value using lower case letters (not upper case
//! letters as would typically be used)
//! - \%p to print a pointer as a hexadecimal value
//! - \%\% to print out a \% character
//!
//! For \%s, \%d, \%i, \%u, \%p, \%x, and \%X, an optional number may reside
//! between the \% and the format character, which specifies the minimum number
//! of characters to use for that value; if preceded by a 0 then the extra
//! characters will be filled with zeros instead of spaces.  For example,
//! ``\%8d'' will use eight characters to print the decimal value with spaces
//! added to reach eight; ``\%08d'' will use eight characters as well but will
//! add zeroes instead of spaces.
//!
//! The type of the arguments after \e pcString must match the requirements of
//! the format string.  For example, if an integer was passed where a string
//! was expected, an error of some kind will most likely occur.
//!
//! \return None.
//
//*****************************************************************************
void
UARTprintf(const char *pcString, ...)
{
    va_list vaArgP;

    //
    // Start the varargs processing.
    //
    va_start(vaArgP, pcString);

    UARTvprintf(pcString, vaArgP);

    //
    // We're finished with the varargs now.
    //
    va_end(vaArgP);
}