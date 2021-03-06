/**
 * @ingroup moduleCommunications
 * @{
 *
 * @file example_basic_comms_router_sapi.c
 *
 * @brief Resets Radio, configures this device to be a Zigbee Router, joins a network, then sends a
 * message to the coordinator once per second. Uses the Simple API.
 *
 * This matches example_basic_communications_COORDINATOR.c
 * Get the coordinator running first, or else the router won't have anything to join to.
 * Basic Router Startup:
 * - Reset Radio
 * - Set Startup Options = CLEAR_STATE and CLEAR_CONFIG - This will restore the Module to "factory" configuration
 * - Reset Radio
 * - Set Zigbee DeviceType to ROUTER
 * - If you want to set a custom PANID or channel list, do that here and then reset the radio
 * - Register Application (Configure the Module for our application)
 * - Start Application
 * - Wait for the Start Confirm
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
 * YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� 
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
#include "../ZM/module_errors.h"
#include "../ZM/simple_api.h"
#include "../ZM/zm_phy_spi.h"
#include "module_example_utils.h"  

extern unsigned char zmBuf[ZIGBEE_MODULE_BUFFER_SIZE];

/** Used to store return value from module operations */
moduleResult_t result;  

#define RESTART_AFTER_ZM_FAILURE

int main( void )
{
    halInit(); 
    moduleInit();    
    printf("\r\n++++++++++++++++++++++++++++++++++++++++++++++++++++\r\n");          
    printf("\r\nBasic Communications Example - ROUTER - using Simple API\r\n");
    HAL_ENABLE_INTERRUPTS();

#define MODULE_START_DELAY_IF_FAIL_MS 5000

    start:    while ((result = sapiStartModule(ROUTER, CHANNEL_MASK_11 | CHANNEL_MASK_13, ANY_PAN)) != MODULE_SUCCESS)
    {
        printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);
        delayMs(MODULE_START_DELAY_IF_FAIL_MS);
    }

    printf("On Network!\r\n");
    setLed(0);
    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();                
    displayDeviceInformation();
#else
    displayBasicDeviceInformation();
#endif    

    /* Now the network is running - send a message to the coordinator every few seconds.*/
    unsigned char counter = 0;
    unsigned char testMessage[] = {0xB0,0xB1,0xB2,0xB3,0xB4};
#define TEST_CLUSTER 0x77    

    pollAndDisplay();

    while (1)
    {
        printf("Sending Message %u  ", counter++);
        result = sapiSendData(0, TEST_CLUSTER, testMessage, 5);
        if (result == MODULE_SUCCESS)
        {
            printf("Success\r\n");
        } else {
            printf("ERROR %02X ", result);
#ifdef RESTART_AFTER_ZM_FAILURE
            printf("\r\nRestarting\r\n");
            goto start;
#else        
            printf("stopping\r\n");
            while(1);
#endif
        }
        toggleLed(1);
        delayMs(2000);
    }
}

/* @} */
