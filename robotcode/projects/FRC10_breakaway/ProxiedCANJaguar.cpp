#include "ProxiedCANJaguar.h"
#include "LRTUtil.h"

ProxiedCANJaguar::ProxiedCANJaguar(int channel) :
	m_controller(CANBusController::GetInstance())
	, m_channel(channel) {
	
}

void ProxiedCANJaguar::Set(float speed) {
	m_controller.Set(m_channel, LRTUtil::clamp<float>(speed, -1.0, 1.0));
}

float ProxiedCANJaguar::Get() {
	return m_controller.Get(m_channel);
}

float ProxiedCANJaguar::GetCurrent() {
	return 0; //TODO
}

void ProxiedCANJaguar::ConfigNeutralMode(CANJaguar::NeutralMode mode) {
	m_controller.ConfigNeutralMode(m_channel, mode);
}

