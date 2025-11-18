#include <avr/io.h>
#include <util/delay.h>


void uart_init(void) {
    // (20-1):  UBRR0 = FCPU / (16×BAUD) – 1, For 16 MHz and 9600 baud = 103 
    uint16_t ubrr = 103;
    UBRR0H = (uint8_t)(ubrr >> 8); //(page 149 datasheet)
    UBRR0L = (uint8_t)ubrr;

    // Table 19-4. Asynchronous (UMSEL01 = 00), No Parity (UPM01 = 00),1 Stop bit (USBS0 = 0)

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Table 19-7 8 data bits (UCSZ01 = 11)
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);      // enable transmitter + receiver Table 19.10.3 
}


uint8_t uart_receive(void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}


void uart_transmit(uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait until Data Register Empty flag (UDRE0) = 1, Table 19.10.2 bit 5
    UDR0 = data;                 // UDR = Data Buffer, write data shifts out via PD1 (TX)
}


int main(void) {
    uart_init();
    
    uint8_t c;
    uint8_t send_val;
    
    while (1) {
        c = uart_receive();
        
        if (c == '1') {
            send_val = 1;
            uart_transmit(send_val);
        }
        else if (c == '2') {
            send_val = 2;
            uart_transmit(send_val);
        }
        else if (c == '3') {
            send_val = 3;
            uart_transmit(send_val);
        }
        else if (c == '4') {
            send_val = 4;
            uart_transmit(send_val);
        }
        else if (c == '0') {
            send_val = 0;
            uart_transmit(send_val);
        }
    }
}
