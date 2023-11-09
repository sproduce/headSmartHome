

#ifndef CHANNELSTATUS_H_
#define CHANNELSTATUS_H_


byte tmpByte;

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

void setChannelStatus(const uint16_t *channel)
{
	digitalWrite(LATCH_PIN, LOW);
	tmpByte = *channel >> 8;
	shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, tmpByte);
	tmpByte = *channel & 0xFF;
	shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, *channel);
	digitalWrite(LATCH_PIN, HIGH);
}

#endif /* CHANNELSTATUS_H_ */
