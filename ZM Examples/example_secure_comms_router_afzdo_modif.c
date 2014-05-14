/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_basic_comms_end_device_afzdo.c
 *
 * @brief Resets Radio, configures this device to be a Zigbee End Device, joins a network,
 * then sends a message to the coordinator every few seconds. Sleeps inbetween transmissions.
 * Will also transmit a message when the button is pressed.
 *
 * Calibrates the Very-Low-Power Oscillator (VLO) in the MSP430. Uses VLO to wakup the processor.
 * Note: you could also use an external 32kHz xtal for more accuracy/stability
 *
 * Uses the AF/ZDO interface.
 *
 * This matches example_basic_communications_coordinator.c
 * Get the coordinator running first, or else the end device won't have anything to join to.
 *
 * $Rev: 1642 $
 * $Author: dsmith $
 * $Date: 2012-10-31 11:17:04 -0700 (Wed, 31 Oct 2012) $
 *
 * @section support Support
 * Please refer to the wiki at www.anaren.com/air-wiki-zigbee for more information. Additional support
 * is available via email at the following addresses:
 * - Questions on how to use the product: AIR@anaren.com
 * - Feature requests, comments, and improvements:  featurerequests@teslacontrols.com
 * - Consulting engagements: sales@teslacontrols.com
 *
 * @section license License
 * Copyright (c) 2012 Tesla Controls. All rights reserved. This Software may only be used with an 
 * Anaren A2530E24AZ1, A2530E24CZ1, A2530R24AZ1, or A2530R24CZ1 module. Redistribution and use in 
 * source and binary forms, with or without modification, are subject to the Software License 
 * Agreement in the file "anaren_eula.txt"
 * 
 * YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS” 
 * WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO 
 * EVENT SHALL ANAREN MICROWAVE OR TESLA CONTROLS BE LIABLE OR OBLIGATED UNDER CONTRACT, NEGLIGENCE, 
 * STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR 
 * INDIRECT DAMAGES OR EXPENSE INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, 
 * PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE 
 * GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY 
 * DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 */

/*#include "../HAL/hal.h"
#include "../ZM/module.h"
#include "../ZM/module_errors.h"
#include "../ZM/module_utilities.h"
#include "../ZM/af.h"
#include "../ZM/zdo.h"
#include "module_example_utils.h"*/
   
#include "../HAL/hal.h"
#include "../Common/utilities.h"
#include "../ZM/module.h"
#include "../ZM/af.h"
#include "../ZM/zdo.h"
#include "../ZM/module_errors.h"
#include "../ZM/module_utilities.h"
#include "../ZM/zm_phy.h"
#include "module_example_utils.h"
#include <stdint.h>
#include <stddef.h>

/** function pointer (in hal file) for the function that gets called when the timer generates an int*/
extern void (*timerIsr)(void);

/** function pointer (in hal file) for the function that gets called when a button is pressed*/
extern void (*buttonIsr)(int8_t);

/** Used to store return value from module operations */
moduleResult_t result;

/** Handles timer interrupt */
void handleTimer();

/** Handles button interrupt */
void handleButtonPress(int8_t whichButton);

void parseMessageAndDisplay(uint8_t receivedMessage[], int msgSize);
#define RESTART_AFTER_ZM_FAILURE

uint8_t counter = 0;
//uint8_t testMessage[10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x30, 0x31, 0x32, 0x33, 0x34};
uint8_t testMessage[4] = {0x30, 0x31, 0x32, 0x33};
//0x34, 0x30, 0x31, 0x32, 0x33, 0x34};
uint8_t receivedMessage[];

/* This example may be used to measure the module's sleep current. To do this, first define the 
compile option DISABLE_END_DEVICE_POLLING. In IAR, this can be done in Project Options : C/C++ compiler : Preprocessor
In the defined symbols box, add:
DISABLE_END_DEVICE_POLLING
This will prevent the end device from waking up every second or so and checking for any new messages.
This must be defined project-wide because it affects other files. It will not allow the end device 
to receive messages though. Second, undefine SEND_MESSAGE_ON_TIMER in this file to not use the timer. 
This will stop the module from communicating and will allow you to measure sleep current or when you 
want to manually wake up (e.g. on a button). */

#define SEND_MESSAGE_ON_TIMER
extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

int main( void )
{
    halInit();
    moduleInit();     
    printf("\r\n****************************************************\r\n");    
    printf("Basic Communications Example - END DEVICE - using AFZDO\r\n");
    /* Configure the buttonIsr (called from the hal file) to point to our handleButtonPress() */
    buttonIsr = &handleButtonPress;   
    halSetWakeupFlags(WAKEUP_AFTER_BUTTON);

    /** Calibrate the Very Low Power Oscillator (VLO) to use for sleep timing */
    printf("Calibrating VLO...\r\n");

    int16_t vlo = -1;
    while (vlo < 0)
    {
        vlo = calibrateVlo();
        printf("VLO = %u Hz; ", vlo);
    }
    printf(" Done.\r\n");
    timerIsr = &handleTimer; 
    HAL_ENABLE_INTERRUPTS();   

#define MODULE_START_DELAY_IF_FAIL_MS 5000
    /* Use the default module configuration */

    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_END_DEVICE;
    /* Change this if you are using a custom PAN */
    defaultConfiguration.panId = ANY_PAN;
    struct applicationConfiguration ac;
   
    /* Turn Off nwk status LED if on */
    clearLed(ON_NETWORK_LED);
    /* Loop until module starts */      
    while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
    {
        /* Module startup failed; display error and blink LED */
        setLed(NETWORK_FAILURE_LED);                    
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);

        delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);                    
        clearLed(NETWORK_FAILURE_LED);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);
    }
    printf("On Network!\r\n");
    setLed(ON_NETWORK_LED);  // Indicate we got on the network

    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();                
    displayDeviceInformation();
