#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <lcd.h>
#include <time.h>

//SONAR
#define TRIG	 0
#define ECHO	 2
#define SENSOR	 29
#define LED	 29

//LCD
#define RS	25
#define E	24
#define D4	23
#define D5	22
#define D6	21
#define D7	14

#define WAITING_CLOSER 	1
#define WAITING_INPUT 	2
#define PLAYING		3

void setup();
void L_datetime(int lcd);
void *L_func();
int S_getCM();
void *T_func();

int d = 0;
int gameStatus = WAITING_CLOSER;

int main()
{
	
	pthread_t p_thread[2];
	int thr_id;
	int status;
	char p1[] = "thread_lcd";
	char p2[] = "thread_touch";
	char pM[] = "thread_main";
	
	//create thread
	thr_id = pthread_create(&p_thread[0], NULL, L_func, (void*)p1);
	if(thr_id <0)
	{
		perror("thread create error : ");
		exit(0);
	}

	printf("create thread done\n");

//	thr_id = pthread_create(&p_thread[1], NULL, T_fuc, (void*)p2);
//	if(thr_id < 0)
//	{
//		perror("thread create error : ");
//		exit(0);
//	}


	//exit thread
	//pthread_join(p_thread[0], (void**)&status);
	//pthread_join(p_thread[1], (void**)&status);

	//setup devices
	setup();
	
	while(1)
	{
	
		d=S_getCM();
		sleep(1);
		if(gameStatus==WAITING_CLOSER && d<50){
			gameStatus = WAITING_INPUT;
		}
		printf("gs: %d\n", gameStatus);
	}


	

}


void setup() 
{
	printf("setup\n");
	wiringPiSetup();
	pinMode(TRIG, OUTPUT);
	pinMode(ECHO, INPUT);

	//TRIG pin must start LOW
	digitalWrite(TRIG, LOW);
	delay(30);
}
					
int S_getCM() 
{
	
	//Send trig pulse
	digitalWrite(TRIG, HIGH);
	delayMicroseconds(20);
	digitalWrite(TRIG, LOW);

	//wait for echo start
	while(digitalRead(ECHO) == LOW);
	long startTime = micros();
	while(digitalRead(ECHO) == HIGH);
        long travelTime = micros() - startTime;
					
        //Get distance in cm
        int distance = travelTime / 58;
	printf("Distance:  %d\n", distance);
					
        return distance;
}		

void *L_func()
{
	sleep(1);
	int lcd;
	int count = 0;
	lcd = lcdInit(2, 16, 4, RS, E, D4, D5, D6, D7, 0,0,0,0);

	while(1){
	switch(gameStatus)
	{
		case WAITING_CLOSER :
			while(gameStatus==WAITING_CLOSER){
				lcdPosition(lcd, 4, 0);
				lcdPuts(lcd, "* COME *");
				lcdPosition(lcd, 2, 1);
				lcdPuts(lcd, "* CLOSER ! *");
				sleep(1);
				lcdClear(lcd);
				L_datetime(lcd);
			}
		case WAITING_INPUT :
			while(count<3){
				printf("cnt: %d\n", count);
				lcdPosition(lcd, 2, 0);
				lcdPuts(lcd, "PRESS ANY KEY");
				sleep(1);
				lcdClear(lcd);
				lcdPosition(lcd, 2, 1);
				lcdPuts(lcd, "PRESS ANY KEY");
				sleep(1);
				lcdClear(lcd);
				count++;
				if(gameStatus == PLAYING)
				{
					count = 0;
					break;
				}
			}
			count = 0;
			gameStatus = WAITING_CLOSER;
		case PLAYING :
			//PLAYING DISPLAY
			break;
	}
	}
}

void L_datetime(int lcd){
	while(1){
		time_t timer;
		char buffer_date[26];
		char buffer_time[26];
		struct tm* tm_info;

		time(&timer);
		tm_info=localtime(&timer);

		strftime(buffer_date, 26, "DATE: %Y:%m:%d", tm_info);
		strftime(buffer_time, 26, "TIME: %H:%M:%S", tm_info);

		lcdPosition(lcd, 0, 0);
		lcdPuts(lcd, buffer_date);

		lcdPosition(lcd, 0, 1);
		lcdPuts(lcd, buffer_time);
		sleep(1);
		lcdClear(lcd);
		break;
	}
}
