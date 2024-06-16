#include <mcp2515.h>
#include <stdint.h>
#include <EEPROM.h>


#define LISTEN_CHANNELS 16
#define CHANNELS 16 //count of shift register output

#define HEAD_NUMBER 1 //ODD number only MAX value 13


#define CHANGE_STATUS_DELAY 50

#define FIRST_CH  HEAD_NUMBER * 16
#define LAST_CH  FIRST_CH + CHANNELS

#define STATUS_NEW_BYTE 1 // EEPROM address save status clear byte



#define TEST_ATTEMPTS 20
#define TEST_DELAY 2000


#define RESET_BUTTON 3
#define COUNT_BUTTONS 1

struct button {
	uint8_t channel;
	uint8_t status = 0;
	uint32_t changeTime = 0;
	uint32_t startTime = 0;
	uint32_t lastDuration = 0;
} buttons[COUNT_BUTTONS];


uint32_t channelStatus = 0, lastChannelStatus = 0, allOffStatus = 0;


	#define LATCH_PIN 16 //A2
	#define DATA_PIN 19 //A5
	#define CLOCK_PIN 17 //A3
	#define CLEAR_PIN 15 //A1
	#define OE_PIN 18 //A4


	#include "channelStatus.h"

	#include "libEeprom.h"

MCP2515 mcp2515(10);
can_frame canData;

volatile bool canReceived = false;

uint32_t statusChange[CHANNELS + 1];
uint32_t statusOnDelay[CHANNELS + 1];
uint8_t forI;
uint8_t statusNew;

void buttonsInit() {
	pinMode(RESET_BUTTON,INPUT_PULLUP);
	buttons[0].channel = RESET_BUTTON;
}



void clearCan(){
  mcp2515.clearRXnOVR();
  mcp2515.clearMERR();
  mcp2515.clearInterrupts();
}






void sendResetMessage(void)
{
	canData.can_id = 0x777;
	canData.can_dlc = 1;
	canData.data[0] = 1; //can message cannot be empty
	mcp2515.sendMessage(&canData);
}

void sendByteMessage(uint8_t canId, uint8_t canDataByte = 1) {
	canData.can_id = canId;
	canData.can_dlc = 1;
	canData.data[0] = canDataByte;
	mcp2515.sendMessage(&canData);
}






bool setupEndpoint()
{
	uint8_t currentBit = 0;
	buttons[0].status = 0;
	buttons[0].startTime = 0;
	for(;;)
	{
		updateChannel(&channelStatus, &lastChannelStatus);

		if (buttonRead(&buttons[0]) && buttons[0].status) {
			channelStatus = 0;
			if (currentBit > CHANNELS){ // end learn endPoint
				canData.can_id = 0x707;
				canData.can_dlc = 1;
				canData.data[0] = 1;// can message cannot be empty
				mcp2515.sendMessage(&canData);
				clearCan();
				return 1;
			}
			if (currentBit < CHANNELS){
				channelStatus = bit(currentBit);
				canData.can_id = 0x700;
				canData.can_dlc = 1;
				canData.data[0] = FIRST_CH + currentBit;
				mcp2515.sendMessage(&canData);
			} else {// currentBit = CHANNELS
				channelStatus = pow(2,CHANNELS)-1;
				canData.can_id = 0x700;
				canData.can_dlc = 1;
				canData.data[0] = 0xF0 + HEAD_NUMBER;
				mcp2515.sendMessage(&canData);
			}
				currentBit++;
		}
	}
}



bool buttonRead(struct button *currentButton) {

	uint8_t curretnStatus = !digitalRead(currentButton->channel);

	if (curretnStatus != currentButton->status){
		if (!currentButton->changeTime ) {
			currentButton->changeTime = millis();
		}
		if (millis() - currentButton->changeTime > 50){
			currentButton->changeTime = 0;
			currentButton->status = curretnStatus;
			currentButton->lastDuration = millis() - currentButton->startTime;
			currentButton->startTime = millis();
			return true;
		}
	} else {
		currentButton->changeTime = 0;
	}


	return false;
}



void canInterrupt()
{
	canReceived = true;
}


void canRead()
{
	uint8_t changeBit,status;
	while (mcp2515.readMessage(&canData) == MCP2515::ERROR_OK) {
		if (canData.can_id >= FIRST_CH && canData.can_id <= LAST_CH){
			changeBit = canData.can_id - FIRST_CH;
			status = canData.data[0];
			if (status > 1){
				bitToggle(channelStatus, changeBit);
			} else {
				bitWrite(channelStatus, changeBit, status);
			}
			statusChange[changeBit] = millis();
			bitSet(channelStatus, 0);// set ON first channel
		} else {
			if (canData.can_id == 0xF0 + HEAD_NUMBER){
				if (channelStatus || millis() - statusChange[CHANNELS] > 6000){
					allOffStatus = channelStatus;
					channelStatus = 0;
				} else {
					channelStatus = allOffStatus;
				}
				statusChange[CHANNELS] = millis();
			}
		}
	}
}

void testProgram()
{
	uint8_t countTest = 1;

	uint32_t lastChange = millis();


	channelStatus = 1;

	for(;;){

		if (millis() - lastChange > TEST_DELAY * countTest){
			lastChange = millis();
			updateChannel(&channelStatus, &lastChannelStatus);
			channelStatus <<=1;
//			if (countTest%2 == 0){
//				channelStatus |= 1;
//			}

			if (bitRead(channelStatus, CHANNELS)){
//				if (countTest%2 == 0){
//					countDelay++;
//				}
				channelStatus = 1;
				countTest++;
			}
		}


		if (countTest == TEST_ATTEMPTS){break;}
		if (buttonRead(&buttons[0])){break;}
	}
	channelStatus = 0;
	updateChannel(&channelStatus, &lastChannelStatus);
}





void setup() {
	for(forI = 0;forI<CHANNELS + 1;forI++){
		statusChange[forI] = 0;
		statusOnDelay[forI] = 0;
	}

	statusOnDelay[5] = 2400000;
	statusOnDelay[14] = 500;
	statusOnDelay[15] = 500;

	shiftRegisterInit();
	buttonsInit();
	pinMode(2, INPUT_PULLUP);

	mcp2515.reset();
	mcp2515.setBitrate(CAN_125KBPS,MCP_8MHZ);
	mcp2515.setNormalMode();
	attachInterrupt(0, canInterrupt, FALLING);

	if (!digitalRead(RESET_BUTTON)){
		sendResetMessage();
		clearEeprom();
	}

	statusNew = !getStatusNew(STATUS_NEW_BYTE);

	if (statusNew){
		delay(100); //cache shift register write
		testProgram();
	}


}




uint8_t channel;


void loop() {

	if (canReceived){
		canReceived = false;
		canRead();
		mcp2515.clearInterrupts();
	}

	updateChannel(&channelStatus, &lastChannelStatus);

	buttonRead(&buttons[0]);
	if (buttons[0].status && millis() - buttons[0].startTime >6000){
		setupEndpoint();
		setStatusNew(STATUS_NEW_BYTE);
	}

	for (channel = 0;channel < CHANNELS; channel++){
		if (bitRead(channelStatus, channel)){ // is channel ON
			if (statusOnDelay[channel] && (millis() - statusChange[channel] > statusOnDelay[channel])){
				bitClear(channelStatus, channel);
			}
		} else {//channel OFF

		}
	}




}
