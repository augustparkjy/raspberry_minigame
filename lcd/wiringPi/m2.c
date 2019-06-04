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
//#define SENSOR	 29
//#define LED	 29

//LCD
#define RS	25
#define E	24
#define D4	23
#define D5	22
#define D6	21
#define D7	14

//TOUCH
#define GR	29
#define WH	1
#define YL	28
#define RD	4

//LED
#define LED_GR	7
#define LED_WH	6
#define LED_YL	3
#define LED_RD	27

//game status
#define WAITING_CLOSER 	1
#define WAITING_INPUT 	2
#define PLAYING			3
#define RESULT_TIMEOUT		4
#define RESULT_MISS		5

#define DELAYTIME		200
#define MAX_STAGE		100

void setup();
void L_datetime(int lcd);
void *L_func(void* data);
int S_getCM();
void *T_func(void* data);
void *LED_func(void* data);
void game();
int getTouch();
int gameStatus = WAITING_CLOSER;

int output = 0;
int best = 0;
int now = 0;
int input=0;

//main
int main()
{
	int d = 0;
	pthread_t p_thread[2];
	int thr_id;
	int status;

	char p1[] = "thread_lcd";
	char p2[] = "thread_touch";
	char p3[] = "thread_led";

	//create thread
	thr_id = pthread_create(&p_thread[0], NULL, L_func, (void*)p1);
	if (thr_id <0)
	{
		perror("[lcd] thread create error : ");
		exit(0);
	}

	thr_id = pthread_create(&p_thread[1], NULL, T_func, (void*)p2);
	if (thr_id < 0)
	{
		perror("[touch] thread create error : ");
		exit(0);
	}

	thr_id = pthread_create(&p_thread[2], NULL, LED_func, (void*)p3);
	if (thr_id < 0)
	{
		perror("[led] thread create error : ");
		exit(0);
	}

	//setup devices
	setup();


	while (1)
	{
		
		if (gameStatus == WAITING_CLOSER)
		{
			d = S_getCM();
			sleep(1);
			if(d<=30)
				gameStatus = WAITING_INPUT;
		}
		
		if (gameStatus == PLAYING)
			game();

	}

	//exit
}

//wiringPi 
void setup()
{
	printf("setup\n");
	wiringPiSetup();
	pinMode(TRIG, OUTPUT);
	pinMode(ECHO, INPUT);

	//TRIG pin must start LOW
	digitalWrite(TRIG, LOW);

	//pinMode
	pinMode(GR, INPUT);
	pinMode(WH, INPUT);
	pinMode(YL, INPUT);
	pinMode(RD, INPUT);

	//LEDÀÇ pinMode
	pinMode(LED_GR, OUTPUT);
	pinMode(LED_WH, OUTPUT);
	pinMode(LED_YL, OUTPUT);
	pinMode(LED_RD, OUTPUT);

	//delay(30);
}

//sonar get distance(cm) function
int S_getCM()
{

	//Send trig pulse
	digitalWrite(TRIG, HIGH);
	delayMicroseconds(20);
	digitalWrite(TRIG, LOW);

	//wait for echo start
	while (digitalRead(ECHO) == LOW);
	long startTime = micros();
	while (digitalRead(ECHO) == HIGH);
	long travelTime = micros() - startTime;

	//Get distance in cm
	int distance = travelTime / 58;
	printf("Distance:  %d\n", distance);

	return distance;
}

