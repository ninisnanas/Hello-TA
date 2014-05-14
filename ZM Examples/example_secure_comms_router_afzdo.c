/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_secure_comms_router_afzdo.c
 *
 * @brief Resets Radio, configures this device to be a Zigbee Router, joins a network, initializes
 * security and then sends a message to the coordinator once per second.
 * Uses the AF/ZDO interface.
 *
 * This matches example_secure_communications_COORDINATOR.c
 * Get the coordinator running first, or else the router won't have anything to join to.
 *
 * Note: security mode should be the same for all devices on the network and can be one of:
- SECURITY_MODE_PRECONFIGURED_KEYS
- USE_SECURITY_MODE_COORD_DIST_KEYS
- SECURITY_MODE_OFF
 *
 * $Rev: 1770 $
 * $Author: dsmith $
 * $Date: 2013-03-07 14:55:49 -0800 (Thu, 07 Mar 2013) $
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

#include "../HAL/hal.h"
#include "../Common/utilities.h"
#include "../ZM/module.h"
#include "../ZM/module_errors.h"
#include "../ZM/module_utilities.h"
#include "../ZM/af.h"
#include "../ZM/zdo.h"
#include "../ZM/zm_phy.h"
#include "../LED/led_matrix.h"
#include "../LED/font.h"
#include "module_example_utils.h"   //for handleReturnValue() and polling()
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/** function pointer (in hal file) for the function that gets called when the timer generates an int*/
extern void (*timerIsr)(void);

extern uint8_t text_array[100];

/** Used to store return value from module operations */
moduleResult_t result;

/** Handles timer interrupt */
void handleTimer();

/** Method to parse the message received and display it to LED */
void parseMessageAndDisplay(uint8_t receivedMessage[], int msgSize);

#define RESTART_AFTER_ZM_FAILURE

/** encryption key used in security */
uint8_t key[16] = "*Tesla Controls*";

unsigned char counter = 0;
unsigned char testMessage[] = {0xE0,0xE1,0xE2,0xE3,0xE4};
unsigned char receivedMessage[];

int h1 = 48; // in ASCII
int h0 = 48;
int m1 = 48;
int m0 = 48;

/* This example may be used to measure the module's sleep current. To do this, first define the 
compile option DISABLE_END_DEVICE_POLLING. In IAR, this can be done in Project Options : C/C++ compiler : Preprocessor
In the defined symbols box, add:
DISABLE_END_DEVICE_POLLING
This will prevent the end device from waking up every second or so and checking for any new messages.
This must be defined project-wide because it affects other files. It will not allow the end device 
to receive messages though. Second, undefine SEND_MESSAGE_ON_TIMER in this file to not use the timer. 
This will stop the module from communicating and will allow you to measure sleep current or when you 
want to manually wake up (e.g. on a button). */

/* ZigBee buffer that holds incoming message */
extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

uint8_t text_arr[100];
extern int data_size;

void increase_counter();

char str1[6];

int main( void )
{
    //halInit();
    //moduleInit();    
    printf("\r\n****************************************************\r\n");    
    printf("Secure Communications (Modified) - END DEVICE - using AFZDO\r\n");
    
    led_init();
    
    /** Calibrate the Very Low Power Oscillator (VLO) to use for sleep timing */
    printf("Calibrating VLO...\r\n");

    /*
    int16_t vlo = -1;
    while (vlo < 0)
    {
        vlo = calibrateVlo();
        printf("VLO = %u Hz; ", vlo);
    }
    printf(" Done.\r\n");
    /*
    timerIsr = &handleTimer;
    int16_t timerResult = initTimer(1);
    if (timerResult != 0)
    {
      printf("timerResult Error %i, STOPPING\r\n", timerResult);
      while(1);
    }
    */
    //HAL_ENABLE_INTERRUPTS();
    
    str1[0] = (char) h1;
    str1[1] = (char) h0;
    str1[2] = (char) ':';
    str1[3] = (char) m1;
    str1[4] = (char) m0;
    str1[5] = (char) '\0';
    
    int counter = 0;
    while(1)
    {
      if (counter == 246) 
      {
        str1[2] = (char) ' ';
        increase_counter();
        write_to_led(str1, CLOCK);
      } else 
        if (counter == 491)
        {
          str1[2] = (char) ':';
          write_to_led(str1, CLOCK);
          counter = 0;
        }
      send_image(text_array, data_size);
      counter++;
    }

#define MODULE_START_DELAY_IF_FAIL_MS 5000

    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_END_DEVICE;
    defaultConfiguration.securityMode = SECURITY_MODE_PRECONFIGURED_KEYS;
    defaultConfiguration.securityKey = key;
    
    /*
    struct applicationConfiguration endDeviceConf;
    endDeviceConf.endPoint = 1;
    endDeviceConf.latencyRequested = LATENCY_NORMAL;
    endDeviceConf.profileId = 0xcc00; // the clock profile is 0xCC00
    endDeviceConf.deviceId = 0x8866;
    endDeviceConf.deviceVersion = 0x01;
    endDeviceConf.numberOfBindingInputClusters = 0; // number of binding input cluster
    endDeviceConf.bindingInputClusters[0] = 0x0000; // basic cluster
    endDeviceConf.bindingInputClusters[1] = 0xfc01; // identify cluster
    endDeviceConf.bindingInputClusters[2] = 0xfc02; // synchronise clock cluster
    endDeviceConf.bindingInputClusters[3] = 0xfc03; // send string message cluster
    endDeviceConf.numberOfBindingOutputClusters = 0; // number of binding output cluster
    endDeviceConf.bindingOutputClusters[0] = 0x0000;
    endDeviceConf.bindingOutputClusters[1] = 0xfc01;
    endDeviceConf.bindingOutputClusters[2] = 0xfc02;
    endDeviceConf.bindingOutputClusters[3] = 0xfc03;*/
    
    /* Turn Off nwk status LED if on */
    clearLed(ON_NETWORK_LED);
    /* Loop until module starts */
    while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
    //while ((result = startModule(&defaultConfiguration, &endDeviceConf)) != MODULE_SUCCESS)
    {
        /* Module startup failed; display error and blink LED */
        setLed(NETWORK_FAILURE_LED);
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);
        clearLed(NETWORK_FAILURE_LED);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);
    }

    printf("On Network!\r\n");
    setLed(ON_NETWORK_LED); // Indicate we got on the network

    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();
    displayDeviceInformation();
