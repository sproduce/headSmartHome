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


uint32_t getUint32(uint8_t number){
	uint32_t temp;
	EEPROM.get((number * 4), temp);

	return temp;
}


void setUint32(uint32_t value, uint8_t number){
	EEPROM.put((number * 4), value);
}







#endif /* LIBEEPROM_H_ */
