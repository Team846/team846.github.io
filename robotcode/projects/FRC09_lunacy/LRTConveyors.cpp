#include "LRTConveyors.h"
#include "Victor.h"
#include "Jaguar.h"
#include "Relay.h"
#include "LRTConnections.h"
#include "LRTDriverControls.h"

LRTConveyors::LRTConveyors() {
	printf("Initializing LRTConveyers\n");
	m_pickupRear = new Jaguar(LRTConnections::kPWMPickupRear);
	m_pickupFront = new Relay(LRTConnections::kRelayPickupFront);

	m_liftFront = new Relay(LRTConnections::kRelayLiftFront);
	m_liftRear = new Relay(LRTConnections::kRelayLiftRear);

	m_shooter = new Jaguar(LRTConnections::kPWMShooter);
	
	m_encraw = new DigitalInput(LRTConnections::kDIOShooterEncoder);
	m_shooterEncoder = new Counter(m_encraw);
	m_shooterEncoder->SetMaxPeriod(1/kShooterMinRPS);
	m_shooterEncoder->Start();

	Init();
	
	printf("Finishing LRTConveyers\n");
}
LRTConveyors::~LRTConveyors() {
	delete m_pickupRear;
	delete m_pickupFront;
	delete m_liftFront;
	delete m_liftRear;
	delete m_shooter;
	delete m_encraw;
	delete m_shooterEncoder;
}

void LRTConveyors::SetPickup(int state){
	m_pickupOn=state;
}
void LRTConveyors::SetLift(int state){
	m_liftOn=state;
}


void LRTConveyors::SetAllOff(){
	SetPickup(0);
	SetLift(0);
	SetShooterRaw(0);
}

void LRTConveyors::SetShooterRaw(float pwm){
	m_shooterSpeed=pwm;
}

void LRTConveyors::SetShooter(float speed){
//	m_shooterSpeed=speed;
	SetShooter(speed, kShooterGain);
}

void LRTConveyors::SetShooter(float speed, float gain){
	static int count = 0;
//	static float savedValue = 0;
//	float actualSpeed = ReadShooterRawSpeed();
	float actualSpeed = ReadShooterSpeed();
	float error = speed - actualSpeed;
	float relativeError = error/speed * 100;
	float correction = gain*error;
	if(correction < 0) correction = 0;
	count++;
//	if(count % 3 != 0) //optional divider to reduce frequency of error correction
//		m_shooterSpeed = savedValue;
//	else
		m_shooterSpeed = speed + correction;
	
	if (count % 50 == 0) {
		printf("Target=%10.4f, Speed=%10.4f\n", speed, actualSpeed);
		printf("Error=%10.4f, Percent Error=%10.2f%%, Correction=%10.4f, Gain=%10.4f\n", error, relativeError, correction, gain);
	}
	
	if(m_shooterSpeed < 0.) m_shooterSpeed = 0.;
	if(m_shooterSpeed > 1.) m_shooterSpeed = 1.;
	
//	savedValue = m_shooterSpeed;
}

float LRTConveyors::ReadShooterRawSpeed()
{
//	printf("%f\n", m_shooterEncoder->GetPeriod());
	if (m_shooterEncoder->GetStopped()) {
		return 0;
	}
//	return 0;
	return 1/m_shooterEncoder->GetPeriod();
}
float LRTConveyors::ReadShooterSpeed() {
	return ReadShooterRawSpeed()/kShooterMaxRPS;
}


void LRTConveyors::ApplyOutputs(){
//	printf("%d", m_encraw->Get());
	switch (m_pickupOn) {
		case 1:
			m_pickupRear->Set(-1);
			m_pickupFront->Set(Relay::kForward);
			break;
		case -1:
			m_pickupRear->Set(1);
			m_pickupFront->Set(Relay::kReverse);
			break;
		default:
			m_pickupRear->Set(0);
			m_pickupFront->Set(Relay::kOff);
			break;
	}

	switch (m_liftOn) {
		case 1:
			m_liftFront->Set(Relay::kForward);
			m_liftRear->Set(Relay::kReverse);
			break;
		case -1:
			m_liftFront->Set(Relay::kReverse);
			m_liftRear->Set(Relay::kForward);
			break;
		default:
			m_liftFront->Set(Relay::kOff);
			m_liftRear->Set(Relay::kOff);
			break;
	}

	m_shooter->Set(m_shooterSpeed);
}



void LRTConveyors::Init() {
	m_pickupOn = false;
	m_liftOn = false;
	m_shooterSpeed = 0;
	
	m_shooterAtSpeed = false;
	m_shooterIsClear = true; // NOTE: User must load balls in NOT TOUCHING top roller.
}

void LRTConveyors::PreloadBalls() {
	m_liftOn = 0;
	m_shooterSpeed = 0;
	m_shooterAtSpeed = false;
	// wait for shooter to stop
	if (ReadShooterSpeed() > 0.2) {
		// shooter still moving, do nothing.
	} else {
		// now start piling balls up
		m_liftOn = 1;
		m_shooterIsClear = false;
	}
}

// TODO FIXME: Must press twice to start lift shooting?
// TODO FIXME At Atlanta: Reduce TimeoutForWaitingForShooterToGetToSpeed
void LRTConveyors::Shoot(float speed) {
	static float lastShooterTargetSpeed = 0;
	if (lastShooterTargetSpeed != speed)
		m_shooterAtSpeed = false;
	
	if (!m_shooterIsClear) {
		++m_shootTime;
		if (m_shootTime < kTimeToClearLift) {
			m_liftOn = -1; // reverse lift
			m_shooterSpeed = kShooterReverseSpeed;
		} else {
			m_liftOn = 0; // lift off
			m_shootTime = 0;
			m_shooterIsClear = true;
			m_shooterAtSpeed = false;
		}
		lastShooterTargetSpeed = 0;
	} else {
		SetShooter(speed);
		lastShooterTargetSpeed = speed;
		
		if (!m_shooterAtSpeed) {
			if (ReadShooterSpeed() < 0.8*speed && m_shootTime < kTimeoutForWaitingForShooterToGetToSpeed) {
				++m_shootTime;
			} else {
				m_shootTime = 0;
				m_shooterAtSpeed = true;
			}
		} else {
			m_liftOn = 1; // fire away!
		}
	}
}
void LRTConveyors::NotShooting() {
	// FIXME: this is ugly
	m_shootTime = 0;
	m_liftOn = 0;
}
