#include "DutyCycleSubscriber.h"
#include "..\CAN\VirtualCANBusController.h"

DutyCycleSubscriber::DutyCycleSubscriber()
{
}

DutyCycleSubscriber::~DutyCycleSubscriber()
{

}

void DutyCycleSubscriber::Subscribe(int channel)
{
    VirtualCANBusController::GetInstance().SetDutyCycleSubscriber(channel, this);
}
