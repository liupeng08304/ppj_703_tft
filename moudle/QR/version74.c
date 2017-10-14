

//#ifndef __AVR__
//#define PROGMEM
//#define memcpy_P memcpy
//#define __LPM(x) *x
//#else
//#include <avr/pgmspace.h>
//#endif
const unsigned char framebase[]  = {//PROGMEM
0xfe,0x00,0x00,0x00,0x0b,0xf8,
0x82,0x00,0x00,0x00,0x12,0x08,
0xba,0x00,0x00,0x00,0x12,0xe8,
0xba,0x00,0x00,0x00,0x1a,0xe8,
0xba,0x00,0x0f,0x80,0x3a,0xe8,
0x82,0x00,0x08,0x80,0x02,0x08,
0xfe,0xaa,0xaa,0xaa,0xab,0xf8,
0x00,0x00,0x08,0x80,0x00,0x00,
0x02,0x00,0x0f,0x80,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x0f,0x80,0x0f,0x80,0x0f,0x80,
0x08,0x80,0x08,0x80,0x08,0x80,
0x0a,0x80,0x0a,0x80,0x0a,0x80,
0x08,0x80,0x08,0x80,0x08,0x80,
0x0f,0x80,0x0f,0x80,0x0f,0x80,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,
0x0a,0x00,0x00,0x00,0x00,0x00,
0x78,0x00,0x00,0x00,0x00,0x00,
0x9a,0x00,0x0f,0x80,0x0f,0x80,
0x00,0x80,0x08,0x80,0x08,0x80,
0xfe,0x00,0x0a,0x80,0x0a,0x80,
0x82,0x00,0x08,0x80,0x08,0x80,
0xba,0x00,0x0f,0x80,0x0f,0x80,
0xba,0x00,0x00,0x00,0x00,0x00,
0xba,0x00,0x00,0x00,0x00,0x00,
0x82,0x00,0x00,0x00,0x00,0x00,
0xfe,0x00,0x00,0x00,0x00,0x00,
};

const unsigned char framask[] = {
0xff,0xff,0xff,0xff,0xff,0xf8,
0x10,0x04,0x00,0x80,0x08,0x00,
0x40,0x01,0x00,0x02,0x00,0x02,
0x00,0x01,0x00,0x00,0x40,0x00,
0x08,0x00,0x03,0xe0,0x02,0x1f,
0x00,0x18,0x7c,0x00,0x70,0xf8,
0x00,0xf0,0xf8,0x00,0xf8,0x10,
0x00,0x00,0x04,0x00,0x00,0x00,
0x80,0x00,0x00,0x08,0x00,0x00,
0x00,0x40,0x00,0x00,0x01,0x00,
0x00,0x00,0x02,0x00,0x00,0x00,
0x02,0x00,0x00,0x00,0x01,0x00,
0x00,0x00,0x1f,0xc0,0x00,0x00,
0x03,0xf8,0x00,0x00,0x00,0x3f,
0x80,0x03,0xe0,0x03,0xff,0x00,
0x1f,0x00,0x1f,0xfc,0x00,0x7c,
0x00,0x7f,0xf8,0x00,0xf8,0x00,
0xff,0xf8,0x00,0xf8,0x00,0xff,
0xfc,0x00,0x00,0x00,0x01,0xff,
0x00,0x00,0x00,0x00,0x3f,0xe0,
0x00,0x00,0x00,0x03,0xfe,0x00,
0x00,0x00,0x00,0x00,
};
