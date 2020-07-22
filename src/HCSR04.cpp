/*
  HCSR04 - Library for arduino, for HC-SR04 ultrasonic distance sensor.
  Created by Dirk Sarodnick, 2020.
*/

#include "Arduino.h"
#include "HCSR04.h"

HCSR04Sensor::HCSR04Sensor() {}

void HCSR04Sensor::begin(int triggerPin, int* echoPins) {
	this->triggerPin = triggerPin;
	this->echoPins = echoPins;
	this->timeout = 100000;
	this->unlock = false;
	
	size_t size = sizeof(echoPins)/sizeof(echoPins[0]);
	this->triggerTimes = new unsigned long[size];
	this->echoTimes = new unsigned long[size];
}

void HCSR04Sensor::begin(int triggerPin, int* echoPins, int timeout, bool unlock) {
	this->triggerPin = triggerPin;
	this->timeout = timeout;
	this->unlock = unlock;
	
	size_t size = sizeof(echoPins)/sizeof(echoPins[0]);
	this->triggerTimes = new unsigned long[size];
	this->echoTimes = new unsigned long[size];
	
	this->echoPins = new int[size];
	for (int i = 0; i < size; i++) {
		this->echoPins[i] = digitalPinToInterrupt(echoPins[i]);
	}
}

long* HCSR04Sensor::measureMicroseconds() {
	size_t size = sizeof(this->echoPins)/sizeof(this->echoPins[0]);
	
	// Unlock sensors that are possibly in a locked state, if this feature is enabled.
	if (this->unlock) this->unlockSensors();
	
	// Make sure that trigger pin is LOW.
	digitalWrite(triggerPin, LOW);
	delayMicroseconds(2);
	
	// Hold trigger for 10 microseconds, which is signal for sensor to measure distance.
	digitalWrite(triggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(triggerPin, LOW);
	
	// Attach interrupts to echo pins for the starting point
	for (int i = 0; i < size; i++) {
		switch (i) {
			case 0: attachInterrupt(this->echoPins[i], &triggerInterrupt0, RISING); break;
			case 1: attachInterrupt(this->echoPins[i], &triggerInterrupt1, RISING); break;
			case 2: attachInterrupt(this->echoPins[i], &triggerInterrupt2, RISING); break;
			case 3: attachInterrupt(this->echoPins[i], &triggerInterrupt3, RISING); break;
			case 4: attachInterrupt(this->echoPins[i], &triggerInterrupt4, RISING); break;
			case 5: attachInterrupt(this->echoPins[i], &triggerInterrupt5, RISING); break;
			case 6: attachInterrupt(this->echoPins[i], &triggerInterrupt6, RISING); break;
			case 7: attachInterrupt(this->echoPins[i], &triggerInterrupt7, RISING); break;
			case 8: attachInterrupt(this->echoPins[i], &triggerInterrupt8, RISING); break;
			case 9: attachInterrupt(this->echoPins[i], &triggerInterrupt9, RISING); break;
		}
	}
	
	// Wait until all echos are returned or timed out.
	while(1) {
		delayMicroseconds(1);
		
		bool finished = true;
		bool waiting = true;
		
		long currentMicros = micros();
		for (int i = 0; i < size; i++) {
			waiting &= currentMicros - this->triggerTimes[i] < this->timeout;
			
			if (this->triggerTimes[i] > 0) {
				switch (i) {
					case 0: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt0, FALLING); break;
					case 1: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt1, FALLING); break;
					case 2: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt2, FALLING); break;
					case 3: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt3, FALLING); break;
					case 4: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt4, FALLING); break;
					case 5: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt5, FALLING); break;
					case 6: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt6, FALLING); break;
					case 7: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt7, FALLING); break;
					case 8: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt8, FALLING); break;
					case 9: detachInterrupt(this->echoPins[i]); attachInterrupt(this->echoPins[i], &echoInterrupt9, FALLING); break;
				}
			} else finished &= false;
			
			if (this->echoTimes[i] > 0) {
				switch (i) {
					case 0: detachInterrupt(this->echoPins[i]); break;
					case 1: detachInterrupt(this->echoPins[i]); break;
					case 2: detachInterrupt(this->echoPins[i]); break;
					case 3: detachInterrupt(this->echoPins[i]); break;
					case 4: detachInterrupt(this->echoPins[i]); break;
					case 5: detachInterrupt(this->echoPins[i]); break;
					case 6: detachInterrupt(this->echoPins[i]); break;
					case 7: detachInterrupt(this->echoPins[i]); break;
					case 8: detachInterrupt(this->echoPins[i]); break;
					case 9: detachInterrupt(this->echoPins[i]); break;
				}
			} else finished &= false;
		}
		
		if (!waiting || finished) break;
	}
	
	// Determine the durations of each sensor.
	long* results = new long[size];
	for (int i = 0; i < size; i++) {
		results[i] = this->echoTimes[i] - this->triggerTimes[i];
		
		this->triggerTimes[i] = 0;
		this->echoTimes[i] = 0;
	}

	return results;
}

