#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2024-10-05 22:51:41

#include "Arduino.h"
#include <mcp2515.h>
#include <stdint.h>
#include <EEPROM.h>
#define LISTEN_CHANNELS 32
#define CHANNELS 24
#define HEAD_NUMBER 1
#define CHANGE_STATUS_DELAY 50
#define FIRST_CH  (HEAD_NUMBER * 2 -1) * 16
#define LAST_CH  FIRST_CH + CHANNELS
#define STATUS_NEW_BYTE 1
#define DELAY_SEND_STATUS 30000
#define TEST_ATTEMPTS 20
#define TEST_DELAY 3000
#define RESET_BUTTON 3
#define COUNT_BUTTONS 1
extern union channel_status channelStatus;
extern uint32_t lastChannelStatus;
#define LATCH_PIN 16
#define DATA_PIN 19
#define CLOCK_PIN 17
#define CLEAR_PIN 15
#define OE_PIN 18
#include "channelStatus.h"
#include "libEeprom.h"

void buttonsInit() ;
void clearCan();
void sendResetMessage(void) ;
bool setupEndpoint() ;
bool buttonRead(struct button *currentButton) ;
void canInterrupt() ;
void canRead() ;
void testProgram() ;
void sendChanelStatus() ;
void setup() ;
void loop() ;

#include "headSmartHome.ino"


#endif