#else
    displayBasicDeviceInformation();
#endif  

#define TEST_CLUSTER 0x77

uint16_t shortAddress;
uint8_t addressToSend[2];
    /* get the end point short address */
    result = zbGetDeviceInfo(DIP_SHORT_ADDRESS);
    shortAddress = CONVERT_TO_INT(zmBuf[SRSP_DIP_VALUE_FIELD] , zmBuf[SRSP_DIP_VALUE_FIELD+1]);
    printf("short address is %04X\r\n", shortAddress);
    
    // break the short address into two 8 bits
    addressToSend[0] = shortAddress;
    addressToSend[1] = (uint8_t)(shortAddress >> 8);
    
    printf("address to send is: %02X %02X", addressToSend[0], addressToSend[1]);
    
    /* Register the endpoint to coordinator */
    /* Indicate that we are sending a message */
        setLed(SEND_MESSAGE_LED);
        printf("Sending Register Message");
        
        /* Attempt to send the message */
        result = afSendData(DEFAULT_ENDPOINT,DEFAULT_ENDPOINT,0, TEST_CLUSTER, addressToSend, 2);
        clearLed(SEND_MESSAGE_LED);
        if (result == MODULE_SUCCESS)
        {
            printf("Success\r\n");
        } else {
            printf("ERROR %02X ", result);
        }

    /* Now the network is running - send a message to the coordinator every few seconds.*/


#define CLOCK_CLUSTER 0x55
#define MESSAGE_CLUSTER 0x66
#define CLUSTER_INDEX 5
#define MESSAGE_INDEX 20
#define MESSAGE_INFO_LENGTH 19
#define CLOCK_INFO_LENGTH 4
    
/*
#ifdef SEND_MESSAGE_ON_TIMER
    int16_t timerResult = initTimer(4);
    if (timerResult != 0)
    {
        printf("timerResult Error %i, STOPPING\r\n", timerResult);
        while (1);   
    }
#endif*/

    while (1)
    {
      if (MODULE_HAS_MESSAGE_WAITING())  {
        //pollAndDisplay();
        getMessage();
        if (zmBuf[SRSP_LENGTH_FIELD] > 0)
        {
          // find out what printHexBytes does
          /*
          printHexBytes(zmBuf, (zmBuf[SRSP_LENGTH_FIELD] + SRSP_HEADER_SIZE));
          zmBuf[SRSP_LENGTH_FIELD] = 0;
          clearLeds();
          */
          // misalnya message udah berhasil diambil, saatnya ditranslate
          if (zmBuf[CLUSTER_INDEX] == 0x55) 
          {
            printf("this is a clock\r\n");
            // the message contains clock information
            // the length of clock information message is always 4 (HH:MM)
            int i;
            for (i = 0; i < CLOCK_INFO_LENGTH; i++)
            {
              receivedMessage[i] = zmBuf[MESSAGE_INDEX + i];
            }
            printf("%c%c:%c%c\r\n", receivedMessage[0], receivedMessage[1], receivedMessage[2], receivedMessage[3]);
            // call to method to parse & update the clock
            int a = 5;
            parseMessageAndDisplay(receivedMessage, a);
          } else if (zmBuf[CLUSTER_INDEX] == 0x66)
          {
            printf("this is a string\r\n");
            int i;
            for (i = 0; i < MESSAGE_INFO_LENGTH; i++)
            {
              receivedMessage[i] = zmBuf[MESSAGE_INDEX + i];
              printf("%c", receivedMessage[i]);
            }
            printf("\r\n");
          }
          zmBuf[SRSP_LENGTH_FIELD] = 0;
        }
      }
        
      
            
        /* Indicate that we are sending a message */
        /*setLed(SEND_MESSAGE_LED);
        printf("Sending Message %u  ", counter++);
        /* Attempt to send the message */
        /*result = afSendData(DEFAULT_ENDPOINT,DEFAULT_ENDPOINT,0, TEST_CLUSTER, testMessage, 5);
        clearLed(SEND_MESSAGE_LED);
        if (result == MODULE_SUCCESS)
        {
            printf("Success\r\n");
        } else {
            printf("ERROR %02X ", result);*/
/*#ifdef RESTART_AFTER_ZM_FAILURE
            printf("\r\nRestarting\r\n");
            goto start;
#else
            printf("stopping\r\n");
            while(1);
#endif
        }  */    
        //HAL_SLEEP();
    }
}

void handleTimer()  //Will automatically wakeup timer
{
    printf("#");
}

void handleButtonPress(int8_t whichButton)
{
    toggleLed(0);
    printf("Button Pressed\r\n");
}

void parseMessageAndDisplay(uint8_t receivedMessage[], int msgSize)
{
  int i;
  for (i = 0; i < msgSize; i++)
  {
    int msgToParse = receivedMessage[i];
    /*switch(msgToParse)
    {
      case 
    }*/
  }
}

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
  else if (a == '55') return 60;
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

/* @} */
