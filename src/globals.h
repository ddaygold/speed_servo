#include <Wire.h>
#include <Servo.h>
#define PINA 2
#define PINB 3
#define ADDRESS_PIN 13
volatile int speed = 0;
#define SPEED 0
volatile int pterm = 0;
#define PTERM 1
volatile int iterm = 0;
#define ITERM 2
volatile int dterm = 0;
#define DTERM 3
#define VOLTAGE_PIN A0
#define VOLTAGE 4

#define VICTOR_PIN 5
Servo victor;

volatile int ticks = 0;
int current_speed = 1510;

void processPID();
void receive(int incoming);
void writeVoltage();
void setParam(int incoming, volatile int * dest);
void spin();