//lcd function
void *L_func(void* data)
{
	char* t_name = (char*)data;
	printf("[%s is running]\n", t_name);
	sleep(1);

	int lcd;
	int count = 0;
	
	int s;
	int ms;
	//4bits
	lcd = lcdInit(2, 16, 4, RS, E, D4, D5, D6, D7, 0, 0, 0, 0);

	while (1)
	{
		printf("-------------gameStatus : %d\n", gameStatus);
		switch (gameStatus)
		{
			//come closer 
		case WAITING_CLOSER:
			lcdPosition(lcd, 4, 0);
			lcdPuts(lcd, "* COME *");
			lcdPosition(lcd, 2, 1);
			lcdPuts(lcd, "* CLOSER ! *");
			sleep(1);
			lcdClear(lcd);
			L_datetime(lcd);
			lcdClear(lcd);
			break;
	
		case WAITING_INPUT:
			while (count<3) {
				printf("cnt: %d\n", count);
				lcdPosition(lcd, 2, 0);
				lcdPuts(lcd, "PRESS ANY KEY");
				sleep(1);
				lcdClear(lcd);
				if (gameStatus == PLAYING)
				{
					count = 0;
					break;
				}
				lcdPosition(lcd, 2, 1);
				lcdPuts(lcd, "PRESS ANY KEY");
				sleep(1);
				lcdClear(lcd);
				count++;
				if (gameStatus == PLAYING)
				{
					count = 0;
					break;
				}
			}
			if (count == 3)
			{
				count = 0;
				gameStatus = WAITING_CLOSER;
				break;
			}
			break;
		
		case PLAYING:
	
			lcdPosition(lcd, 3, 0);
			lcdPuts(lcd, "* G A M E *");
			lcdPosition(lcd, 2, 1);
			lcdPuts(lcd, "* S T A R T *");
			sleep(1);
			lcdClear(lcd);

			s = 10;
			ms = 00;
			while (gameStatus == PLAYING)
			{
				lcdClear(lcd);

				lcdPosition(lcd, 1, 0);
				lcdPrintf(lcd, "BEST  %d  TIMER ", best);
				lcdPosition(lcd, 2, 1);
				lcdPrintf(lcd, "NOW  %d  [%d]", now, s);
				
				if (s == 0)
				{
					gameStatus = RESULT_TIMEOUT;
					break;
				}
		
				delay(1000);
				s--;
				lcdClear(lcd);
			
			}			
			break;
		
		case RESULT_MISS:
			if(best >= now)
			{
			for(int i = 0 ; i < 2 ; i ++)
			{
				lcdClear(lcd);
				lcdPosition(lcd, 6, 0);
				lcdPuts(lcd, "GG");
				lcdPosition(lcd, 0, 1);
				lcdPuts(lcd, " * TRY AGAIN! * ");
				sleep(1);
				lcdClear(lcd);
			}
			now =0;
			gameStatus = WAITING_CLOSER;
			}
			else{
				for(int i=0;i<2;i++)
				{
					lcdClear(lcd);
					lcdPosition(lcd, 6, 0);
					lcdPuts(lcd, "GG");
					lcdPosition(lcd, 0, 1);
					lcdPuts(lcd, " * NEW RECORD* *");
					delay(500);
					lcdClear(lcd);
					lcdPosition(lcd, 6, 0);
					lcdPuts(lcd, "GG");
					lcdPosition(lcd, 0, 1);
					lcdPuts(lcd, "* *NEW RECORD * ");
					delay(500);
					lcdClear(lcd);
				}
				best = now;
				now = 0;
				gameStatus = WAITING_CLOSER;
			}
			break;
		case RESULT_TIMEOUT:
	
			if (best >= now)
			{
				for (int i = 0; i < 2; i++)
				{
					lcdClear(lcd);
					lcdPosition(lcd, 1, 0);
					lcdPuts(lcd, " * TIME OUT * ");
					lcdPosition(lcd, 0, 1);
					lcdPuts(lcd, " * TRY AGAIN! * ");
					sleep(1);
					lcdClear(lcd);
				}
				now = 0;
				gameStatus = WAITING_CLOSER;
			}
		
			else
			{
				for (int i = 0; i < 2; i++)
				{	
					lcdClear(lcd);
					lcdPosition(lcd, 1, 0);
					lcdPuts(lcd, " * TIME OUT * ");
					lcdPosition(lcd, 0, 1);
					lcdPuts(lcd, "* *NEW RECORD * ");
					delay(500);
					lcdClear(lcd);
					lcdPosition(lcd, 1, 0);
					lcdPuts(lcd, " * TIME OUT * ");
					lcdPosition(lcd, 0, 1);
					lcdPuts(lcd, " * NEW RECORD* *");
					delay(500);
					lcdClear(lcd);
				}
				best = now;
				now = 0;
				gameStatus = WAITING_CLOSER;
			}
			break;
		}
	}
}

void *T_func(void* data)
{
	char* t_name = (char*)data;
	printf("[%s is running]\n", t_name);
	sleep(1);


	
	while (1)
	{

	
		
		if (gameStatus == WAITING_INPUT)
		{
			
			
			if (digitalRead(GR) == 1) input = 1;
			else if (digitalRead(WH) == 1) input = 2;
			else if (digitalRead(YL) == 1) input = 3;
			else if (digitalRead(RD) == 1) input = 4;
		
			if (input>0 && gameStatus == WAITING_INPUT){
				gameStatus = PLAYING;

		
			input = 0;}
		}
		
	}
}

void *LED_func(void* data)
{
	char* t_name = (char*)data;
	printf("[%s is running]\n", t_name);
	sleep(1);

	
	while (1)
	{
		delay(100);
		if (input == 1 || output == 1)
		{
			digitalWrite(LED_GR, 100);
			delay(100);
			digitalWrite(LED_GR, 0);

                        delay(100);	
		}
		else if (input == 2 || output == 2)
		{
			digitalWrite(LED_WH, 100);
			delay(100);
			digitalWrite(LED_WH, 0);
			delay(100);
	
		}
		else if (input == 3 || output == 3)
		{
			digitalWrite(LED_YL, 100);
			delay(100);
			digitalWrite(LED_YL, 0);
			delay(100);

		}
		else if (input == 4 || output == 4)
		{
			digitalWrite(LED_RD, 100);
			delay(100);
			digitalWrite(LED_RD, 0);
			delay(100);
	
		}
		output=0;
	}
}

void L_datetime(int lcd) {
	while (1) {
		time_t timer;
		char buffer_date[26];
		char buffer_time[26];
		struct tm* tm_info;


		timer = time(NULL);

	
		timer += 28800;


		tm_info = localtime(&timer);

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

void game()
{
	srand(time(NULL));

	int rnum[MAX_STAGE];

	now = 0;

	sleep(1);

	while (gameStatus == PLAYING)
	{
	
		delay(200);

		for (int i = 0; i < now+1; i++)
		{
			rnum[i] = (rand() % 4) + 1;
		}
		for(int h=0; h<now+1;h++){
			output = rnum[h];
			delay(500);

		}
		output=0;

	
		for(int j = 0; j < now +1 ;j++)
		{
			input = getTouch();
			
			delay(500);
			
	
			if(rnum[j] != (input))
			
			{
		
			
				input =0;
				gameStatus = RESULT_MISS;

				return;
			}else{
			
		
			
			}
			
			output = 0;
			input = 0;
			delay(100);
		}
	

		now++;
	}

	return;
}
int getTouch(){

	while(1)
	{
	if(digitalRead(GR)==1){      
		return 1;
	}
	else if(digitalRead(WH)==1){ return  2;}
	else if(digitalRead(YL)==1){ return  3;}
	else if(digitalRead(RD)==1){ return 4;}
	}
}
