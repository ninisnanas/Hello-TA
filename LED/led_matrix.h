#define DATA_LATCH_PIN          BIT4
#define SPI_CLK_OUT_PIN         BIT5
#define SPI_DATA_OUT_PIN        BIT6
#define SPI_BIT_BANGING_DELAY() __delay_cycles(1)
#define SCROLLING_DELAY()       __delay_cycles(16000)
#define PUSH_BUTTON 0x80

#define NUM_OF_LED_MATRIXES             (4)

#define CLOCK 0
#define TEXT 1

#define CHARNUM 6

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
void spi_send_bit_bang(uint8_t byte);

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
void led_matrix_output_latch(void);

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
void led_matrix_send_row(uint8_t row, uint8_t data);

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
void send_image(uint8_t *text_data, int size);

void led_init();

void write_to_led(char *str, int type);

int lookup(char a);