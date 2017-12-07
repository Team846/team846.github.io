//Author: Brandon Liu (2010)

#include "LRTKicker.h"
#include "LRTConnections.h"
#include "LRTConfig.h"
#include <string>
#include "LRTBallDetector.h"
#include "LRTDriverStationLCD.h"
#include "AsynchronousPrinter.h"

using namespace std;

LRTKicker::LRTKicker(LRTBallDetector& detector)
: m_task("KickerTask", (FUNCPTR)LRTKicker::KickerTaskRunner)
, m_semaphore(semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE))
, m_detector(detector)
, m_relay(LRTConnections::kRelayKicker)
, m_sense(LRTConnections::kDioKickerSense)
, m_state (kIdle)
{
	ApplyConfig();
	m_task.Start((UINT32)this);
}

void LRTKicker::ApplyConfig() {
	LRTConfig &config = LRTConfig::GetInstance();
	
	string prefix("LRTKicker.");
	
	m_delayPastSense = config.Get<double>(prefix+"delayPastSense", 0.0);
	m_kickerReleaseTime = config.Get<double>(prefix+"kickerReleaseTime", 0.05);
}

void LRTKicker::Stop() {
	AsynchronousPrinter::Printf("LRTKicker::Stop\n");
	{ Synchronized s(m_semaphore);
		m_state = kIdle;
	}
//	m_task.Suspend();
}


void LRTKicker::Release() {
	AsynchronousPrinter::Printf("LRTKicker::Release\n");
	//Interlock: release only if ball is in possession
//	if (m_detector.IsBallClose()) {
		{ Synchronized s(m_semaphore);
			m_state = kReleaseAndWind;
		}
//	}
}

void LRTKicker::UnwindPulse() {
	AsynchronousPrinter::Printf("LRTKicker::UnwindPulse\n");
	{ Synchronized s(m_semaphore);
		m_state = kUnwindPulse;
	}
}

void LRTKicker::Disable() {
	m_task.Suspend();
}
void LRTKicker::Enable() {
	m_task.Resume();
}

bool LRTKicker::GetSense() {
	return (m_sense.Get() == kKickerSenseDepressed_LogicLevel);
}

int LRTKicker::GetSenseRaw() {
	return m_sense.Get();
}

void LRTKicker::KickerTaskRunner(UINT32 kicker_ptr) {
	( (LRTKicker*)kicker_ptr )->KickerTask();
}
void LRTKicker::KickerTask() {
	while (true) {
		KickerState state;
		{ Synchronized s(m_semaphore);
			state = m_state;
		}
		switch(state) {
			case kReleaseAndWind:
				m_relay.Set(kKickerWindup);

				double kickerReleaseTime;
				{ Synchronized s(m_semaphore);
					kickerReleaseTime = m_kickerReleaseTime;
				}
				Wait(kickerReleaseTime);
				
				while (state == kReleaseAndWind) {
					if (GetSense()) {
						double delayPastSense;
						{ Synchronized s(m_semaphore);
							delayPastSense = m_delayPastSense;
						}
						Wait(delayPastSense);
						break;
					}
					Wait(kKickerPollIntervalWhenActive);
					
					{ Synchronized s(m_semaphore);
						state = m_state;
						// FIXME: will locking the semaphore cause delays?
					}
				}
				
				m_relay.Set(Relay::kOff);

				{ Synchronized s(m_semaphore);
					m_state = kIdle;
				}
				break;
				
			case kUnwindPulse:
				m_relay.Set(kKickerReverse);
				Wait(kKickerUnwindPulseLength);
				m_relay.Set(Relay::kOff);

				{ Synchronized s(m_semaphore);
					m_state = kIdle;
				}
				break;
				
			case kIdle:
				m_relay.Set(Relay::kOff);
				Wait(kKickerPollIntervalWhenIdle);
				break;
		}
	}
}
