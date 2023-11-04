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


union durationUnion
{
	uint32_t duration;
	uint8_t durationByte[4];
} tmpDuration;

struct button {
	uint8_t channel;
	uint8_t status = 0;
	uint32_t changeTime = 0;
	uint32_t startTime = 0;
	uint32_t lastDuration = 0;
} buttons[COUNT_BUTTONS];


//struct button {
//	uint8_t channel;
//	uint8_t status = 0;
//	uint32_t changeTime = 0;
//	uint32_t startTime = 0;
//	uint32_t lastDuration = 0;
//} buttons[COUNT_BUTTON];



uint16_t channelStatus=0, allOffStatus = 0;
uint32_t lastChannelStatusUpdate = 0;


#if DEBUG == 0
	#include "channelStatusDebug.h"
#else
	#define LATCH_PIN 16 //A2
	#define DATA_PIN 19 //A5
	#define CLOCK_PIN 17 //A3
	#define CLEAR_PIN 15 //A1
	#define OE_PIN 18 //A4
	#include "channelStatus.h"
#endif





MCP2515 mcp2515(10);
can_frame canData;

volatile bool canReceived = false;
volatile bool changeChannelStatus = false;




void buttonsInit() {
	pinMode(RESET_BUTTON,INPUT_PULLUP);
	buttons[0].channel = RESET_BUTTON;
}



void setup() {
	//Serial.begin(9600);


	shiftRegisterInit();
	buttonsInit();
	pinMode(2, INPUT_PULLUP);

	mcp2515.reset();
	mcp2515.setBitrate(CAN_250KBPS);
	mcp2515.setNormalMode();

	changeChannelStatus = true;

	if (!digitalRead(RESET_BUTTON)){
		sendResetMessage();
		setupEndpoint();
	}
	attachInterrupt(0, canInterrupt, FALLING);
}



void sendResetMessage(void)
{
	canData.can_id = 0x777;
	canData.can_dlc = 1;
	canData.data[0] = 1; //can message cannot be empty
	mcp2515.sendMessage(&canData);
}

void sendByteMessage(unsigned canId, uint8_t canDataByte) {
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
		if (buttonRead(&buttons[0]) && buttons[0].status) {
			channelStatus = 0;
			if (currentBit > 16){ // end learn endPoint
				setChannelStatus(&channelStatus);
				canData.can_id = 0x707;
				canData.can_dlc = 1;
				canData.data[0] = 1;// can message cannot be empty
				mcp2515.sendMessage(&canData);
				mcp2515.clearRXnOVR();
				mcp2515.clearMERR();
				mcp2515.clearInterrupts();
				return 1;
			}
			if (currentBit < 16){
				channelStatus = bit(currentBit);
				canData.can_id = 0x700;
				canData.can_dlc = 1;
				canData.data[0] = FIRST_CH + currentBit;
				mcp2515.sendMessage(&canData);
			} else {// currentBit = 16
				channelStatus = 0xFFFF;
				canData.can_id = 0x700;
				canData.can_dlc = 1;
				canData.data[0] = 0xf0 + HEAD_NUMBER;
				mcp2515.sendMessage(&canData);
			}
			setChannelStatus(&channelStatus);
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
	if (mcp2515.readMessage(&canData) == MCP2515::ERROR_OK) {

		tmpDuration.durationByte[0] = canData.data[1];
		tmpDuration.durationByte[1] = canData.data[2];
		tmpDuration.durationByte[2] = canData.data[3];
		tmpDuration.durationByte[3] = canData.data[4];

		if (canData.can_id >= FIRST_CH && canData.can_id <= LAST_CH){
			channelStatus ^= bit(canData.can_id - FIRST_CH);
			channelStatus |= bit(15);// set ON last channel
		} else {
			if (canData.can_id == 0xF0 + HEAD_NUMBER){
				if (tmpDuration.duration > 3000){
					allOffStatus = channelStatus;
					channelStatus = 0;
				} else {
					channelStatus = allOffStatus;
				}

			}
		}
		changeChannelStatus = true;

		//Serial.println(canData.data[0]);
		//Serial.println(tmpDuration.duration);
	}

}




bool firstChange = true;

void loop() {

	if (canReceived){
		canReceived = false;
		canRead();
	}


	if (changeChannelStatus){
		if (firstChange){
			firstChange = false;
			lastChannelStatusUpdate = millis();
		}
		if (millis() - lastChannelStatusUpdate >100)
		{
			lastChannelStatusUpdate = millis();
			changeChannelStatus = false;
			firstChange = true;
			setChannelStatus(&channelStatus);
		}
	}



	buttonRead(&buttons[0]);
	if (buttons[0].status && millis() - buttons[0].startTime >5000){
		setupEndpoint();
	}

//	if (buttonRead(&buttons[0]) && buttons[0].status) {
//
//	}


}

