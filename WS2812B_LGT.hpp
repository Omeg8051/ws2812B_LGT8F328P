#ifndef WS2812B_LGT_H
#define WS2812B_LGT_H

#include <stdint.h>

extern const unsigned char pinBit[];
extern const unsigned char pinAddr[];


namespace WS2812B_LGT {
    struct LED {
            uint8_t green;
            uint8_t red;
            uint8_t blue;
            struct LED* next;
            LED() {};
            LED(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {};
        };
    void initLEDStrip(LED* lights, unsigned int size); //expects lights to be array at least as big as size
    class LEDStrip {

        public:
            LED* start;
            unsigned int size;
            LEDStrip(LED* start, unsigned int size);
            void write(unsigned char pin);
            void writeRange(unsigned char pin, unsigned int count);
    };
};
#endif