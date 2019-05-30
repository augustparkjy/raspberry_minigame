#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <signal.h>

#define TRUE 1

#define TRIG 0
#define ECHO 2
#define SENSOR 29
#define LED 29

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
	//Send trig pulse
	digitalWrite(TRIG, HIGH);
        delayMicroseconds(1000);
					                                                   digitalWrite(TRIG, LOW);
        //Wait for echo start
	while(digitalRead(ECHO) == LOW);		                                   //Wait for echo end
        long startTime = micros();
        while(digitalRead(ECHO) == HIGH);
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
	pinMode(SENSOR, INPUT);
	pinMode(LED, OUTPUT);
	//pinMode(SENSOR, INPUT);

	while(1)
	{
//		signal(SIGINT, sig_handler);

		d = getCM();
		printf("Distance: %dcm\n", d);
		if(d <50){
			digitalWrite(LED, 100);	
			pinMode(SENSOR, INPUT);
			if(digitalRead(SENSOR) == 1)
				digitalWrite(LED, 0);
			//			printf("---------------%d\n", digitalRead(SENSOR));
			//if(digitalRead(SENSOR)==1){
			//	digitalWrite(LED, 0);
			//}
		}
		else{
		
			digitalWrite(LED, 0);	
//			printf("---------%d\n", digitalRead(SENSOR));
		}

	}	

											return 0;
                                                                                   }
