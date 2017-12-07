//Author: David Liu (2010)

#ifndef LRT_CANBUSCTRLR_H_
#define LRT_CANBUSCTRLR_H_

#include "WPILib.h"
#include <queue>
#include <string>
#include "CANJaguar/CANJaguar.h"
#include "SafeCANJaguar.h"

class CANBusController : public SensorBase {
	
public:
	virtual ~CANBusController();
	static CANBusController& GetInstance();
	
	void Set(int id, float val);
	float Get(int id);
	void ConfigNeutralMode(int id, CANJaguar::NeutralMode mode);
	void Enumerate();

private:
	CANBusController();
	DISALLOW_COPY_AND_ASSIGN(CANBusController);
	static void BusWriterTaskRunner();
	void BusWriterTask();

	int BusIdToIndex(int id);
	
	const static int kMinJaguarId = 2;
	const static int kMaxJaguarId = 6;
	const static int kNumJaguars = kMaxJaguarId-kMinJaguarId+1;
	
	volatile float m_setpoints[kNumJaguars];
	volatile bool m_setpointChanged[kNumJaguars];
	volatile CANJaguar::NeutralMode m_neutralModes[kNumJaguars];
	volatile bool m_neutralModeChanged[kNumJaguars];
	SafeCANJaguar* m_jaguars[kNumJaguars];
	
//	bool m_enabled;
	static CANBusController *m_instance;
	
	Task m_busWriterTask;
	SEM_ID m_semaphore;
	
};

#endif
