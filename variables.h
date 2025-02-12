/*
 * variables.h
 *
 *  Created on: Feb 11, 2025
 *      Author: volodia
 */

#ifndef VARIABLES_H_
#define VARIABLES_H_



struct button {
	uint8_t channel;
	uint8_t status = 0;
	uint32_t changeTime = 0;
	uint32_t startTime = 0;
	uint32_t lastDuration = 0;
} buttons[COUNT_BUTTONS];





union {
	uint32_t value = 0;
	uint8_t byteValue[4];
}fourByteUnion;




uint32_t channelStatus = 0, lastChannelStatus = 0;
uint32_t dualChannel = 0, alwaysOnChannel = 0;

uint32_t allOffStatus = 0, lastUpdateCan = 0;

uint32_t statusOnDelay[SHIFT_CH];


volatile bool canReceived = false;

uint32_t statusChange[LISTEN_CHANNELS] = {0};
uint8_t forI;





#endif /* VARIABLES_H_ */
