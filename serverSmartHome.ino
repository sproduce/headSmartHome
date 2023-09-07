#include "Arduino.h"
#include <mcp2515.h>
#include "canParse.h"


#define CHANNELS 16

#define HEAD_NUMBER 1

#define FIRST_CH  HEAD_NUMBER * 16
#define LAST_CH  FIRST_CH + CHANNELS

#define LATCH_PIN A2 //16
#define DATA_PIN A5 //19
#define CLOCK_PIN A3 //17
#define CLEAR_PIN A1 //15
#define OE_PIN A4 //18

#define RESET_BUTTON 3
#define COUNT_BUTTON 1

struct button {
	uint8_t channel;
	uint8_t status = false;
	uint32_t changeTime = 0;
} buttons[COUNT_BUTTON];


MCP2515 mcp2515(10);
can_frame canData;


volatile bool canReceived = false;
volatile bool changeChannelStatus = false;


union allChannel{
	uint16_t status=0;
	uint8_t statusByte[2];
} channel;


void buttonsInit() {
	pinMode(RESET_BUTTON,INPUT_PULLUP);
	buttons[0].channel = RESET_BUTTON;
}

void shiftRegisterInit() {
	pinMode(LATCH_PIN, OUTPUT);
	pinMode(DATA_PIN, OUTPUT);
	pinMode(CLOCK_PIN, OUTPUT);
	pinMode(CLEAR_PIN, OUTPUT);
	digitalWrite(CLEAR_PIN, LOW);
	digitalWrite(CLEAR_PIN, HIGH);

	digitalWrite(LATCH_PIN, LOW);
	digitalWrite(LATCH_PIN, HIGH);

	pinMode(OE_PIN, OUTPUT);
	digitalWrite(OE_PIN, LOW);
}


void setup() {
	shiftRegisterInit();
	buttonsInit();
	pinMode(2, INPUT_PULLUP);


	mcp2515.reset();
	mcp2515.setBitrate(CAN_50KBPS);
	mcp2515.setNormalMode();
	Serial.begin(9600);

	changeChannelStatus = true;

	if (!digitalRead(RESET_BUTTON)){
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
	Serial.println("setupMode");
	sendResetMessage();
	for(;;)
	{
		if (buttonRead(&buttons[0])) {
			if (buttons[0].status) {
				channel.status = 0;
				if (currentBit > 16){
					channel.status = 0;
					setChannelStatus();
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
					channel.status |= 1UL << currentBit;
					canData.can_id = 0x700;
					canData.can_dlc = 1;
					canData.data[0] = FIRST_CH + currentBit;
					mcp2515.sendMessage(&canData);
				} else {
					channel.status = 0xFFFF;
					canData.can_id = 0x700;
					canData.can_dlc = 1;
					canData.data[0] = 0xf0 + HEAD_NUMBER;
					mcp2515.sendMessage(&canData);
				}
				setChannelStatus();
				currentBit++;
			}
		}
	}
}



bool buttonRead(struct button * currentButton) {

uint8_t curretnStatus = !digitalRead(currentButton->channel);

	if (curretnStatus != currentButton->status){
		if (!currentButton->changeTime ) {
			currentButton->changeTime = millis();
		}
		if (millis() - currentButton->changeTime > 100){
			currentButton->changeTime = 0;
			currentButton->status = curretnStatus;
			return true;
		}
	} else {
		if (currentButton->changeTime > 110){
			currentButton->changeTime = 0;
		}
	}


	return false;
}




void canInterrupt()
{
	canReceived = true;
}

void setChannelStatus()
{
	digitalWrite(LATCH_PIN, LOW);
	shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, channel.statusByte[1]);
	shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, channel.statusByte[0]);
	digitalWrite(LATCH_PIN, HIGH);
}


void canRead()
{
	if (mcp2515.readMessage(&canData) == MCP2515::ERROR_OK) {
		if (canData.can_id >= FIRST_CH && canData.can_id <= LAST_CH){
			channel.status ^= 1UL << (canData.can_id - FIRST_CH);

		} else {
			if (canData.can_id = 0xF0 + HEAD_NUMBER){
				channel.status = 0;
			}
		}
		changeChannelStatus = true;
	}

}






void loop() {

	if (canReceived){
		canReceived = false;
		canRead();
	}
	if (changeChannelStatus){
		changeChannelStatus = false;
		setChannelStatus();
	}

}