#else
    displayBasicDeviceInformation();
#endif  

    /* Now the network is running - send a message to the coordinator every few seconds.*/
#define REGISTER_CLUSTER 0xfc01
#define CLOCK_CLUSTER 0xfc02
#define MESSAGE_CLUSTER 0xfc03
#define CLUSTER_INDEX 5
#define MESSAGE_INDEX 20
#define MESSAGE_INFO_LENGTH 19
#define CLOCK_INFO_LENGTH 4
    
uint16_t shortAddress;
uint8_t addressToSend[2];

    //update_clock();

    /* Get the end point short address */
    result = zbGetDeviceInfo(DIP_SHORT_ADDRESS);
    shortAddress = CONVERT_TO_INT(zmBuf[SRSP_DIP_VALUE_FIELD], zmBuf[SRSP_DIP_VALUE_FIELD + 1]);
    printf("Device's Short Address is %04X\r\n", shortAddress);
    
    /* Break the short address into two 8 bits */
    addressToSend[0] = shortAddress;
    addressToSend[1] = (uint8_t) (shortAddress >> 8);
    printf("Address to send is: %02X %02X\r\n", addressToSend[0], addressToSend[1]);
    
    /* Register the End Device to Coordinator */
    /* Indicate that we are sending a message */
    setLed(SEND_MESSAGE_LED);
    printf("Sending Register Message...");
    
    /* Attempt to send the message */
    result = afSendData(DEFAULT_ENDPOINT, DEFAULT_ENDPOINT, 0, REGISTER_CLUSTER, addressToSend, 2);
    clearLed(SEND_MESSAGE_LED);
    if (result == MODULE_SUCCESS)
    {
      printf("SUCCESS!\r\n");
    } else {
      printf("ERROR %02X ", result);
    }
    
    /*
    timerIsr = &handleTimer;
    int16_t timerResult = initTimer(1);
    if (timerResult != 0)
    {
      printf("timerResult Error %i, STOPPING\r\n", timerResult);
      while(1);
    }*/
    
    printf("hihihihi\r\n");
    
    while (1)
    {
      send_image(text_arr, data_size);
      /*
      printf("Current Time is %d:%d\r\n", hour, minute);
        if (MODULE_HAS_MESSAGE_WAITING())
        {
          //pollAndDisplay();
          getMessage();
          if (zmBuf[SRSP_LENGTH_FIELD] > 0)
          {
            // check the cluster index
            uint16_t cluster = zmBuf[CLUSTER_INDEX] | (zmBuf[CLUSTER_INDEX + 1] << 8);
            printf("cluster received was %04x\r\n", cluster);
            if (cluster == 0xfc02)
            {
              printf("This is a clock message\r\n");
              // the message contains clock information
              // the length of clock information message is always 4 (HH:MM)
              int i;
              for (i = 0; i < CLOCK_INFO_LENGTH; i++)
              {
                receivedMessage[i] = zmBuf[MESSAGE_INDEX + i];
              }
              printf("%c%c:%c%c\r\n", receivedMessage[0], receivedMessage[1], receivedMessage[2], receivedMessage[3], receivedMessage[4]);
              // call method to parse & update the clock
              /* SOON TO BE UPDATED!!!! 
            } else if (cluster == 0xfc03)
            {
              printf("This is a string message\r\n");
              int i;
              for (i = 0; i < MESSAGE_INFO_LENGTH; i++)
              {
                receivedMessage[i] = zmBuf[MESSAGE_INDEX + i];
                printf("%c", receivedMessage[i]);
              }
              printf("\r\n");
            } else {
              printf("ERROR RETRIEVING -- Cluster not identified");
            }
            zmBuf[SRSP_LENGTH_FIELD] = 0;
          }
        }*/
    }
}
                    
void handleTimer() // Will automatically wake up timer
{
}

void increase_counter()
{
  m0++;
  if (m0 > 57) // in ascii
  {
    m1++;
    if (m1 > 53)
    {
      h0++;
      if (h0 > 57)
      {
        h1++;
        h0 = 48;
      } else if (h0 == 52 && h1 == 50)
      {
        h0 = 48;
        h1 = 48;
      }
      m1 = 48;
    }
    m0 = 48;
  }
  str1[0] = (char) h1;
  str1[1] = (char) h0;
  str1[3] = (char) m1;
  str1[4] = (char) m0;
}

/* @} */
