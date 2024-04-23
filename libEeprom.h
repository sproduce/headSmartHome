/*
 * libEeprom.h
 *
 *  Created on: Apr 22, 2024
 *      Author: volodia
 */

#ifndef LIBEEPROM_H_
#define LIBEEPROM_H_

void clearEeprom(){
	for (uint16_t i = 0 ; i < EEPROM.length() ; i++) {
	    EEPROM.write(i, 0);
	  }
}

uint8_t getStatusNew(uint16_t byteNumber)
{
	return EEPROM.read(byteNumber);
}


void setStatusNew(uint16_t byteNumber){
	EEPROM.put(byteNumber, 1);
}



#endif /* LIBEEPROM_H_ */
