#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2025-04-12 00:05:54

#include "Arduino.h"
#include <mcp2515.h>
#include <stdint.h>
#include <EEPROM.h>
#define MAJOR 1
#define MINOR 1
#define PATCH 4
#define HEAD_NUMBER 1
#define LISTEN_CHANNELS 32
#define SHIFT_REGISTER_COUNT 3
#define SHIFT_CH SHIFT_REGISTER_COUNT * 8
#define FIRST_CH  (HEAD_NUMBER * 2 - 1) * 16
#define LAST_CH  FIRST_CH + 31
#define CHANGE_STATUS_DELAY 50
#define STATUS_NEW_EEPROM 0
#define STATUS_ALWAYSON_EEPROM 1
#define STATUS_DUALCHANNEL_EEPROM 2
#define STATUS_ONDELAY_EEPROM 3
#define DELAY_SEND_STATUS 30000
#define TEST_ATTEMPTS 20
#define TEST_DELAY 3000
#define RESET_BUTTON 3
#define COUNT_BUTTONS 1
#define LATCH_PIN 16
#define DATA_PIN 19
#define CLOCK_PIN 17
#define CLEAR_PIN 15
#define OE_PIN 18
#include "variables.h"
#include "channelStatus.h"
#include "libEeprom.h"

void buttonsInit() ;
void clearCan();
void sendChanelStatus() ;
void sendResetMessage(void) ;
void configChannel(const can_frame *canData);
bool setupEndpoint() ;
bool buttonRead(struct button *currentButton) ;
void canInterrupt() ;
void canRead() ;
void testProgram()  ;
void setup() ;
void loop() ;

#include "headSmartHome.ino"


#endif
