#ifndef WS2812B_LGT_C
#define WS2812B_LGT_C

#include "WS2812B_LGT.hpp"

#if !(F_CPU == 8000000 || F_CPU == 16000000 || F_CPU == 20000000 || F_CPU == 32000000)
#error "On an AVR, this version of the PololuLedStrip library only supports 8, 16 20 and 32 MHz."
#endif

// Only support LGT8F328P pins
const unsigned char pinBit[] =
{
	1, 2, 4, 8, 16, 32, 64, 128,  // PORTD
	1, 2, 4, 8, 16, 32,           // PORTB
	1, 2, 4, 8, 16, 32, 64        // PORTC
};

const unsigned char pinAddr[] =
{
	(0x2B),
	(0x2B),
	(0x2B),
	(0x2B),
	(0x2B),
	(0x2B),
	(0x2B),
	(0x2B),
	(0x25),
	(0x25),
	(0x25),
	(0x25),
	(0x25),
	(0x25),
	(0x28),
	(0x28),
	(0x28),
	(0x28),
	(0x28),
	(0x28),
	(0x28)
};

WS2812B_LGT::LEDStrip::LEDStrip(LED* start, unsigned int size) : start(start), size(size) {};

void WS2812B_LGT::LEDStrip::writeRange(unsigned char pin, unsigned int count) {
	LED* colors=this->start;
	unsigned char port_addr = pinAddr[pin];
	unsigned char bit_mask = pinBit[pin];
	//set pin as output
	*((unsigned char *)(port_addr - 1)) |= bit_mask;
	asm volatile("cli\n");
	//Sends LEDS count times
	while(count--) {
		//Sends a color to a single LED
		
		// RMW version of code
		asm volatile(
			
			//aquire port initial data
			"ld %[pin_val], X\n"
			
			//Use immediate offset instruction so that the pointer at "colors" don't get changed and results in corrupted next pointer being processed.
			"ldd __tmp_reg__, Z+0\n"        // Read the green component
			"rcall send_led_strip_byte%=\n" // Send green component
			"ldd __tmp_reg__, Z+1\n"        // Read the red component
			"rcall send_led_strip_byte%=\n" // Send red component
			"ldd __tmp_reg__, Z+2\n"        // Read the blue component
			"rcall send_led_strip_byte%=\n" // Send blue component
			"rjmp led_strip_asm_end%=\n"    // Jump past the assembly subroutines.
			
			// send_led_strip_byte subroutine:  Sends a byte to the LED strip.
			"send_led_strip_byte%=:\n"
			"rcall send_led_strip_bit%=\n"  // Send most-significant bit (bit 7).
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"
			"rcall send_led_strip_bit%=\n"  // Send least-significant bit (bit 0).
			"ret\n"

			// send_led_strip_bit subroutine:  Sends single bit to the LED strip by driving the data line
			// high for some time.  The amount of time the line is high depends on whether the bit is 0 or 1,
			// but this function always takes the same time (2 us).
			"send_led_strip_bit%=:\n"
			
			
			
	#if F_CPU == 8000000
			"rol __tmp_reg__\n"                      // Rotate left through carry.
	#endif
			//"sbi %[port], %[pin]\n"                           // Drive the line high.
			//"sbi" instruction have to have its value determined at compile time.
			//This alternative allows the pin to be determined at run time.
			//Longer procedure might make this branch lose 8 MHz or lower clock support
			"or %[pin_val], %[msk]\n"
			"st X, %[pin_val]\n"
			"eor %[pin_val], %[msk]\n"
			
	#if F_CPU != 8000000
			"rol __tmp_reg__\n"                      // Rotate left through carry.
	#endif

	#if F_CPU == 16000000
			"nop\n" "nop\n"
	#elif F_CPU == 20000000
			"nop\n" "nop\n" "nop\n" "nop\n"
	#elif F_CPU == 32000000
			//support for LGT8F @ 32MHZ
			"nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
	#endif

			"brcs .+2\n"
			//"cbi %[port], %[pin]\n"              // If the bit to send is 0, drive the line low now.
			"st X, %[pin_val]\n"
			
	#if F_CPU == 8000000
			"nop\n" "nop\n"
	#elif F_CPU == 16000000
			"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
	#elif F_CPU == 20000000
			"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
			"nop\n" "nop\n"
	#elif F_CPU == 32000000
			//support for LGT8F @ 32MHZ
			"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
			"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
	#endif
			"brcc .+2\n"
			//"cbi %[port], %[pin]\n"              // If the bit to send is 1, drive the line low now.
			"st X, %[pin_val]\n"

	#if F_CPU == 32000000
			//support for LGT8F @ 32MHZ
			"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
			"nop\n" "nop\n" "nop\n"
	#endif

			"ret\n"
			"led_strip_asm_end%=:"
			:[msk] "+r" (bit_mask),   // pin mask
			[pin_val] "=r" (pin) // pin number and port data
			:[port] "x" (port_addr),   // port addr
			[col] "z" (colors)    // LED color address
			
			: "r0", "memory"
		);
		
		colors=colors->next;
	}
	//sei();      //re-enable interrupts
	asm volatile("sei\n");
	
}

void WS2812B_LGT::LEDStrip::write(unsigned char pin) {
	this->writeRange(pin, this->size);
}

void WS2812B_LGT::initLEDStrip(LED* lights, unsigned int size) {
	for(int i=size-1;i>=0;i--) {
		//create circular linked list of LEDs
		lights[i].next=&lights[(i+1)%size];
	}
}

#endif