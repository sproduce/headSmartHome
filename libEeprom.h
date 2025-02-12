/*
 * libEeprom.h
 *
 *  Created on: Apr 22, 2024
 *      Author: volodia
 */

#ifndef LIBEEPROM_H_
#define LIBEEPROM_H_
#include "variables.h"




void clearEeprom(){
	for (uint16_t i = 0 ; i < EEPROM.length() ; i++) {
	    EEPROM.write(i, 0);
	  }
}


uint32_t getUint32(uint8_t number){
	fourByteUnion.value = 0;
	for(forI = 0; forI < 4; forI++){
		fourByteUnion.byteValue[forI] = EEPROM.read((number * 4) + forI);
	}

	return fourByteUnion.value;
}


void setUint32(uint32_t value, uint8_t number){
	fourByteUnion.value = value;
	for(forI = 0; forI < 4; forI++){
		EEPROM.put((number * 4) + forI, fourByteUnion.byteValue[forI]);
	}

}







#endif /* LIBEEPROM_H_ */
