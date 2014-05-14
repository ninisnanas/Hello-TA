/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_secure_comms_coordinator_afzdo.c
 *
 * @brief Resets Radio, configures this device to be a Zigbee Coordinator, and displays any received
 * messages. Supports several security options such as preconfigured key, etc.
 * Uses the AF/ZDO interface.
 *
 * This matches example_secure_communications_ROUTER.c
 * Get this running before the router, or else the router won't have anything to join to.
 *
 * Note: security mode should be the same for all devices on the network and can be one of:
- SECURITY_MODE_PRECONFIGURED_KEYS
- SECURITY_MODE_COORD_DIST_KEYS
- SECURITY_MODE_OFF
 *
 * $Rev: 1996 $
 * $Author: dsmith $
 * $Date: 2014-01-02 19:10:16 -0800 (Thu, 02 Jan 2014) $
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
#include "../ZM/module.h"
#include "../ZM/af.h"
#include "../ZM/zdo.h"
#include "../ZM/module_errors.h"
#include "../ZM/module_utilities.h"
#include "../ZM/zm_phy.h"
#include "module_example_utils.h"
#include <stdint.h>
#include <stddef.h>

/* Encryption key used in security - must be 16 bytes in length */
uint8_t key[16] = "*Tesla Controls*";

moduleResult_t result;

extern uint8_t zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];
    uint8_t counter = 0;
    uint8_t testMessage[] = {0x30, 0x31, 0x32, 0x33, 0x34};
    
    /* Array to hold address */
    uint16_t shortAddress[100];
    int addressPointer = 0;
    
    int onNetwork = 0;

#define CLOCK_CLUSTER 0xfc02
#define MESSAGE_CLUSTER 0xfc03
#define CLUSTER_INDEX 5
#define MESSAGE_INDEX 4


int main( void )
{
    halInit();
    moduleInit();

    printf("\r\n****************************************************\r\n");
    printf("Secure Communications Example (Modified) - COORDINATOR - using AFZDO\r\n");
    HAL_ENABLE_INTERRUPTS();
    setLed(0);

#define MODULE_START_DELAY_IF_FAIL_MS 5000

    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_COORDINATOR;
    defaultConfiguration.securityMode = SECURITY_MODE_COORD_DIST_KEYS;
    defaultConfiguration.securityKey = key;
    
    struct applicationConfiguration af;
    
    af.bindingInputClusters[0] = 0x0000;
    af.bindingInputClusters[1] = 0xfc01;
    af.bindingInputClusters[2] = 0xfc02;
    af.bindingInputClusters[3] = 0xfc03;
    af.bindingOutputClusters[0] = 0x0000;
    af.bindingOutputClusters[1] = 0xfc01;
    af.bindingOutputClusters[2] = 0xfc02;
    af.bindingOutputClusters[3] = 0xfc03;
    af.deviceId = 0x8888;
    af.deviceVersion = 0x01;
    af.endPoint = 1;
    af.latencyRequested = LATENCY_NORMAL;
    af.numberOfBindingInputClusters = 4;
    af.numberOfBindingOutputClusters = 4;
    af.profileId = 0xcc00; // clock profile
    
    while ((result = startModule(&defaultConfiguration, &af)) != MODULE_SUCCESS)
    //while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
    {
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS);
    }
    
    if (result == MODULE_SUCCESS) onNetwork = 1;

    printf("On Network!\r\n");
    setLed(1);

    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();                
    displayDeviceInformation();
#else
    displayBasicDeviceInformation();
#endif  
    
    for (addressPointer = 0; addressPointer < 100; addressPointer++)
    {
      shortAddress[addressPointer] = 0xFFFF;
    }
    addressPointer = 0;  
    
    /* Now the network is running - continually poll for any received messages from the Module */
    while (1)
    {
      if (onNetwork == 1)
      {
        //printf("addressPointer = %d\r\n", addressPointer);
        if (MODULE_HAS_MESSAGE_WAITING())
        {
          printf("Got connection from end device\r\n");
          getMessage();
          if (zmBuf[SRSP_LENGTH_FIELD] == 13)
          {
            if (shortAddress[addressPointer] != 0)
            {
              addressPointer++;
            }
            shortAddress[addressPointer] = zmBuf[MESSAGE_INDEX + 1] | (zmBuf[MESSAGE_INDEX] << 8);
            printf("End device with short address: %04X registered to index %d\r\n", shortAddress[addressPointer], addressPointer);
          }
          zmBuf[SRSP_LENGTH_FIELD] = 0;
        } else if (addressPointer > 0) 
        {
          /* Send message every four seconds */
          printf("Sending message %u ", counter++);
          if (counter % 2 == 0) result = afSendData(DEFAULT_ENDPOINT, DEFAULT_ENDPOINT, shortAddress[addressPointer], CLOCK_CLUSTER, testMessage, 5);
          else result = afSendData(DEFAULT_ENDPOINT, DEFAULT_ENDPOINT, shortAddress[addressPointer], MESSAGE_CLUSTER, testMessage, 5);
          clearLed(SEND_MESSAGE_LED);
          
          if (result == MODULE_SUCCESS)
          {
            printf ("Success\r\n");
          } else
          {
            printf("ERROR %02X\r\n", result);
          }
        }
      }
      
      delayMs(4000);
    }
    /* Now the network is running - continually poll for any received messages from the Module 
    while (1)
    {
        if (MODULE_HAS_MESSAGE_WAITING())
        {
          printf("Got connection from end device");
          getMessage();
          if (zmBuf[SRSP_LENGTH_FIELD] == 13)
          {
            if (shortAddress[addressPointer] != 0)
            {
              addressPointer++;
            }
            shortAddress[addressPointer] = zmBuf[MESSAGE_INDEX + 1] | (zmBuf[MESSAGE_INDEX] << 8);
            printf("End device with short address: %04X registered to index %d\r\n", shortAddress[addressPointer], addressPointer);
          }
          zmBuf[SRSP_LENGTH_FIELD] = 0;
        }
        
        printf("Sending Message %u ", counter++);
        /* Attempt to send the message 
        if (counter % 2 == 0) result = afSendData(DEFAULT_ENDPOINT, DEFAULT_ENDPOINT, shortAddress[addressPointer], CLOCK_CLUSTER, testMessage, 5);
        else result = afSendData(DEFAULT_ENDPOINT, DEFAULT_ENDPOINT, shortAddress[addressPointer], MESSAGE_CLUSTER, testMessage, 5);
        clearLed(SEND_MESSAGE_LED);
        if (result == MODULE_SUCCESS)
        {
          printf("Success\r\n");
        } else
        {
          printf("ERROR %02X ", result);
        }
        delayMs(4000);
    }*/
}

void handleTimer()
{
  printf("#");
}

/* @} */

