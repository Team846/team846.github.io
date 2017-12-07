#ifndef __motorSim_h
#define __motorSim_h


//#define SIM

#ifdef SIM

void SIMmotorDrive(void);
void SimulateArmMotors(void);
void GetSimulatedArmPositions(void);

#endif //SIM

#endif	//__motorSim_h
