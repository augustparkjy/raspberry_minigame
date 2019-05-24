#include <wiringPi.h>
#include <lcd.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

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

void waitingSonar(int lcd);
void datetime(int lcd);

int main()
{	
	int lcd;
	int sonarIn = 0;
	int best = 0;

	if(wiringPiSetup() == -1){
		printf("setup err\n");
		return 1;
	}

	lcd = lcdInit(2, 16, 4, RS, E, D4, D5, D6, D7, 0, 0, 0, 0);
	
	while(1)
		waitingSonar(lcd);

	return 0;
}

void waitingSonar(int lcd)
{
	int in_sonar = 0; // sonar input
	int in_touch = 0; // touch sensor input

	int tick = 3;

	while(1){
		lcdPosition(lcd, 4, 0);
		lcdPuts(lcd, "* COME *");
		lcdPosition(lcd, 2, 1);
		lcdPuts(lcd, "* CLOSER ! *");
		sleep(1);
		lcdClear(lcd);
		datetime(lcd);

		scanf("%d", &in_sonar);
		
		if(in_sonar==1) //감지되었을 때
		{
			while(tick!=0){//일정 시간 동안만 게임 시작을 위한 입력 대기
				lcdPosition(lcd, 2, 0);
				lcdPuts(lcd, "PRESS ANY KEY");
				sleep(1);
				lcdClear(lcd);
				lcdPosition(lcd, 2, 1);
				lcdPuts(lcd, "PRESS ANY KEY");
				sleep(1);
				lcdClear(lcd);
				//sleep 상태에서 입력 들어 올 때 처리
				if(in_touch==1)
				{
					//게임 시작
				}
				tick--;
			}
			tick =3;
		}
	}
}
void datetime(int lcd){

	while(1){
	time_t timer;
	char buffer_date[26];
	char buffer_time[26];
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer_date, 26, "DATE: %Y:%m:%d", tm_info);
	strftime(buffer_time, 26, "TIME: %H:%M:%S", tm_info);
	//시간 한국시간 동기화
	lcdPosition(lcd, 0, 0);
	lcdPuts(lcd, buffer_date);

	lcdPosition(lcd, 0, 1);
	lcdPuts(lcd, buffer_time);
	sleep(1);
	lcdClear(lcd);
	break;
	}
}

void record(int lcd){
	int now;
	int clock;
}

