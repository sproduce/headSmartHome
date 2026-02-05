

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


void setChannelStatus(uint32_t *channelStatus, uint32_t *lastChannel){
		fourByteUnion.value = *channelStatus;
		shiftRegisterClean();
		digitalWrite(LATCH_PIN, LOW);

		for (int8_t i = SHIFT_REGISTER_COUNT; i >= 0; i--){
			shiftOut(DATA_PIN, CLOCK_PIN , 1 ,fourByteUnion.byteValue[i]);
		}
		digitalWrite(LATCH_PIN, HIGH);
		channelStatusUpdate = 0;
		*lastChannel = *channelStatus;
}



void updateChannel(uint32_t *channelStatus, uint32_t *lastChannelStatus){
	// start cache timer
	if (!channelStatusUpdate && *channelStatus != *lastChannelStatus){
		channelStatusUpdate = millis();
		return ;
	}

	if (channelStatusUpdate && (millis() - channelStatusUpdate > 20)){
		//Serial.println("Update Channel");
		//Serial.println(channelStatus->channelStatus);
		switch (*channelStatus)
		{
			case 0:
				if (*lastChannelStatus == ((1UL << SHIFT_CH) - 1)){
					*channelStatus = *lastChannelStatus;
					for (int8_t i = 0; i < SHIFT_CH; i++){
						bitClear(*channelStatus, i);
						setChannelStatus(channelStatus, lastChannelStatus);
						delay(CHANGE_STATUS_DELAY);
					}
				} else {
					setChannelStatus(channelStatus, lastChannelStatus);
				}

			break;
			case uint32_t(pow(2,SHIFT_CH)-1):
					*channelStatus = 0;
					for (int8_t i = SHIFT_CH -1; i >= 0; i--){
						bitSet(*channelStatus, i);
						setChannelStatus(channelStatus, lastChannelStatus);
						delay(CHANGE_STATUS_DELAY);
					}
			break;
			default:
				setChannelStatus(channelStatus, lastChannelStatus);
		}
	}
}






#endif /* CHANNELSTATUS_H_ */
