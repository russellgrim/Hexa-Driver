/******************************************************************************
 * @File		Controller.cpp
 * @Brief		DC motor control systems are implemented here 
 * @Date		20/11/2019 (Last Updated)
 * @Author(s)	William Bednall
 ******************************************************************************/
#include <Arduino.h>
#include "Controller.h"

//Controller::Controller(const laFormat *_LA) : LinearActuator(_LA){}
Controller::Controller(const byte _LinearActuatorID) : LinearActuator(_LinearActuatorID){
	controllerMode = 0;
	posGain = 0.5;
	pos_Setpoint = 800.0;
	velDesired = 0.0;
	velGain = 0.5;
	velIntGain = 0.2;
	timeKeep = millis();
	timeSinceUpdate = millis();
	sampleRate = 10; //Set default sample rate to 10 ms = 100 Hz
	togglePosVel = false;
	togglePIdebug = false;

	//Run time controll vars - Sweep mode
	dirA_runTime = 1000;
	dirB_runTime = 1000;
	duty_runTime = 75;

	//Run time controll vars - Single mode
	durationSingle_runTime = 500;
	flagSingle_runTime = false;

	sweepMS_runTime = millis();
	singleMS_runTime = millis();
}

void Controller::closedSpinTest(){
	if (GetEncoderPos() <= 0){
		SpinMotor(75, dirB); //Change turning to head towards end point
	}

	//at end point
	if (GetEncoderPos() >= 500){
		SpinMotor(75, dirA); //Change turning to head towards start point
	}

	if (getMotorDir() == true && GetEncoderPos() < 50){ SpinMotor(50, dirA); } //deceleration to start point
	if (getMotorDir() == true && GetEncoderPos() < 20){ SpinMotor(48, dirA); } //deceleration to start point
	if (getMotorDir() == true && GetEncoderPos() <= 5){ SpinMotor(47, dirA); } //deceleration to start point

	if (getMotorDir() == false && GetEncoderPos() > 450){ SpinMotor(50, dirB); } //deceleration to end point
	if (getMotorDir() == false && GetEncoderPos() > 480){ SpinMotor(48, dirB); } //deceleration to end point
	if (getMotorDir() == false && GetEncoderPos() >= 495){ SpinMotor(47, dirB); } //deceleration to end point

	//Serial.print(GetEncoderPos());
	//Serial.print(", ");
}

void Controller::setPosGain(float gain){
	posGain = gain;
}

void Controller::setVelGain(float gain){
	velGain = gain;
}

void Controller::setVelIntGain(float gain){
	velIntGain = gain;
}

void Controller::setPoint(float setpoint){
	pos_Setpoint = setpoint;
}

void Controller::setSampleRate(unsigned int rate){
	sampleRate = rate;
}

//Toggle position and velocity streaming
void Controller::streamPosVel(bool toggle){
	togglePosVel = toggle;
}

void Controller::update(){
	if (enableLA == true){
		//Controller mode selection
		if (controllerMode == 1){
			position();
		} else if(controllerMode == 2){
			runTimeSweep();
		} else if(controllerMode == 3){
			runTimeSingleUpdate();
		} else {
			SpinMotor(0, dirB);
		}

		//Velocity sampling
		if (millis() - timeSinceUpdate > sampleRate){
			timeSinceUpdate = millis();
			VelocityUpdate();
			if (togglePosVel){
				Serial.print("s,");
				Serial.print(millis());
				Serial.print(",");
				Serial.print(GetEncoderPos());
				Serial.print(",");
				Serial.println(GetEncoderRPM());	
			}
		}
	} else {
		SpinMotor(0, dirB);
	}
}

//The position controller is a P loop with a single proportional gain.
void Controller::position(){

	//Position error calc
	float posError = pos_Setpoint - GetEncoderPos();

	//Position proportional calc
	velDesired += posGain * posError;

	//---

	/*** Velocity PI controller not yet working correctly

	//Velocity error calc
	float velError = velDesired - GetEncoderRPM();

	//Velocity proportional calc
	outDesired += velGain * velError;
	
	//Velocity integral calc
	outDesired += velIntGain * velError * GetEncoderRPM();

	//Limit the ouput velocity to 75/255 duty
    float vel_Limit = 75.0;
    if (outDesired > vel_Limit) outDesired = vel_Limit;
    if (outDesired < -vel_Limit) outDesired = -vel_Limit;

    //Set motor direction
	uint8_t dirSet = 0;
	if (outDesired < 0){
		dirSet = 1;
	} else {
		dirSet = 2;
	}

	SpinMotor(abs(velDesired), dirSet);

	*/

	//Temp to skip velocity PI stage

	//Limit the ouput velocity to 75 duty, Duty cycle can be 0-255
	float vel_Limit = 75.0;
	if (velDesired > vel_Limit) velDesired = vel_Limit;
	if (velDesired < -vel_Limit) velDesired = -vel_Limit;

	//Set motor direction
	uint8_t dirSet = 0;
	if (velDesired < 0){
		dirSet = 1;
	} else {
		dirSet = 2;
	}

	if (togglePIdebug){
		//Limit print speed
		if (millis() - timeKeep > 100){
			Serial.print(GetEncoderPos());
			Serial.print(" ");
			Serial.print(velDesired);
			Serial.print(" ");
			Serial.print(outDesired);
			Serial.print(" ");
			Serial.println(dirSet);
			timeKeep = millis();
		}
	}

	SpinMotor(abs(velDesired), dirSet);


}

//Simple controller to switch motor direction based on a time delay
void Controller::runTimeSweep(){
	if (getMotorDir() == true){
		if (millis() - sweepMS_runTime > dirA_runTime){
			SpinMotor(duty_runTime, 2);
			sweepMS_runTime = millis();
			//Serial.println(GetEncoderPos());
		}

	} else if(getMotorDir() == false) {
		if (millis() - sweepMS_runTime > dirB_runTime){
			SpinMotor(duty_runTime, 1);
			sweepMS_runTime = millis();
			//Serial.println(GetEncoderPos());
		}

	}
}

void Controller::runTimeSingleFire(uint16_t duration, uint8_t dir, uint8_t duty){
	durationSingle_runTime = duration;
	flagSingle_runTime = true; //Raise a single shot flag
	sweepMS_runTime = millis();
	SpinMotor(duty, dir);
}

void Controller::runTimeSingleUpdate(){
	if (flagSingle_runTime){ //Check if single command is currently running motor
		if (millis() - sweepMS_runTime > durationSingle_runTime){
			SpinMotor(0, dirB); //Stop motor
			flagSingle_runTime = false; //drop flag
		}
	}
}

Controller* idToInstance(uint8_t LA_ID){
	if (LA_ID == 0){
		return &LA0;
	} else if (LA_ID == 1){
		return &LA1;
	} else if (LA_ID == 2){
		return &LA2;
	} else if (LA_ID == 3){
		return &LA3;
	} else if (LA_ID == 4){
		return &LA4;
	} else if (LA_ID == 5){
		return &LA5;
	}
	//return LA0.getInstance(LA_ID);
}

//Instantiate 6 Linear Actuator Objects and Allocate Their IO
Controller LA0(0);
Controller LA1(1);
Controller LA2(2);
Controller LA3(3);
Controller LA4(4);
Controller LA5(5);