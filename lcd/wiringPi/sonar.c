#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define TRUE 1

#define TRIG 0
#define ECHO 2

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
					
int getCM() 
{
	printf("getCM\n");
	//Send trig pulse
	digitalWrite(TRIG, HIGH);
        delayMicroseconds(20);
					                                                digitalWrite(TRIG, LOW);
	printf("before while\n");
        //Wait for echo start
	while(digitalRead(ECHO) == LOW);					
		      printf("after 1 while\n");                                                                  //Wait for echo end
        long startTime = micros();
        while(digitalRead(ECHO) == HIGH);
	printf("a 2 while\n");
        long travelTime = micros() - startTime;
					
        //Get distance in cm
        int distance = travelTime / 58;
	printf("D %d\n", distance);
					
        return distance;
}
		
int main()
{
	int d = 0;
	setup();
											while(1)									{											d = getCM();
		printf("Distance: %dcm\n", d);
		if(d <30)
			break;
	}			                                                                                                                                                return 0;
                                                                                                                          }
