#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <mcp2515.h>


LiquidCrystal_I2C lcd(0x27,16,2);

MCP2515 mcp2515(10);

byte p1[8]={B11111,B10001,B10001,B10001,B10001,B10001,B10001,B11111};

struct can_frame canReceived;
uint8_t switchGroup = 0;
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

	mcp2515.reset();
	mcp2515.setBitrate(CAN_125KBPS);
	mcp2515.setNormalMode();

	writeLcd();
}




void loop() {


	if (mcp2515.readMessage(&canReceived) == MCP2515::ERROR_OK) {
		switchGroup = canReceived.data[1];
		writeLcd();
	}

}
