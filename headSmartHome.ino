#include <mcp2515.h>
#include <stdint.h>
#include <EEPROM.h>




#define MAJOR 1
#define MINOR 1
#define PATCH 6


#define HEAD_NUMBER 1 //MAX value 7
#define SHIFT_REGISTER_COUNT 2  // possible value 2 or 3


#define LISTEN_CHANNELS 32

#define SHIFT_CH SHIFT_REGISTER_COUNT * 8
#define FIRST_CH  (HEAD_NUMBER * 2 - 1) * 16
#define LAST_CH  FIRST_CH + 31

#define CHANGE_STATUS_DELAY 50


#define STATUS_MODE_EEPROM 0 // EEPROM address save status clear byte
#define STATUS_ALWAYSON_EEPROM 1
#define STATUS_DUALCHANNEL_EEPROM 2
#define STATUS_ONDELAY_EEPROM 3
//!!!!! 3 + SHIFT_CH  MAX NEXT VALUE 35



#define DELAY_SEND_STATUS 30000 //30 sec update server status

#define TEST_ATTEMPTS 20
#define TEST_DELAY 3000

#define COUNT_BUTTONS 1
#define RESET_BUTTON 3

#define STATUS_MODE_LED 0
#define STATUS_MODE_DELAY_1 3000
#define STATUS_MODE_DELAY_2 350




#define LATCH_PIN 16 //A2
#define DATA_PIN 19 //A5
#define CLOCK_PIN 17 //A3
#define CLEAR_PIN 15 //A1
#define OE_PIN 18 //A4


#include "variables.h"
#include "channelStatus.h"
#include "libEeprom.h"



MCP2515 mcp2515(10);
can_frame canData;

//void(* resetFunc) (void) = 0;

void pinoutInit() {
	pinMode(RESET_BUTTON,INPUT_PULLUP);
	buttons[0].channel = RESET_BUTTON;

	pinMode(STATUS_MODE_LED, OUTPUT);
}



void clearCan(){
  mcp2515.clearRXnOVR();
  mcp2515.clearMERR();
  mcp2515.clearInterrupts();
}



void sendChanelStatus()
{
	fourByteUnion.value = channelStatus;
	canData.can_id = 0x100;
	canData.can_dlc = 5;
	canData.data[0] = HEAD_NUMBER;
	canData.data[1] = fourByteUnion.byteValue[0];
	canData.data[2] = fourByteUnion.byteValue[1];
	canData.data[3] = fourByteUnion.byteValue[2];
	canData.data[4] = fourByteUnion.byteValue[3];
    //write(socketCan, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame);
	mcp2515.sendMessage(&canData);
	lastUpdateCan = millis();
}



void sendResetMessage(void)
{
	canData.can_id = 0x777;
	canData.can_dlc = 1;
	canData.data[0] = 1; //can message cannot be empty
	mcp2515.sendMessage(&canData);
}

