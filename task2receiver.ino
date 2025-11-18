#include <avr/io.h>

void uart_init(void) {
    // (20-1):  UBRR0 = FCPU / (16×BAUD) – 1, For 16 MHz and 9600 baud = 103 
    uint16_t ubrr = 103;               // Eq. (20-1) for 9600 baud
    UBRR0H = (uint8_t)(ubrr >> 8); //(page 149 datasheet)
    UBRR0L = (uint8_t)ubrr;

    // Table 19-4. Asynchronous (UMSEL01 = 00), No Parity (UPM01 = 00),1 Stop bit (USBS0 = 0)

    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Table 19-7 8 data bits (UCSZ01 = 11)
    UCSR0B = (1 << RXEN0); // enable receiver Table 19.10.3 
}


uint8_t uart_receive(void) {
    while (!(UCSR0A & (1 << RXC0)));   // Wait until Receive Complete (RXC0) = 1, Table 19.10.2 bit 5
    return UDR0;                       // Read data from buffer
}


void leds_init(void) {
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);   // outputs
}

void leds_show(uint8_t val) {
    PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));

    if (val == 1) PORTB |= (1 << PB0);
    else if (val == 2) PORTB |= (1 << PB1);
    else if (val == 3) PORTB |= (1 << PB2);
}


int main(void) {
    uart_init();
    leds_init();

    while (1) {
        uint8_t data = uart_receive(); // wait for byte
        leds_show(data);               // display 1–3
    }
}