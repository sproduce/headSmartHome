

#ifndef CHANNELSTATUS_H_
#define CHANNELSTATUS_H_


uint32_t channelStatusUpdate = 0;//timestamp

void shiftRegisterClean(){
	digitalWrite(CLEAR_PIN, LOW);
	digitalWrite(CLEAR_PIN, HIGH);
}



void shiftRegisterInit() {
	pinMode(LATCH_PIN, OUTPUT);
	pinMode(DATA_PIN, OUTPUT);
	pinMode(CLOCK_PIN, OUTPUT);
	pinMode(CLEAR_PIN, OUTPUT);

	shiftRegisterClean();

	digitalWrite(LATCH_PIN, LOW);
	digitalWrite(LATCH_PIN, HIGH);

	pinMode(OE_PIN, OUTPUT);
	digitalWrite(OE_PIN, LOW);
}


void setChannelStatus(const uint32_t *channel, uint32_t *lastChannel){
		shiftRegisterClean();
		digitalWrite(LATCH_PIN, LOW);
		for (int8_t i = CHANNELS-1; i >= 0; i--){
			digitalWrite(DATA_PIN, bitRead(*channel,i));
			digitalWrite(CLOCK_PIN, HIGH);
			digitalWrite(CLOCK_PIN, LOW);
		}
		digitalWrite(LATCH_PIN, HIGH);
		channelStatusUpdate = millis();
		*lastChannel = *channel;
}



void updateChannel(uint32_t *channelStatus, uint32_t *lastChannelStatus){

	if (*channelStatus != *lastChannelStatus && millis() - channelStatusUpdate >50){
		switch (*channelStatus)
		{
			case 0:
				for (int8_t i = 0; i < CHANNELS; i++){
					bitClear(*lastChannelStatus, i);
					setChannelStatus(lastChannelStatus, lastChannelStatus);
					delay(CHANGE_STATUS_DELAY);
				}
			break;
			case uint32_t(pow(2,CHANNELS)-1):
					for (int8_t i = CHANNELS -1; i >= 0; i--){
						bitSet(*lastChannelStatus, i);
						setChannelStatus(lastChannelStatus, lastChannelStatus);
						delay(CHANGE_STATUS_DELAY);
					}

			break;
			default:
				setChannelStatus(channelStatus, lastChannelStatus);
		}
	}
}






#endif /* CHANNELSTATUS_H_ */
