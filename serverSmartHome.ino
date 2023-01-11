#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);


byte p1[8]={B11111,B10001,B10001,B10001,B10001,B10001,B10001,B11111};

uint32_t switchGroup=0;
byte kolSwitch = sizeof(switchGroup)*8;

uint8_t syncArray[32][8];

void writeLcd()
{
	for(int i=0;i<kolSwitch;i++)
	{
		int line = int(i/16);
		lcd.setCursor(i-16*line,line);

		if ((switchGroup>>i)&1U){
			lcd.write(0xff);
		} else {
			lcd.write(1);
		}
	}
}



void setup() {
	lcd.init();
	lcd.createChar(1,p1);
	lcd.clear();
	lcd.backlight();

	writeLcd();
}




void loop() {

	for(int i=0;i<kolSwitch;i++){
		switchGroup |=1UL << i;
		writeLcd();
		delay(4000);
		switchGroup=0;
	}


}
