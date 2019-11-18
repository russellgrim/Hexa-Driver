/******************************************************************************
 * @File		LinearActuator.h
 * @Brief		Linear actuator class, used to drive the motor H-Bridge and track
 *				encoder steps.
 * @Date		17/11/2019 (Last Updated)
 * @Author(s)	William Bednall
 ******************************************************************************/
#ifndef LinearActuator_h
#define LinearActuator_h

#include "PinDefinitionsRev1_00.h"

class LinearActuator {

	static LinearActuator *instances[6];

	//Glue routines to connect ISR to a specific class instance
	static void HandleGlueRoutine_0 ();
	static void HandleGlueRoutine_1 ();
	static void HandleGlueRoutine_2 ();
	static void HandleGlueRoutine_3 ();
	static void HandleGlueRoutine_4 ();
	static void HandleGlueRoutine_5 ();

	private:
		const byte LinearActuatorID;
		const laFormat *LA;

		volatile int virtualPosition; //Updated by the ISR through a glue routine
		bool motorDirection;
		
	public:
		LinearActuator(const byte _LinearActuatorID);
		void EncoderInterruptHandler();
		int GetEncoderPos();
		void ResetEncoderPos();
		void SpinMotor(unsigned char dutyCycle, unsigned char direction);
		bool getMotorDir();
};

#endif
