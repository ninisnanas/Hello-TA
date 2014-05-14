#include "font.h"
#include "demo_cfg.h"
#include "msp430.h"
#include <stdint.h>
#include <string.h>

uint8_t test_array[8 * NUM_OF_LED_MATRIXES];
uint8_t text_array[1000];
char *str = "Tesss";
char *long_str = "Hai, apa kabarmu?";
char *clk_text = "00:00";
int data_size = 0;
  
void delay_ms(unsigned int ms);
int lookup(char a);
void update_image(uint8_t *array);

/**************************************************************************//**
*
* spi_send_bit_bang
*
* @brief      send a 8 bit (one byte) SPI data with bit-banging
*
* @param      byte     data to be sent
*
* @return     -
*
******************************************************************************/
void spi_send_bit_bang(uint8_t byte)
{
  uint8_t i;

  for(i=0 ; i<8 ; i++)
  {
	// send LSB
	if(byte & 0x01)
	{
	  P1OUT |= SPI_DATA_OUT_PIN;
	}
	else
	{
	  P1OUT &= ~SPI_DATA_OUT_PIN;
	}

	// delay
	SPI_BIT_BANGING_DELAY();

	// set clock
	P1OUT |= SPI_CLK_OUT_PIN;

	// delay
	SPI_BIT_BANGING_DELAY();

	// reset clock
	P1OUT &= ~SPI_CLK_OUT_PIN;

	// shift right byte
	byte >>= 1;
  }

  // reset SPI data line
  P1OUT &= ~SPI_DATA_OUT_PIN;
}

/**************************************************************************//**
*
* led_matrix_output_latch
*
* @brief      set output latch
*
* @param      -
*
* @return     -
*
******************************************************************************/
void led_matrix_output_latch(void)
{
  // set output latch
  P1OUT |= DATA_LATCH_PIN;
  P1OUT &= ~DATA_LATCH_PIN;
}

/**************************************************************************//**
*
* led_matrix_send_row
*
* @brief      send data to a LED matrix row
*
* @param      row     row number (0-7)
* @param      data    8 bit LED matrix row data
*
* @return     -
*
******************************************************************************/
void led_matrix_send_row(uint8_t row, uint8_t data)
{
  static const uint16_t row_byte[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
  
  // send first byte (data byte)
  spi_send_bit_bang(data);

  // send second byte (row data)
  spi_send_bit_bang(row_byte[(row&0x07)]);
}

/**************************************************************************//**
*
* send_image
*
* @brief      send image to the LED matrix/matrices
*
* @param      running_text_data     data to be put into LED
*
* @return     -
*
******************************************************************************/
void send_image(uint8_t *text_data, int size)
{
  static uint16_t first_idx = 0;
  static uint8_t row = 0;
  uint16_t i, j;
  
  // start continuous conversion
  ADC10CTL0 = REFON + MSC + SREF_1 + ADC10SHT_2 + ADC10ON + ADC10IE;
  ADC10CTL1 = INCH_0 + CONSEQ_2 + ADC10DIV_7 + ADC10SSEL_3;
  
  // start continuous conversions
  ADC10CTL0 |= ENC + ADC10SC;
  
  j = first_idx + row;
  for(i=0 ; i<NUM_OF_LED_MATRIXES ; i++)
  {
    if(j >= size)
    {
      j = j - size;
    }

    // send data
    led_matrix_send_row(row, text_data[j]);

    // go to next led matrix data
    j += 8;
  }

  // set output latch
  led_matrix_output_latch();

  // increment row
  row++;
  if(row >= 8) row = 0;
}

/**************************************************************************//**
*
* send_image
*
* @brief      send image to the LED matrix/matrices
*
* @param      running_text_data     data to be put into LED
*
* @return     -
*
******************************************************************************/
void send_scrolling_text(uint8_t *text_data, int size)
{
  static uint16_t first_idx = 0;
  static uint8_t row = 0;
  uint16_t i, j;
  
  // start continuous conversion
  ADC10CTL0 = REFON + MSC + SREF_1 + ADC10SHT_2 + ADC10ON + ADC10IE;
  ADC10CTL1 = INCH_0 + CONSEQ_2 + ADC10DIV_7 + ADC10SSEL_3;
  
  // start continuous conversions
  ADC10CTL0 |= ENC + ADC10SC;
 
  j = first_idx + row;
  for(i=0 ; i<NUM_OF_LED_MATRIXES ; i++)
  {
    if(j >= size)
    {
      j = j - size;
    }

    // send data
    led_matrix_send_row(row, text_data[j]);

    // go to next led matrix data
    j += 8;
  }

  // set output latch
  led_matrix_output_latch();

  // increment row
  row++;
  if(row >= 8) row = 0;
}

void led_init()
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  // configure SPI pins and latch
  P1DIR |= DATA_LATCH_PIN | SPI_DATA_OUT_PIN | SPI_CLK_OUT_PIN;
  
  // set latch pin to inactive
  P1OUT &= ~DATA_LATCH_PIN;
  
  // set both GPIO for SPI communication low
  P1OUT &= ~(SPI_DATA_OUT_PIN | SPI_CLK_OUT_PIN);
}

