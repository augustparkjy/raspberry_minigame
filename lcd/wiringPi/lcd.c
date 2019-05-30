#include <wiringPi.h>
#include <lcd.h>
#include <stdio.h>

#define RS 25
#define E  24
#define D0 29
#define D1 28
#define D2 27
#define D3 26
#define D4 23
#define D5 22
#define D6 21
#define D7 14
//#define SDA 8
//#define SCL 9
//#define V0 //GND
//#define RW //GND
//#define LEDPLUS //5V
//#define LEDMINUS //GND

int main()
{	
	int lcd;
	if(wiringPiSetup() == -1){
		printf("setup err\n");
		return 1;
	}

	lcd = lcdInit(2, 16, 8, RS, E, D0, D1, D2, D3, D4, D5, D6, D7);

	lcdPosition(lcd, 0, 0);
	lcdPuts(lcd, "HELLO, WORLD!");
	printf("lcdputs\n");	
	sleep(1);
	lcdClear(lcd);
	lcdPuts(lcd, "HIHI");
	sleep(1);

	return 0;
}

