#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>
#include <EEPROM.h>
//Pins for the Quad encoder
#define PINA 2
#define PINB 3
//Pin to specify the i2c address
//High is address 1, low is address 0
#define ADDRESS_PIN 13
volatile double speed = 0;
volatile double pterm = 0.0;
volatile double iterm = 0.0;
volatile double dterm = 0.0;
enum command{
	SPEED   = 0,
	PTERM   = 1,
	ITERM   = 2,
	DTERM   = 3,
	VOLTAGE = 4,
	ESTOP   = 5
};
#define VOLTAGE_PIN A0

#define VICTOR_PIN 5
Servo victor;

volatile double ticks = 0;
int last_ticks = 0;

  /*
    If the Victor motor controller PWM width extremes are 870 microsecond and 2140 microseconds, with
    a center at 1510 microseconds, we have a width of 635 microseconds
  */
#define PWM_CENTER 1510
#define PWM_WIDTH  635
int current_speed = 0;
double controller_output = PWM_CENTER;
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
void printDebug();