void write_to_led(char *str, int type)
{
  int word = strlen(str);
  int i = 0;
  
  data_size = word * 5 + word;
  
  int counter = 1;
  int a = 0;
  
  if (type == TEXT)
  {
    for (i = 0; i < data_size; i++)
    {
      if (counter % 6 == 0)
      {
        test_array[i] = 0x00;
        counter = 1;
        a++;
      } else
      {
        test_array[i] = FontLookup[lookup(str[a])][(counter - 1) % 5];
        counter++;
      }
    }
  } else 
  {
    data_size = 8 * NUM_OF_LED_MATRIXES;
    counter = 1;
    a = 0;
    test_array[0] = 0x00;
    test_array[1] = 0x00;
    for (i = 2; i < data_size; i++)
    {
      if (counter % 6 == 0)
      {
        test_array[i] = 0x00;
        counter = 1;
        a++;
      } else
      {
        if (a == 2 && counter == 5)
        {
          test_array[i] = FontLookup[lookup(str[a])][(counter - 1) % 5];
          counter = 1;
          a++;
        } else 
        {
          test_array[i] = FontLookup[lookup(str[a])][(counter - 1) % 5];
          counter++;
        }
      }
    }
    test_array[data_size - 1] = 0x00;
  }
}

void update_image(uint8_t *array)
{
  int i = 0;
  uint8_t temp = array[0];
  for (i = 0; i < data_size - 1; i++)
  {
    array[i] = array[i + 1];
  }
  array[data_size] = temp;
}

/*int main( void )
{
  led_init(); 
  
  write_to_led(clk_text, CLOCK);
  //write_to_led(long_str, TEXT);
  
  while(1) {
    sendImage(test_array, data_size);
    //send_scrolling_text(test_array, data_size);
  };
}*/

int lookup(char a) {
  if (a == ' ') return 0;
  else if (a == '!') return 1;
  else if (a == '"') return 2;
  else if (a == '#') return 3;
  else if (a == '$') return 4;
  else if (a == '%') return 5;
  else if (a == '&') return 6;
  else if (a == '\'') return 7;
  else if (a == '(') return 8;
  else if (a == ')') return 9;
  else if (a == '*') return 10;
  else if (a == '+') return 11;
  else if (a == ',') return 12;
  else if (a == '-') return 13;
  else if (a == '.') return 14;
  else if (a == '/') return 15;
  else if (a == '0') return 16;
  else if (a == '1') return 17;
  else if (a == '2') return 18;
  else if (a == '3') return 19;
  else if (a == '4') return 20;
  else if (a == '5') return 21;
  else if (a == '6') return 22;
  else if (a == '7') return 23;
  else if (a == '8') return 24;
  else if (a == '9') return 25;
  else if (a == ':') return 26;
  else if (a == ';') return 27;
  else if (a == '<') return 28;
  else if (a == '=') return 29;
  else if (a == '>') return 30;
  else if (a == '?') return 31;
  else if (a == '@') return 32;
  else if (a == 'A') return 33;
  else if (a == 'B') return 34;
  else if (a == 'C') return 35;
  else if (a == 'D') return 36;
  else if (a == 'E') return 37;
  else if (a == 'F') return 38;
  else if (a == 'G') return 39;
  else if (a == 'H') return 40;
  else if (a == 'I') return 41;
  else if (a == 'J') return 42;
  else if (a == 'K') return 43;
  else if (a == 'L') return 44;
  else if (a == 'M') return 45;
  else if (a == 'N') return 46;
  else if (a == 'O') return 47;
  else if (a == 'P') return 48;
  else if (a == 'Q') return 49;
  else if (a == 'R') return 50;
  else if (a == 'S') return 51;
  else if (a == 'T') return 52;
  else if (a == 'U') return 53;
  else if (a == 'V') return 54;
  else if (a == 'W') return 55;
  else if (a == 'X') return 56;
  else if (a == 'Y') return 57;
  else if (a == 'Z') return 58;
  else if (a == '[') return 59;
  else if (a == ']') return 61;
  else if (a == '^') return 62;
  else if (a == '_') return 63;
  else if (a == '`') return 64;
  else if (a == 'a') return 65;
  else if (a == 'b') return 66;
  else if (a == 'c') return 67;
  else if (a == 'd') return 68;
  else if (a == 'e') return 69;
  else if (a == 'f') return 70;
  else if (a == 'g') return 71;
  else if (a == 'h') return 72;
  else if (a == 'i') return 73;
  else if (a == 'j') return 74;
  else if (a == 'k') return 75;
  else if (a == 'l') return 76;
  else if (a == 'm') return 77;
  else if (a == 'n') return 78;
  else if (a == 'o') return 79;
  else if (a == 'p') return 80;
  else if (a == 'q') return 81;
  else if (a == 'r') return 82;
  else if (a == 's') return 83;
  else if (a == 't') return 84;
  else if (a == 'u') return 85;
  else if (a == 'v') return 86;
  else if (a == 'w') return 87;
  else if (a == 'x') return 88;
  else if (a == 'y') return 89;
  else if (a == 'z') return 90;
  else if (a == '{') return 91;
  else if (a == '|') return 92;
  else if (a == '#') return 93;
  else return 60;
}