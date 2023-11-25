#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2023-11-26 00:03:08

#include "Arduino.h"
#include "Arduino.h"
#include <mcp2515.h>
#include <stdint.h>
#define DEBUG 1
#define CHANNELS 16
#define HEAD_NUMBER 1
#define FIRST_CH  HEAD_NUMBER * 16
#define LAST_CH  FIRST_CH + CHANNELS
#define RESET_BUTTON 3
#define COUNT_BUTTONS 1
extern uint16_t channelStatus;
extern uint32_t lastChannelStatusUpdate;
#define LATCH_PIN 16
#define DATA_PIN 19
#define CLOCK_PIN 17
#define CLEAR_PIN 15
#define OE_PIN 18
#include "channelStatus.h"

void buttonsInit() ;
void setup() ;
void sendResetMessage(void) ;
void sendByteMessage(uint8_t canId, uint8_t canDataByte) ;
bool setupEndpoint() ;
bool buttonRead(struct button *currentButton) ;
void canInterrupt() ;
void toggleChannel(uint8_t channel) ;
void OffChanel(uint8_t channel) ;
void canRead() ;
void loop() ;

#include "headSmartHome.ino"


#endif
