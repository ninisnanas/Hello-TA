/**
* @ingroup moduleCommunications
* @{
*
* @file example_basic_comms_router_afzdo.c
*
* @brief Resets Radio, configures this device to be a Zigbee Router, joins a network, then sends a
* message to the coordinator once per second.
* Uses the AF/ZDO interface.
* Pressing the button will display device information.
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
* If devices are not communicating, look at the device information fields and verify that both
* the coordinator and router have the same PAN ID and that the Extended PAN ID of the router
* matches the MAC address of the coordinator.
*
* @note code size for this example is ~0.5kB larger than SAPI example due to displayDeviceInformation() in handleButtonPress().
*
* $Rev: 2018 $
* $Author: bcostabile $
* $Date: 2014-01-19 12:51:53 -0800 (Sun, 19 Jan 2014) $
*
* @section support Support
* Please refer to the wiki at www.anaren.com/air-wiki-zigbee for more information. Additional support
* is available via email at the following addresses:
* - Questions on how to use the product: AIR@anaren.com
* - Feature requests, comments, and improvements:  featurerequests@teslacontrols.com
* - Consulting engagements: sales@teslacontrols.com
*
* @section license License
* Copyright (c) 2013 Tesla Controls. All rights reserved. This Software may only be used with an 
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
#include "../ZM/module_utilities.h"
#include "../ZM/af.h"
#include "../ZM/zdo.h"
#include "module_example_utils.h"

/** function pointer (in hal file) for the function that gets called when a button is pressed*/
extern void (*buttonIsr)(int8_t);

/** Used to store return value from module operations */
moduleResult_t result;                

/** Handles button interrupt */
void handleButtonPress(int8_t whichButton);

// DEBUG
//#define RESTART_AFTER_ZM_FAILURE

uint8_t counter = 0;
uint8_t testMessage[] = {0xE0,0xE1,0xE2,0xE3,0xE4};

/** The number of failed messages before initiating a network restart */
uint8_t failCount = 0;
#define MAX_FAILED_MESSAGES_BEFORE_RESTART  2

int main( void )
{
    halInit();
    moduleInit();
    printf("\r\n****************************************************\r\n");
    printf("Basic Communications Example - ROUTER - using AFZDO\r\n");
    buttonIsr = &handleButtonPress;
    
#define MODULE_START_DELAY_IF_FAIL_MS 5000
    
    /* Use the default module configuration */
    struct moduleConfiguration defaultConfiguration = DEFAULT_MODULE_CONFIGURATION_ROUTER;
    /* Change this if you are using a custom PAN */
    defaultConfiguration.panId = ANY_PAN;
    
    /* Turn Off nwk status LED if on */
    clearLed(ON_NETWORK_LED);
    /* Loop until module starts */

#ifdef RESTART_AFTER_ZM_FAILURE
start:
#endif
    /* Try Auto-Start first. This will NOT clear out the module's previous network state; the module 
    will just pick up where it left off. This is a good demonstration of a fast join. */
    defaultConfiguration.startupOptions = 0; 
    /* Start the module, first using auto-start, and then if it fails or a button is pressed then
    do a standard startup instead, clearing out all previous network information.
    If message sending fails (e.g. Error 0xCD in method 0x2300) then we increment a failCount.
    Once we've had a few failures then we know that we need to do a full start. 
    To do this, we switch to our default startup options, which clear everything out. */
    if (((expressStartModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION, MODULE_REGION_NORTH_AMERICA)) != MODULE_SUCCESS) 
        || (failCount >= MAX_FAILED_MESSAGES_BEFORE_RESTART) || (buttonIsPressed(ANY_BUTTON)))
    {
        /* Note: If using auto-start (eg startupOptions = 0) then expressStartModule will not return an 
        error if no network is present, and this loop will never take more than one time. 
        This can happen when the module was previously programmed to be one device type (e.g. coordinator)
        but running this code you want it to be a different one (e.g. router). Auto-start mode will not
        return an error if there's no network; this is discovered when the router goes to send a message
        and it fails. In this case then skip auto-start and do a full restart, clearing out old information. */         
        
        printf("Switching to Default\r\n");
        defaultConfiguration.startupOptions = DEFAULT_STARTUP_OPTIONS;   
        while ((result = startModule(&defaultConfiguration, GENERIC_APPLICATION_CONFIGURATION)) != MODULE_SUCCESS)
        {
            /* Module startup failed; display error and blink LED */
            setLed(NETWORK_FAILURE_LED);                    
            printf("Module start unsuccessful. Error Code 0x%02X. Retrying...\r\n", result);
            delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);                    
            clearLed(NETWORK_FAILURE_LED);
            delayMs(MODULE_START_DELAY_IF_FAIL_MS/2);
        }
    }
    printf("On Network!\r\n");
    /* Indicate we got on the network */
    setLed(ON_NETWORK_LED); 
    
    /* On network, display info about this network */
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();                
    displayDeviceInformation();
#else
    displayBasicDeviceInformation();
#endif
    
    /* Now the network is running - send a message to the coordinator every few seconds.*/
    
    HAL_ENABLE_INTERRUPTS();
    
#define TEST_CLUSTER 0x77
    
    while (1)
    {
        /* Indicate that we are sending a message */
        setLed(SEND_MESSAGE_LED);
        printf("Sending Message %u  ", counter++);
        /* Attempt to send the message.
        Note: if you would like to send the message with APS Acknowledgments then change
        AF_MAC_ACK to AF_APS_ACK below. This can be done on a per-message basis. */
        afSetAckMode(AF_MAC_ACK);        
        result = afSendData(DEFAULT_ENDPOINT,DEFAULT_ENDPOINT,0, TEST_CLUSTER, testMessage, 5);
        clearLed(SEND_MESSAGE_LED);
        if (result == MODULE_SUCCESS)
        {
            printf("Success\r\n");
            failCount = 0;
        } else {
            printf("ERROR %02X ", result);
#ifdef RESTART_AFTER_ZM_FAILURE
            printf("\r\nRestarting\r\n");
            // Note: may need to do a full start. If we do an auto-start we could end up in an endless loop here.
            failCount++;
            goto start;
#else        
            printf("stopping\r\n");
            while(1);
#endif
        }
        delayMs(2000);
    }
}

/** When a button is pressed, display device information */
void handleButtonPress(int8_t whichButton)
{
#ifdef DISPLAY_NETWORK_INFORMATION     
    displayNetworkConfigurationParameters();                
    displayDeviceInformation();
#else
    displayBasicDeviceInformation();
#endif
}

/* @} */
