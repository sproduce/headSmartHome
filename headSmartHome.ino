#include <mcp2515.h>
#include <stdint.h>
#include <EEPROM.h>


#define LISTEN_CHANNELS 32
#define CHANNELS 24 //count of shift register output

#define HEAD_NUMBER 1 //MAX value 7


#define CHANGE_STATUS_DELAY 50

#define FIRST_CH  (HEAD_NUMBER * 2 -1) * 16
#define LAST_CH  FIRST_CH + CHANNELS

#define STATUS_NEW_BYTE 1 // EEPROM address save status clear byte

#define DELAY_SEND_STATUS 30000 //30 sec update server status

#define TEST_ATTEMPTS 20
#define TEST_DELAY 3000


#define RESET_BUTTON 3
#define COUNT_BUTTONS 1

struct button {
	uint8_t channel;
	uint8_t status = 0;
	uint32_t changeTime = 0;
	uint32_t startTime = 0;
	uint32_t lastDuration = 0;
} buttons[COUNT_BUTTONS];

typedef union channel_status
{
	uint32_t channelStatus;
	uint8_t byteStatus[4];
};

union channel_status channelStatus;



uint32_t lastChannelStatus = 0, allOffStatus = 0, lastUpdate = 0;


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
		updateChannel(&channelStatus.channelStatus, &lastChannelStatus);

		if (buttonRead(&buttons[0]) && buttons[0].status) {
			channelStatus = {0};
			if (currentBit > CHANNELS){ // end learn endPoint
				canData.can_id = 0x707;
				canData.can_dlc = 1;
				canData.data[0] = 1;// can message cannot be empty
				mcp2515.sendMessage(&canData);
				clearCan();
				return 1;
			}
			if (currentBit < CHANNELS){
				channelStatus.channelStatus = bit(currentBit);
				canData.can_id = 0x700;
				canData.can_dlc = 1;
				canData.data[0] = FIRST_CH + currentBit;
				mcp2515.sendMessage(&canData);
			} else {// currentBit = CHANNELS
				channelStatus.channelStatus = pow(2,CHANNELS)-1;
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
				bitToggle(channelStatus.channelStatus, changeBit);
			} else {
				bitWrite(channelStatus.channelStatus, changeBit, status);
			}
			statusChange[changeBit] = millis();
			bitSet(channelStatus.channelStatus, 0);// set ON first channel
		} else {
			if (canData.can_id == 0xF0 + HEAD_NUMBER){
				if (channelStatus.channelStatus || millis() - statusChange[CHANNELS] > 6000){
					allOffStatus = channelStatus.channelStatus;
					channelStatus = {0};
				} else {
					channelStatus.channelStatus = allOffStatus;
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


	channelStatus = {1};

	for(;;){

		if (millis() - lastChange > TEST_DELAY * countTest){
			lastChange = millis();
			channelStatus.channelStatus <<= 1;
//			if (countTest%2 == 0){
//				channelStatus |= 1;
//			}

			if (bitRead(channelStatus.channelStatus, CHANNELS)){
//				if (countTest%2 == 0){
//					countDelay++;
//				}
				channelStatus = {1};
				countTest++;
			}
		}
		updateChannel(&channelStatus.channelStatus, &lastChannelStatus);

		if (countTest == TEST_ATTEMPTS){break;}
		if (buttonRead(&buttons[0])){break;}
	}
	channelStatus = {0};
	updateChannel(&channelStatus.channelStatus, &lastChannelStatus);
}




void sendChanelStatus()
{
	canData.can_id = 0x100;
	canData.can_dlc = 5;
	canData.data[0] = HEAD_NUMBER;
	canData.data[1] = channelStatus.byteStatus[0];
	canData.data[2] = channelStatus.byteStatus[1];
	canData.data[3] = channelStatus.byteStatus[2];
	canData.data[4] = channelStatus.byteStatus[3];
    //write(socketCan, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame);
	mcp2515.sendMessage(&canData);

}





void setup() {
	for(forI = 0;forI<CHANNELS + 1;forI++){
		statusChange[forI] = 0;
		statusOnDelay[forI] = 0;
	}

	statusOnDelay[8] = 2400000;
	statusOnDelay[15] = 2400000;
	statusOnDelay[16] = 500;
	statusOnDelay[17] = 500;
	statusOnDelay[18] = 500;
	statusOnDelay[19] = 500;

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
	sendChanelStatus();

}




uint8_t channel;


void loop() {

	if (canReceived){
		canReceived = false;
		canRead();
		mcp2515.clearInterrupts();
		sendChanelStatus();
	}

	if (millis() - lastUpdate > DELAY_SEND_STATUS) {
		lastUpdate = millis();
		sendChanelStatus();
	}


	updateChannel(&channelStatus.channelStatus, &lastChannelStatus);

	buttonRead(&buttons[0]);
	if (buttons[0].status && millis() - buttons[0].startTime > 6000){
		setupEndpoint();
		setStatusNew(STATUS_NEW_BYTE);
	}
// ToDo cancel if nothing to delay
	for (channel = 0;channel < CHANNELS; channel++){
		if (bitRead(channelStatus.channelStatus, channel)){ // is channel ON
			if (statusOnDelay[channel] && (millis() - statusChange[channel] > statusOnDelay[channel])){
				bitClear(channelStatus.channelStatus, channel);
			}
		} else {//channel OFF

		}
	}


}
