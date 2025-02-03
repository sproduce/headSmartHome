

#ifndef CHANNELSTATUS_H_
#define CHANNELSTATUS_H_

typedef union channel_status
{
	uint32_t channelStatus;
	uint8_t byteStatus[4];
};


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


void setChannelStatus(channel_status *channelStatus, uint32_t *lastChannel){
		shiftRegisterClean();
		digitalWrite(LATCH_PIN, LOW);

		for (int8_t i = SHIFT_REGISTER_COUNT; i >= 0; i--){
			shiftOut(DATA_PIN, CLOCK_PIN , 1 , channelStatus->byteStatus[i]);
		}
		digitalWrite(LATCH_PIN, HIGH);
		channelStatusUpdate = 0;
		*lastChannel = channelStatus->channelStatus;
}



void updateChannel(channel_status *channelStatus, uint32_t *lastChannelStatus){

	if (!channelStatusUpdate && channelStatus->channelStatus != *lastChannelStatus){
		channelStatusUpdate = millis();
		return ;
	}

	if (channelStatusUpdate && (millis() - channelStatusUpdate > 20)){
		//Serial.println("Update Channel");
		//Serial.println(channelStatus->channelStatus);
		switch (channelStatus->channelStatus)
		{
			case 0:
				if (*lastChannelStatus == pow(2,SHIFT_CH)-1){
					channelStatus->channelStatus = *lastChannelStatus;
					for (int8_t i = 0; i < SHIFT_CH; i++){
						bitClear(channelStatus->channelStatus, i);
						setChannelStatus(channelStatus, lastChannelStatus);
						delay(CHANGE_STATUS_DELAY);
					}
				} else {
					setChannelStatus(channelStatus, lastChannelStatus);
				}

			break;
			case uint32_t(pow(2,SHIFT_CH)-1):
					channelStatus->channelStatus = 0;
					for (int8_t i = SHIFT_CH -1; i >= 0; i--){
						bitSet(channelStatus->channelStatus, i);
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
