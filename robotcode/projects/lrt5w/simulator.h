#ifndef __SIMULATOR_H
#define __SIMULATOR_H

//called from lrtConnections.h
//redefines various inputs to work when running simulator
#undef mEncoderLeftBin
#undef mEncoderRightBin
#undef mRightLowGearSw
#undef mRightHighGearSw 
#undef mLeftLowGearSw
#undef mLeftHighGearSw
#undef mShoulderUpperLimitSw
#undef mShoulderLowerLimitSw
#undef mForeArmUpperLimitSw 
#undef mForeArmLowerLimitSw 
#undef mCompressorAtPressure
#undef mAutonomousSw0
#undef mAutonomousSw1
#undef mAutonomousSw2
#undef mBashPlateLimit

#define mEncoderLeftBin 0
#define mEncoderRightBin 0
#define mRightLowGearSw  1
#define mRightHighGearSw 0
#define mLeftLowGearSw	 1
#define mLeftHighGearSw  0

#define mShoulderUpperLimitSw 0
#define mShoulderLowerLimitSw 0
#define mForeArmUpperLimitSw  0
#define mForeArmLowerLimitSw  0

#define mCompressorAtPressure 0

#define mAutonomousSw0	0
#define mAutonomousSw1	0
#define mAutonomousSw2	0

#define mBashPlateLimit	0


#endif	//__SIMULATOR_H