void configChannel(const can_frame *canData){
	fourByteUnion.value = 0;
	fourByteUnion.byteValue[0] = canData->data[4];
	fourByteUnion.byteValue[1] = canData->data[3];
	fourByteUnion.byteValue[2] = canData->data[2];
	fourByteUnion.byteValue[3] = canData->data[1];
	switch (canData->data[0]){
		case 5: // set dual channel   (example set 3 line dual "cansend can0 02e#05.00.00.00.04")
			fourByteUnion.byteValue[3] = 0;// only available shift register value
			dualChannel = fourByteUnion.value;
		break;

		case 6: // set always on channel
			alwaysOnChannel = fourByteUnion.value;
		break;

		case 7: // set status on delay (cansend can0 02e#07.00.00.ff.0f.05) line 6 status on ~4sec
			if (canData->can_dlc == 6 && canData->data[5] < SHIFT_CH){
				statusOnDelay[canData->data[5]] = fourByteUnion.value;
			}
		break;

		case 100:// save line extension
			setUint32(alwaysOnChannel, STATUS_ALWAYSON_EEPROM);
			setUint32(dualChannel,STATUS_DUALCHANNEL_EEPROM);
			for(uint8_t i = 0; i < SHIFT_CH; i++){
				setUint32(statusOnDelay[i], STATUS_ONDELAY_EEPROM + i);
			}
		break;
		case 101: //clear line extension
			alwaysOnChannel = 0;
			dualChannel = 0;
			for(uint8_t i = 0; i < SHIFT_CH; i++){
				statusOnDelay[i] = 0;
			}
	}
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
			if (currentBit > SHIFT_CH){ // end learn endPoint
				canData.can_id = 0x707;
				canData.can_dlc = 1;
				canData.data[0] = 1;// can message cannot be empty
				mcp2515.sendMessage(&canData);
				clearCan();
				sendChanelStatus();
				break;
			}
			if (currentBit < SHIFT_CH){
				channelStatus = bit(currentBit);
				canData.can_id = 0x700;
				canData.can_dlc = 1;
				canData.data[0] = FIRST_CH + currentBit;
				mcp2515.sendMessage(&canData);
			} else {// currentBit = CHANNELS ALL OFF
				channelStatus = pow(2,SHIFT_CH)-1;
				canData.can_id = 0x700;
				canData.can_dlc = 1;
				canData.data[0] = LAST_CH;
				mcp2515.sendMessage(&canData);
				bitSet(channelStatus, 31); //31 bit ALL OFF
			}
				currentBit++;
				sendChanelStatus();
		}
	}
}



bool buttonRead(struct button *btn) {
	uint8_t currentStatus = !digitalRead(btn->channel);

	if (currentStatus == btn->status) {
		btn->changeTime = 0;
		return false;
	}

	if (btn->changeTime == 0) btn->changeTime = millis();

	if (millis() - btn->changeTime > 50){
			btn->status = currentStatus;
			btn->changeTime = 0;
			btn->startTime = millis();
			return true;
		}

	return false;
}



void canInterrupt()
{
	canReceived = true;
}


void canRead()
{
	uint8_t channelState,status;
	while (mcp2515.readMessage(&canData) == MCP2515::ERROR_OK) {
		if (canData.can_id >= FIRST_CH && canData.can_id <= LAST_CH){
			channelState = canData.can_id - FIRST_CH;
			if (channelState >=0 && channelState < SHIFT_CH){// line channel
				channelStatus |= alwaysOnChannel; // ON system channel
				bitClear(channelStatus, 31); // status ALL OFF
				status = canData.data[0];

				if (bitRead(dualChannel, channelState)){
					statusChange[channelState + 1] = millis();
					if (status>1){//toggle status dual line
						bitWrite(channelStatus, channelState + 1, bitRead(channelStatus, channelState));
						bitToggle(channelStatus, channelState);
					} else {
						if (status){ // ON/OFF dual line
							bitClear(channelStatus, channelState + 1);
							bitSet(channelStatus, channelState);
						} else {
							bitClear(channelStatus, channelState);
							bitSet(channelStatus, channelState + 1);

						}
					}
				} else {
					if (status > 1){
						bitToggle(channelStatus, channelState);
					} else {
						bitWrite(channelStatus, channelState, status);
					}
				}
			}
			else {//system channel

				switch (channelState)
				{
					case 30:
							configChannel(&canData);
					break;

					case 31:
						fourByteUnion.value = channelStatus;
						fourByteUnion.byteValue[3] = 0;// ONLY SHIFT REGISTER
						if (bitRead(channelStatus, channelState) && (millis() - statusChange[channelState] < 3000)){
							channelStatus = allOffStatus;
							if(fourByteUnion.value){
								bitClear(channelStatus, channelState);
							}
						} else {
							allOffStatus = channelStatus;
							channelStatus = 0;
							bitSet(channelStatus, channelState);
						}
					break;
				}
			}
			statusChange[channelState] = millis(); //!!!!! NOT CORRECT
			sendChanelStatus();
		}
	}
}

