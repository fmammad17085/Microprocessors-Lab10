#include <avr/io.h>
#include <util/delay.h>


void uart_init(void) {
    // (20-1):  UBRR0 = F_CPU / (16 × BAUD) – 1
    // For 16 MHz and 9600 baud → 103
    uint16_t ubrr = 103;

    UBRR0H = (uint8_t)(ubrr >> 8);    // High byte of UBRR (page 149)
    UBRR0L = (uint8_t)ubrr;           // Low byte

    // Table 19-4: Asynchronous USART, No Parity, 1 Stop bit
    // Table 19-7: UCSZ01:0 = 11 → 8-bit data
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    // Table 19-10.3: RXEN0 enables the Receiver
    UCSR0B = (1 << RXEN0);
}


uint8_t uart_receive(void) {
    // Wait until data is received (RXC0 = 1) — Table 19-10.2 bit 7
    while (!(UCSR0A & (1 << RXC0)));

    return UDR0;   // Read received byte from buffer
}


void leds_init(void) {
    // PB0, PB1, PB2 as outputs for the 3 LEDs
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);
}


void leds_show(uint8_t v) {
    // Clear all LEDs first
    PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));

    // Turn ON only the LED corresponding to the value
    if (v == 1) {
        PORTB |= (1 << PB0);
    } else if (v == 2) {
        PORTB |= (1 << PB1);
    } else if (v == 3) {
        PORTB |= (1 << PB2);
    }
}


// ---------------- EEPROM ----------------


void eeprom_write(uint16_t addr, uint8_t data) {
    // Wait until previous write operation completes
    // EEPE = 1 while writing (Table 24-5)
    while (EECR & (1 << EEPE));

    EEAR = addr;     // Set EEPROM address register
    EEDR = data;     // Load data register

    // Required sequence: first set EEMPE, then EEPE (Table 24-5)
    EECR |= (1 << EEMPE);  // Master Write Enable (write allowed for 4 CPU cycles)
    EECR |= (1 << EEPE);   // Start EEPROM write
}


uint8_t eeprom_read(uint16_t addr) {
    // Wait until previous write completes
    while (EECR & (1 << EEPE));

    EEAR = addr;              // Set address
    EECR |= (1 << EERE);      // Trigger EEPROM read
    return EEDR;              // Return data read from EEPROM
}



// ---------------- MAIN PROGRAM ----------------


int main(void) {
    uart_init();    // Initialize UART (9600 baud, 8N1)
    leds_init();    // Configure PB0–PB2 as LED outputs

    uint16_t index = 0;                // EEPROM write address
    uint8_t c;
    uint8_t ready_for_new_sequence = 1; // Reset index on the first element

    while (1) {
        c = uart_receive();   // Wait for UART byte

        // ------------------------ RECORD 1–3 INTO EEPROM ------------------------
        if (c == 1 || c == 2 || c == 3) {

            if (ready_for_new_sequence == 1) {
                index = 0;              // Start new sequence at EEPROM[0]
                ready_for_new_sequence = 0;
            }

            eeprom_write(index, c);     // Store LED pattern to EEPROM
            index++;                    // Move to next address
        }


        // ------------------------ DISPLAY SEQUENCE FROM EEPROM ------------------------
        else if (c == 4) {
            for (uint16_t i = 0; i < index; i++) {
                uint8_t val = eeprom_read(i); // Read EEPROM[i]
                leds_show(val);               // Show LED 1/2/3
                _delay_ms(500);
            }
        }


        // ------------------------ RESET SEQUENCE ------------------------
        else if (c == 0) {
            index = 0;                     // Wipe sequence index
            ready_for_new_sequence = 1;    // Next 1–3 starts a new recording
        }

        else {
            // ignore unknown commands
        }
    }
}
