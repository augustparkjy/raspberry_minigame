#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <lcd.h>
#include <time.h>

#define TRIG 0
#define ECHO 2

#define RS 25
#define E 24
#define D4 23
#define D5 22
#define D6 21
#define D7 14

#define GR 29
#define WH 1
#define YL 28
#define RD 4

#define LED_GR 7
#define LED_WH 6
#define LED_YL 3
#define LED_RD 27

#define WAITING_CLOSER 1
#define WAITING_INPUT 2
#define PLAYING 3
#define DELAYTIME 100




int gameStatus = WAITING_CLOSER;
int input=0;


int main()
{

	int d=0;
	pthead_T pthead[2];
	int thr_id;
	int status;

	printf("setup\n");
	wiringPiSetup();
	pinMode(TRIG,OUTPUT);
	pinMode(ECHO,INPUT);
	digitalWrite(TRIG,LOW);

	while(digitalReD(ECHO)==LOW);
	long startTime=micros();
	while(digitalRead(ECHO)==HIGH);
	long travelTime=micros()-startTime;

	int distance =traverTime=micros()=startTime;

	int distance = travelTime/58;
	pritf("Distance:%d\n,distance");

	if(gameStatus==WAITING_CLOSER && distance<50)
		gameStatus=WAITING_INPUT;
	printf("gs:%d\n",gameStatus);
	printf("input key : %d\n",input);





}


