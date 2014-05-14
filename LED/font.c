//#include "font.h"

const unsigned char FontLookup [][5] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },  // sp 00
    { 0x00, 0x00, 0x2f, 0x00, 0x00 },   // ! 01
    { 0x00, 0x07, 0x00, 0x07, 0x00 },   // " 02
    { 0x14, 0x7f, 0x14, 0x7f, 0x14 },   // # 03 
    { 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   // $ 04
    { 0xc4, 0xc8, 0x10, 0x26, 0x46 },   // % 05 
    { 0x36, 0x49, 0x55, 0x22, 0x50 },   // & 06
    { 0x00, 0x05, 0x03, 0x00, 0x00 },   // ' 07
    { 0x00, 0x1c, 0x22, 0x41, 0x00 },   // ( 08
    { 0x00, 0x41, 0x22, 0x1c, 0x00 },   // ) 09
    { 0x14, 0x08, 0x3E, 0x08, 0x14 },   // * 10
    { 0x08, 0x08, 0x3E, 0x08, 0x08 },   // + 11
    { 0x00, 0x00, 0x50, 0x30, 0x00 },   // , 12
    { 0x10, 0x10, 0x10, 0x10, 0x10 },   // - 13
    { 0x00, 0x60, 0x60, 0x00, 0x00 },   // . 14
    { 0x20, 0x10, 0x08, 0x04, 0x02 },   // / 15
    { 0x3E, 0x51, 0x49, 0x45, 0x3E },   // 0 16
    { 0x00, 0x42, 0x7F, 0x40, 0x00 },   // 1 17
    { 0x42, 0x61, 0x51, 0x49, 0x46 },   // 2 18
    { 0x21, 0x41, 0x45, 0x4B, 0x31 },   // 3 19
    { 0x18, 0x14, 0x12, 0x7F, 0x10 },   // 4 20
    { 0x27, 0x45, 0x45, 0x45, 0x39 },   // 5 21
    { 0x3C, 0x4A, 0x49, 0x49, 0x30 },   // 6 22
    { 0x01, 0x71, 0x09, 0x05, 0x03 },   // 7 23 
    { 0x36, 0x49, 0x49, 0x49, 0x36 },   // 8 24
    { 0x06, 0x49, 0x49, 0x29, 0x1E },   // 9 25
    { 0x00, 0x36, 0x36, 0x00, 0x00 },   // : 26
    { 0x00, 0x56, 0x36, 0x00, 0x00 },   // ; 27
    { 0x08, 0x14, 0x22, 0x41, 0x00 },   // < 28
    { 0x14, 0x14, 0x14, 0x14, 0x14 },   // = 29
    { 0x00, 0x41, 0x22, 0x14, 0x08 },   // > 30
    { 0x02, 0x01, 0x51, 0x09, 0x06 },   // ? 31
    { 0x32, 0x49, 0x59, 0x51, 0x3E },   // @ 32
    { 0x7E, 0x11, 0x11, 0x11, 0x7E },   // A 33
    { 0x7F, 0x49, 0x49, 0x49, 0x36 },   // B 34
    { 0x3E, 0x41, 0x41, 0x41, 0x22 },   // C 35
    { 0x7F, 0x41, 0x41, 0x22, 0x1C },   // D 36
    { 0x7F, 0x49, 0x49, 0x49, 0x41 },   // E 37
    { 0x7F, 0x09, 0x09, 0x09, 0x01 },   // F 38
    { 0x3E, 0x41, 0x49, 0x49, 0x7A },   // G 39
    { 0x7F, 0x08, 0x08, 0x08, 0x7F },   // H 40
    { 0x00, 0x41, 0x7F, 0x41, 0x00 },   // I 41
    { 0x20, 0x40, 0x41, 0x3F, 0x01 },   // J 42
    { 0x7F, 0x08, 0x14, 0x22, 0x41 },   // K 43
    { 0x7F, 0x40, 0x40, 0x40, 0x40 },   // L 44
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   // M 45
    { 0x7F, 0x04, 0x08, 0x10, 0x7F },   // N 46
    { 0x3E, 0x41, 0x41, 0x41, 0x3E },   // O 47
    { 0x7F, 0x09, 0x09, 0x09, 0x06 },   // P 48
    { 0x3E, 0x41, 0x51, 0x21, 0x5E },   // Q 49
    { 0x7F, 0x09, 0x19, 0x29, 0x46 },   // R 50
    { 0x46, 0x49, 0x49, 0x49, 0x31 },   // S 51
    { 0x01, 0x01, 0x7F, 0x01, 0x01 },   // T 52
    { 0x3F, 0x40, 0x40, 0x40, 0x3F },   // U 53
    { 0x1F, 0x20, 0x40, 0x20, 0x1F },   // V 54
    { 0x3F, 0x40, 0x38, 0x40, 0x3F },   // W 55
    { 0x63, 0x14, 0x08, 0x14, 0x63 },   // X 56
    { 0x07, 0x08, 0x70, 0x08, 0x07 },   // Y 57
    { 0x61, 0x51, 0x49, 0x45, 0x43 },   // Z 58
    { 0x00, 0x7F, 0x41, 0x41, 0x00 },   // [ 59
    { 0x55, 0x2A, 0x55, 0x2A, 0x55 },   //55 60
    { 0x00, 0x41, 0x41, 0x7F, 0x00 },   // ] 61
    { 0x04, 0x02, 0x01, 0x02, 0x04 },   // ^ 62
    { 0x40, 0x40, 0x40, 0x40, 0x40 },   // _ 63
    { 0x00, 0x01, 0x02, 0x04, 0x00 },   // ' 64
    { 0x20, 0x54, 0x54, 0x54, 0x78 },   // a 65
    { 0x7F, 0x48, 0x44, 0x44, 0x38 },   // b 66
    { 0x38, 0x44, 0x44, 0x44, 0x20 },   // c 67
    { 0x38, 0x44, 0x44, 0x48, 0x7F },   // d 68
    { 0x38, 0x54, 0x54, 0x54, 0x18 },   // e 69
    { 0x08, 0x7E, 0x09, 0x01, 0x02 },   // f 70
    { 0x0C, 0x52, 0x52, 0x52, 0x3E },   // g 71
    { 0x7F, 0x08, 0x04, 0x04, 0x78 },   // h 72
    { 0x00, 0x44, 0x7D, 0x40, 0x00 },   // i 73
    { 0x20, 0x40, 0x44, 0x3D, 0x00 },   // j 74
    { 0x7F, 0x10, 0x28, 0x44, 0x00 },   // k 75
    { 0x00, 0x41, 0x7F, 0x40, 0x00 },   // l 76
    { 0x7C, 0x04, 0x18, 0x04, 0x78 },   // m 77
    { 0x7C, 0x08, 0x04, 0x04, 0x78 },   // n 78
    { 0x38, 0x44, 0x44, 0x44, 0x38 },   // o 79
    { 0x7C, 0x14, 0x14, 0x14, 0x08 },   // p 80
    { 0x08, 0x14, 0x14, 0x18, 0x7C },   // q 81
    { 0x7C, 0x08, 0x04, 0x04, 0x08 },   // r 82
    { 0x48, 0x54, 0x54, 0x54, 0x20 },   // s 83
    { 0x04, 0x3F, 0x44, 0x40, 0x20 },   // t 84
    { 0x3C, 0x40, 0x40, 0x20, 0x7C },   // u 85
    { 0x1C, 0x20, 0x40, 0x20, 0x1C },   // v 86
    { 0x3C, 0x40, 0x30, 0x40, 0x3C },   // w 87
    { 0x44, 0x28, 0x10, 0x28, 0x44 },   // x 88
    { 0x0C, 0x50, 0x50, 0x50, 0x3C },   // y 89
    { 0x44, 0x64, 0x54, 0x4C, 0x44 },   // z 90
    { 0x08, 0x6C, 0x6A, 0x19, 0x08 },   // { 91
    { 0x30, 0x4E, 0x61, 0x4E, 0x30 },   // | 92
    { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }    // # 93   
};

// Return pointer to font character
unsigned char *fontPtr(char c)
{
  if ( c < ' ' || c-' ' > 93  )
    return (unsigned char*)FontLookup[0];
  else
    return (unsigned char*)FontLookup[(unsigned char)(c-' ')]; 
}