void testProgram() // add status exit for entering configure
{
	uint8_t countTest = 1;

	uint32_t lastChange = millis();

	channelStatus = 1;

	for(;;){
		if (millis() - lastChange > TEST_DELAY * countTest){
			lastChange = millis();
			channelStatus <<= 1;
//			if (countTest%2 == 0){
//				channelStatus |= 1;
//			}

			if (bitRead(channelStatus, SHIFT_CH - 1)){
//				if (countTest%2 == 0){
//					countDelay++;
//				}
				channelStatus = 1;
				countTest++;
			}
		}
		updateChannel(&channelStatus, &lastChannelStatus);

		if (countTest == TEST_ATTEMPTS){break;}
		if (buttonRead(&buttons[0])){break;}
	}
	channelStatus = 0;
	updateChannel(&channelStatus, &lastChannelStatus);
}



void blinkStatusLed() {
	if ((uint16_t)((uint16_t)millis() - statusModeChangeTime) > STATUS_MODE_DELAY_1){
				statusModeTmp = getUint32(STATUS_MODE_EEPROM) * 2;
				statusModeChangeTime = millis();
			}

		if ((uint16_t)((uint16_t)millis() - statusModeChangeTime) > STATUS_MODE_DELAY_2 && statusModeTmp){
			//digitalWrite(STATUS_MODE_LED, !digitalRead(STATUS_MODE_LED));
			PIND = (1 << 0);
			statusModeTmp --;
			statusModeChangeTime = millis();
		}
}





void setup() {
//after reload send 0 config canId
	shiftRegisterInit();
	pinoutInit();
	pinMode(2, INPUT_PULLUP);
	statusModeChangeTime = millis();
//	if (!getUint32(STATUS_NEW_EEPROM)){
//		delay(100); //cache shift register write
//		testProgram();
//	}


	mcp2515.reset();
	mcp2515.setBitrate(CAN_125KBPS,MCP_8MHZ);
	mcp2515.setNormalMode();
	attachInterrupt(0, canInterrupt, FALLING);

	canData.can_id = 0x1 * HEAD_NUMBER;
	canData.can_dlc = 3;
	canData.data[0] = MAJOR;
	canData.data[1] = MINOR;
	canData.data[2] = PATCH;
	mcp2515.sendMessage(&canData);

	if (!digitalRead(RESET_BUTTON)){
		sendResetMessage();
		clearEeprom();
	}


	alwaysOnChannel = getUint32(STATUS_ALWAYSON_EEPROM);
	dualChannel = getUint32(STATUS_DUALCHANNEL_EEPROM);
	for(uint8_t i = 0; i < SHIFT_CH; i++){
		statusOnDelay[i] = getUint32(STATUS_ONDELAY_EEPROM + i);
	}

	sendChanelStatus();
}


void loop() {
	blinkStatusLed();
	if (canReceived){
		canReceived = false;
		canRead();
		mcp2515.clearInterrupts();
	}

	if (millis() - lastUpdateCan > DELAY_SEND_STATUS) {
		sendChanelStatus();
	}


	updateChannel(&channelStatus, &lastChannelStatus);

	buttonRead(&buttons[0]);
	if (buttons[0].status && millis() - buttons[0].startTime > 6000){
		setupEndpoint();
		setUint32(1, STATUS_MODE_EEPROM);
	}
// ToDo cancel if nothing to delay
	for (uint8_t i = 0; i < SHIFT_CH; i++){
		if (statusOnDelay[i] && bitRead(channelStatus, i)){ // is channel ON
			if ((millis() - statusChange[i] > statusOnDelay[i])){
				bitClear(channelStatus, i);
				sendChanelStatus();
			}
		} else {//channel OFF

		}
	}


}
