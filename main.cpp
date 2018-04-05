#include <QTRSensors.h>
#define M1A 3
#define M1B 5
#define M2A 6
#define M2B 9
#define kProportional 7
#define kDerivative 40
#define kIntegral 0.001
#define kSpeedReduction 0
#define baseSpeed 200
#define fastTurnSpeed 230
#define sensorTreshold 700

QTRSensorsAnalog qtra((unsigned char[]){0, 1, 2, 3, 4, 5}, 6);
unsigned int sensorValues[6];

void go(int left, int right) {
	if (right >= 0) {
		analogWrite(M1A, 0);
		analogWrite(M1B, right);
	}

	if (right < 0) {
		analogWrite(M1A, -right);
		analogWrite(M1B, 0);
	}
	if (left >= 0) {
		analogWrite(M2A, 0);
		analogWrite(M2B, left);
	}

	if (left < 0) {
		analogWrite(M2A, -left);
		analogWrite(M2B, 0);
	}
}

void setup() {
	for (int i = 1; i <= 100; i++) {
		qtra.calibrate();
		delay(20);
	}
}

int proportional;
int derivative;
int integral;

int integralIndex = 0;
int optimizedIntegral[13];
int lastError;

int speedReduction;
int speedReductionIndex = 0;
int optimizedSpeedReduction[13];

void loop() {
	int leftMotorSpeed;
	int rightMotorSpeed;
	int position = qtra.readLine(sensorValues);
	if (position == 0) {
		leftMotorSpeed = -fastTurnSpeed;
		rightMotorSpeed = fastTurnSpeed;
	} else if (position == 5000) {
		leftMotorSpeed = fastTurnSpeed;
		rightMotorSpeed = -fastTurnSpeed;
	} else {
		int error = 2500 - position;
		proportional = error;
		derivative = error - lastError;

		int toRemoveIntegral = optimizedIntegral[(++integralIndex) % 10];
		optimizedIntegral[integralIndex % 10] = error;
		integral = integral + error - toRemoveIntegral;

		int power_difference = proportional * kProportional +
			integral * kIntegral + derivative * kDerivative;

		int toRemoveSpeedReduction =
			optimizedSpeedReduction[(++speedReductionIndex) % 10];
		optimizedSpeedReduction[speedReductionIndex % 10] = abs(error);
		speedReduction = speedReduction + abs(error) - toRemoveSpeedReduction;

		if (power_difference > baseSpeed)
			power_difference = baseSpeed;
		else if (power_difference < -baseSpeed)
			power_difference = -baseSpeed;

		if (power_difference > 0) {
			leftMotorSpeed = baseSpeed - speedReduction * kSpeedReduction;
			rightMotorSpeed =
				baseSpeed - power_difference - speedReduction * kSpeedReduction;
		} else {
			leftMotorSpeed =
				baseSpeed + power_difference - speedReduction * kSpeedReduction;
			rightMotorSpeed = baseSpeed - speedReduction * kSpeedReduction;
		}
	}
	go(leftMotorSpeed, rightMotorSpeed);
}
