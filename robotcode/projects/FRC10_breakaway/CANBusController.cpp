//Author: David Liu (2010)

#include "CANBusController.h"
#include <string>
#include <fstream>
#include <sstream>
#include "LRTUtil.h"
#include "LRTConsole.h"
#include "AsynchronousPrinter.h"
#include "SafeCANJaguar.h"

using namespace std;

CANBusController* CANBusController::m_instance = NULL;

CANBusController& CANBusController::GetInstance()
{
	if (m_instance == NULL) {
		m_instance = new CANBusController();	
	}
	return *m_instance;
}

CANBusController::CANBusController()
: m_busWriterTask("CANBusController", (FUNCPTR)CANBusController::BusWriterTaskRunner)
, m_semaphore(semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE))
{
	AddToSingletonList();
	
	for (int id = kMinJaguarId; id <= kMaxJaguarId; ++id) {
		int idx = BusIdToIndex(id);
		m_jaguars[idx] = new SafeCANJaguar(id);
		m_setpoints[idx] = 0.0;
		m_neutralModes[idx] = CANJaguar::kNeutralMode_Jumper;
		m_setpointChanged[idx] = m_neutralModeChanged[idx] = false;
	}

	m_busWriterTask.Start();
}
CANBusController::~CANBusController() {
	for (int id = kMinJaguarId; id <= kMaxJaguarId; ++id) {
		int idx = BusIdToIndex(id);
		delete m_jaguars[idx];
	}
	semDelete(m_semaphore);
	m_semaphore = NULL;
}

void CANBusController::Set(int id, float val) {
	Synchronized s(m_semaphore);
	int idx = BusIdToIndex(id);
	if (m_setpoints[idx] != val) {
		m_setpoints[idx] = val;
		m_setpointChanged[idx] = true;
	}
}
float CANBusController::Get(int id) {
	Synchronized s(m_semaphore);
	int idx = BusIdToIndex(id);
	return m_setpoints[idx];
}

void CANBusController::ConfigNeutralMode(int id, CANJaguar::NeutralMode mode) {
	Synchronized s(m_semaphore);
	int idx = BusIdToIndex(id);
	if (m_neutralModes[idx] != mode) {
		m_neutralModes[idx] = mode;
		m_neutralModeChanged[idx] = true;
	}
}

void CANBusController::Enumerate() {
	for (int id = kMinJaguarId; id <= kMaxJaguarId; ++id) {
		int idx = BusIdToIndex(id);
		m_jaguars[idx]->IsOnline();
	}
}

int CANBusController::BusIdToIndex(int id) {
	if (id > kMaxJaguarId || id < kMinJaguarId) {
		AsynchronousPrinter::Printf("!! CANBusController: %d out of range\n", id);
		return 0;
	}
	return id - kMinJaguarId;
}

void CANBusController::BusWriterTaskRunner() {
	CANBusController::GetInstance().BusWriterTask();
}
void CANBusController::BusWriterTask() {
	while(true) {
		for (int id = kMinJaguarId; id <= kMaxJaguarId; ++id) {
			int idx = BusIdToIndex(id);
			float setpoint;
			CANJaguar::NeutralMode mode;
			bool setpointChanged, neutralModeChanged;
			{ Synchronized s (m_semaphore);
				setpoint = m_setpoints[idx];
				setpointChanged = m_setpointChanged[idx];
				mode = m_neutralModes[idx];
				neutralModeChanged = m_neutralModeChanged[idx];
			}
			if (setpointChanged) {
				m_jaguars[idx]->Set(setpoint);
				{ Synchronized s (m_semaphore);
					m_setpointChanged[idx] = false;
				}
			}
			if (neutralModeChanged) {
				m_jaguars[idx]->ConfigNeutralMode(mode);
				{ Synchronized s (m_semaphore);
					m_neutralModeChanged[idx] = false;
				}
			}
		}
		Wait(0.001); // allow other tasks to run
	}
	//
}