double* HCSR04Sensor::measureDistanceMm() {
	return measureDistanceMm(19.307);
}

double* HCSR04Sensor::measureDistanceMm(float temperature) {
	double speedOfSoundInMmPerMs = 331.3 + 0.0606 / 1000 * temperature; // Cair ≈ (331.3 + 0.606 ⋅ ϑ) m/s
	size_t size = sizeof(this->echoPins)/sizeof(this->echoPins[0]);
	long* results = measureMicroseconds();
	
	// Calculate the distance in mm for each result.
	for (int i = 0; i < size; i++) {
		double distanceCm = results[i] / 2.0 * speedOfSoundInMmPerMs;
		if (distanceCm == 0 || distanceCm > 400) {
			results[i] = -1.0;
		} else {
			results[i] = distanceCm;
		}
	}
}

double* HCSR04Sensor::measureDistanceCm() {
	return measureDistanceCm(19.307);
}

double* HCSR04Sensor::measureDistanceCm(float temperature) {
	double speedOfSoundInCmPerMs = 331.3 + 0.606 / 1000 * temperature; // Cair ≈ (331.3 + 0.606 ⋅ ϑ) m/s
	size_t size = sizeof(this->echoPins)/sizeof(this->echoPins[0]);
	long* results = measureMicroseconds();
	
	// Calculate the distance in cm for each result.
	for (int i = 0; i < size; i++) {
		double distanceCm = results[i] / 2.0 * speedOfSoundInCmPerMs;
		if (distanceCm == 0 || distanceCm > 400) {
			results[i] = -1.0;
		} else {
			results[i] = distanceCm;
		}
	}
}

void HCSR04Sensor::unlockSensors() {
	bool hasLocked = false;
	size_t size = sizeof(this->echoPins)/sizeof(this->echoPins[0]);

	// Check if any sensor is in a locked state and unlock it if necessary.
	for (int i = 0; i < size; i++) {
		if (digitalRead(this->echoPins[i]) == LOW) continue;
		pinMode(this->echoPins[i], OUTPUT);
		digitalWrite(this->echoPins[i], LOW);
		hasLocked = true;
	}
	
	if (hasLocked) delay(50);

	// Revert the pinMode after potential unlocking.
	for (int i = 0; i < size; i++) {
		pinMode(this->echoPins[i], INPUT);
	}
	
	if (hasLocked) delay(50);
}

void HCSR04Sensor::triggerInterrupt(int index) {
	this->triggerTimes[index] = micros();
}

void HCSR04Sensor::echoInterrupt(int index) {
	this->echoTimes[index] = micros();
}

void HCSR04Sensor::triggerInterrupt0() { HCSR04.triggerInterrupt(0); }
void HCSR04Sensor::triggerInterrupt1() { HCSR04.triggerInterrupt(1); }
void HCSR04Sensor::triggerInterrupt2() { HCSR04.triggerInterrupt(2); }
void HCSR04Sensor::triggerInterrupt3() { HCSR04.triggerInterrupt(3); }
void HCSR04Sensor::triggerInterrupt4() { HCSR04.triggerInterrupt(4); }
void HCSR04Sensor::triggerInterrupt5() { HCSR04.triggerInterrupt(5); }
void HCSR04Sensor::triggerInterrupt6() { HCSR04.triggerInterrupt(6); }
void HCSR04Sensor::triggerInterrupt7() { HCSR04.triggerInterrupt(7); }
void HCSR04Sensor::triggerInterrupt8() { HCSR04.triggerInterrupt(8); }
void HCSR04Sensor::triggerInterrupt9() { HCSR04.triggerInterrupt(9); }

void HCSR04Sensor::echoInterrupt0() { HCSR04.echoInterrupt(0); }
void HCSR04Sensor::echoInterrupt1() { HCSR04.echoInterrupt(1); }
void HCSR04Sensor::echoInterrupt2() { HCSR04.echoInterrupt(2); }
void HCSR04Sensor::echoInterrupt3() { HCSR04.echoInterrupt(3); }
void HCSR04Sensor::echoInterrupt4() { HCSR04.echoInterrupt(4); }
void HCSR04Sensor::echoInterrupt5() { HCSR04.echoInterrupt(5); }
void HCSR04Sensor::echoInterrupt6() { HCSR04.echoInterrupt(6); }
void HCSR04Sensor::echoInterrupt7() { HCSR04.echoInterrupt(7); }
void HCSR04Sensor::echoInterrupt8() { HCSR04.echoInterrupt(8); }
void HCSR04Sensor::echoInterrupt9() { HCSR04.echoInterrupt(9); }

HCSR04Sensor HCSR04;
