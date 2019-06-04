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
#define RESULT			4
#define RESULT_WIN		5
#define RESULT_LOSE		6

#define DELAYTIME		200
#define MAX_STAGE		100

void setup();
void L_datetime(int lcd);
void *L_func(void* data);
int S_getCM();
void *T_func(void* data);
void *LED_func(void* data);
void game();

int gameStatus = WAITING_CLOSER;

int input, output = 0;
int best = 0;
int now = 0;

//main함수에서 
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
		//유저의 접근 대기 상태일 때, 1초 간격으로 sonar 센서를 이용해 물체와 거리 감지
		if (gameStatus == WAITING_CLOSER)
		{
			d = S_getCM();
			sleep(1);
		}
		
		//감지된 거리가 50cm 이내일 때, 게임 시작을 위한 입력 대기 상태로 전환
		if (d<=50) 
			gameStatus = WAITING_INPUT;
		
		//게임 중 상태일 때, 게임 함수 호출
		if (gameStatus == PLAYING)
			game();

	}

	//exit thread
	//pthread_join(p_thread[0], (void**)&status);
	//pthread_join(p_thread[1], (void**)&status);

}

//wiringPi 셋업
void setup()
{
	printf("setup\n");
	wiringPiSetup();
	pinMode(TRIG, OUTPUT);
	pinMode(ECHO, INPUT);

	//TRIG pin must start LOW
	digitalWrite(TRIG, LOW);

	//터치 센서 pinMode
	pinMode(GR, INPUT);
	pinMode(WH, INPUT);
	pinMode(YL, INPUT);
	pinMode(RD, INPUT);

	//LED의 pinMode
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
	//4bits 전송 lcd 방식, gpio 부족
	lcd = lcdInit(2, 16, 4, RS, E, D4, D5, D6, D7, 0, 0, 0, 0);

	while (1)
	{
		switch (gameStatus)
		{
			//come closer 문구와 오늘 날짜와 시간을 번갈아서 출력
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
			//유저가 가까이 있음이 확인되면, 게임 할 의사가 있는지 확인
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
			//일정 시간동안 입력이 없으면 게임을 할 의사가 없는 것으로 판단하고 다시 유저 접근 대기 상태로 
			if (count == 3)
			{
				count = 0;
				gameStatus = WAITING_CLOSER;
				break;
			}
			//게임 중
		case PLAYING:
			//게임이 시작됨을 알림
			lcdPosition(lcd, 4, 0);
			lcdPuts(lcd, "■□■ G A M E □■□■");
			lcdPosition(lcd, 3, 1);
			lcdPuts(lcd, "□■ S T A R T □■□");
			delay(500);
			lcdClear(lcd);
			//게임이 진행되는 60초를 카운트하고, 최고기록과 현재기록 그리고 남은 시간을 출력
			s = 60;
			ms = 00;
			while (gameStatus == PLAYING)
			{
				lcdPosition(lcd, 1, 0);
				lcdPrintf(lcd, " BEST %-02d  TIMER", best);
				lcdPosition(lcd, 2, 1);
				lcdPrintf(lcd, "NOW %-02d  %-02d:%-02d", now, s, ms);

				//시간이 다 되면 게임 결과 상태로 전환
				if (s == 0 && ms == 0)
				{
					gameStatus = RESULT;
					break;
				}
				//0.01초 단위로 카운트
				delay(10);
				lcdClear(lcd);
				if (ms != 0)
					ms--;
				else
				{
					ms = 99;
					s--;
				}
			}			
			break;
			//게임 결과
		case RESULT:
			//기록 갱신에 실패했을 때
			if (best >= now)
			{
				for (int i = 0; i < 2; i++)
				{
					lcdPosition(lcd, 1, 0);
					lcdPuts(lcd, "T I M E O U T ■");
					lcdPosition(lcd, 3, 1);
					lcdPuts(lcd, "■□ TRY AGAIN! ■□");
					sleep(1);
					lcdClear(lcd);
				}
				now = 0;
				gameStatus = WAITING_CLOSER;
			}
			//기록 갱신에 성공했을 때
			else
			{
				for (int i = 0; i < 2; i++)
				{
					lcdPosition(lcd, 1, 0);
					lcdPuts(lcd, "T I M E O U T ■");
					lcdPosition(lcd, 3, 1);
					lcdPuts(lcd, "■ NEW RECORD! □");
					delay(500);
					lcdClear(lcd);
					lcdPosition(lcd, 1, 0);
					lcdPuts(lcd, "T I M E O U T □");
					lcdPosition(lcd, 3, 1);
					lcdPuts(lcd, "□ NEW RECORD! ■");
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

int *T_func(void* data)
{
	char* t_name = (char*)data;
	printf("[%s is running]\n", t_name);
	sleep(1);

	//pinMode(GR, INPUT);
	//pinMode(WH, INPUT);
	//pinMode(YL, INPUT);
	//pinMode(RD, INPUT);

	while (1)
	{
		//입력 대기 상태 또는 게임 중 상태일 때만 입력이 유효하도록 함.
		if (gameStatus == WAITING_INPUT || gameStatus == PLAYING)
		{
			//초, 흰, 노, 빨 각각의 터치 센서를 입력하면 해당되는 입력이 인식됨.
			if (digitalRead(GR) == 1) input = 1;
			else if (digitalRead(WH) == 1) input = 2;
			else if (digitalRead(YL) == 1) input = 3;
			else if (digitalRead(RD) == 1) input = 4;

			//입력 대기 상태일 때, 입력이 감지되면 게임 중 상태로 전환
			if (input>0 && gameStatus == WAITING_INPUT)
				gameStatus = PLAYING;

			//게임 중 입력일 경우 터치 충돌을 방지하기 위해 일정 delay를 준다.
			if (gameStatus == PLAYING)
				delay(DELAYTIME);

			//다시 무입력 상태로 
			input = 0;
		}
	}
}

void *LED_func(void* data)
{
	char* t_name = (char*)data;
	printf("[%s is running]\n", t_name);
	sleep(1);

	/*pinMode(LED_GR, OUTPUT);
	pinMode(LED_WH, OUTPUT);
	pinMode(LED_YL, OUTPUT);
	pinMode(LED_RD, OUTPUT);*/

	//터치 입력에 해당하는 led가 켜지거나 게임 진행을 위해 터치와 독립적으로 켜진다.
	while (1)
	{
		if (input == 1 || output == 1)
		{
			digitalWrite(LED_GR, 100);
			delay(DELAYTIME);
			digitalWrite(LED_GR, 0);
		}
		else if (input == 2 || output == 2)
		{
			digitalWrite(LED_WH, 100);
			delay(DELAYTIME);
			digitalWrite(LED_WH, 0);
		}
		else if (input == 3 || output == 3)
		{
			digitalWrite(LED_YL, 100);
			delay(DELAYTIME);
			digitalWrite(LED_YL, 0);
		}
		else if (input == 4 || output == 4)
		{
			digitalWrite(LED_RD, 100);
			delay(DELAYTIME);
			digitalWrite(LED_RD, 0);
		}
	}
}

void L_datetime(int lcd) {
	while (1) {
		time_t timer;
		char buffer_date[26];
		char buffer_time[26];
		struct tm* tm_info;


		timer = time(NULL);

		//set time to KST
		timer += 28800;

		//time(&timer);
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
	//최초 외워야 할 번호는 3개
	int stage = 3;
	int j = 0;

	while (gameStatus == PLAYING)
	{
		//외워야 할 번호를 랜덤 생성하고 번호에 해당하는 led를 output 값을 통해 점등
		for (int i = 0; i < stage; i++)
		{
			rnum[i] = (rand() % 4) + 1;
			output = rnum[i];
			delay(DELAYTIME);
		}

		//암기 도전
		while (j < stage)
		{
			//유저 입력이 있을 때 까지 대기
			if (input == 0)
				continue;
			else
			{
				//입력한 값이 제시된 순서에 해당하지 않으면 게임은 결과 상태로
				if (rnum[j] != input)
				{
					//현재까지 클리어한 stage = stage - 3
					now = stage - 3;
					gameStatus = RESULT;
					//함수 종료
					return;
				}
				//입력한 값이 일치하면 계속 진행
				else
				{
					j++;
				}
			}
		}
		
		j = 0;
		stage++;
	}

	return;
}