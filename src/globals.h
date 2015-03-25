#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>
#include <EEPROM.h>
#define PINA 2
#define PINB 3
#define ADDRESS_PIN 13
volatile double speed = 0;
#define SPEED 0
volatile double pterm = 0.0;
#define PTERM 1
volatile double iterm = 0.0;
#define ITERM 2
volatile double dterm = 0.0;
#define DTERM 3
#define VOLTAGE_PIN A0
#define VOLTAGE 4
#define ESTOP 5

#define VICTOR_PIN 5
Servo victor;

volatile double ticks = 0;
int last_ticks = 0;
int current_speed = 1510;

double controller_output = 0;
PID controller(&ticks, &controller_output, &speed,
	       pterm,iterm,dterm,
	       DIRECT);
long last_tuning_update = 0;

void processPID();
void receive(int incoming);
void writeTicks();
void setParam(int incoming, volatile double * dest);
void setParam(int incoming, volatile int * dest);
void spin();
