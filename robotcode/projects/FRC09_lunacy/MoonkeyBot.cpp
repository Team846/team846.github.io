#include "LRTIterativeRobot.h"
#include "LRTgyro.h"	//this needs to come before WPILib.h to read LRTgyro.h instead of WPILib's version
#include "WPILib.h"
#include "util.h"
#include "LRTDriverControls.h"
#include "LRTConveyors.h"
#include "CurrentSensor.h"
#include "SlewLimitedDrive.h"
#include "LCD.h"
#include "LRTConnections.h"
#include "Logger.h"
#include "LRTEncoder.h"
#include "Compass.h"
#include "Brake.h"
#include "DBSDrive.h"
#include "ArcadeDrive.h"
#include "Victor.h"
#include "sysLib.h"
#include "DigitalInput.h"
#include "AirText.h"
#include "LRTDriverStationLCD.h"
#include "TorqueLimitedMotorOutput.h"
#include <cmath>
#include "util.h"
#include "Switchbox.h"
#include "lrtFileSave.h"


using namespace std;

const static int kMaxBraking = 8; // FIXME: reduce this!

/**
 * This is a demo program showing the use of the RobotBase class.
 * The SimpleRobot class is the base of a robot application that will automatically call your
 * Autonomous and OperatorControl methods at the right time as controlled by the switches on
 * the driver station or the field controls.
 */
class MoonkeyBot : public LRTIterativeRobot {
	static const int ANALOG_1_SLOT = 1;
	static const int ANALOG_2_SLOT = 2;
	static const int DIO_1_SLOT = 4;

	const static int kLeft = 0;
	const static int kRight = 1;
	
	const static float kGyroSensitivity = 0.0125*182.4/180.;	// SparkFun 150 deg/S (DW,DG calibration 4-5-2009)
	
	UINT32 autoTaskState;
	
//	enum AutonomousTask {
//		kGoStraightAndSpinLeft, kGoStraightAndSpinRight,
//		kGoCornerLeft, kGoCornerRight,
//		kGoAllLeft, kGoAllRight,
//		kLoopTheLoopLeft, kLoopTheLoopRight, //go fwd, loop the loop, go fwd
//		kCornerToCornerLeft, kCornerToCornerRight //turn 45, go fwd (until you get to the next corner, turn 45
//	};
	AutonomousTask toDo;
	
	const static float kDefaultGyroPGain = 0.2;
	const static float kDefaultMaxBrake = 0.123;
	const static float kDefaultMaxAccel = 0.099;
	
	
	float m_gyroPGain;
	float m_maxBrake;
	float m_maxAccel;
	
	float m_numLoopsToReachFullPower;
	int m_autonState;
	int m_numAutonLoops;

	LRTDriverControls m_controls;
	DriverStation *m_ds; // driver station object
	LRTDriverStationLCD *m_dsLCD; //LCD for the driver station
	bool m_scrollUpWasPressed;
	bool m_scrollDownWasPressed;

	//	RobotDrive *m_robotDrive; // robot drive system
	DriveMethod *m_driveMethod;
	DBSDrive *m_dbsDrive;
	ArcadeDrive *m_arcadeDrive;

	//	SpeedController *m_driveSlewLimited_ESC[2];
//	CurrentSensor *m_currentLeft, *m_currentRight;
//	double currentSumLeft, currentSumRight;
	long m_currentElapsedTime;
	long m_currentStartTime;

	SpeedController *m_driveLeftESC, *m_driveRightESC;
	TorqueLimitedMotorOutput *m_driveLeft, *m_driveRight;

	LRTConveyors m_conveyors;

	Logger *m_logger;

	LRTEncoder *m_encoders[2];

	Compass *m_compass;

	//DigitalInput m_shooterHallEffect;

	DigitalInput *m_autonomousLeft;
	DigitalInput *m_autonomousSpinOrCorner;
	DigitalInput *m_autonomousRight;
	Switchbox *m_switchbox;
	
	Accelerometer *m_AccelX[2];
	Accelerometer *m_AccelY[2];

	Gyro *m_gyro;
	
	AnalogChannel *m_anaTest;
	

	Brake m_brakeLeft, m_brakeRight;

	struct {
		bool enabled;
		bool buttonWasPressed;
		int value;
	} m_serviceState;

	struct {
		int pickupState; // 0 off, 1 fwd, -1 rev
	} m_teleState;
	
	
	/*
	 * Allow extra autonomous cycles to run in teleop
	 * Safeties: Autonomous must run for at least x seconds
	 * Stops when:
	 *   Cycles completed
	 *   Joystick moved
	 * When stopped, additionalCyclesInTeleOp <- 0
	 * Once completed, Autonomous cannot run until Autonomous init is called again.
	 */
	class AutonExtra {
	public:
		bool runExtended;	//set if we want to run extended autonomouscycles in teleop
		int additionalCyclesInTeleOp; //Autnonmous Cycles to run during teleop
		int cyclesCompleted; //safety; must run cycles in autonomous
		bool ranInTeleOp; //if already ran in teleop, when disabled, we need to reset autonomous.
		static const unsigned k_autonomousExtraCycles = 5*200;
		static const int k_safetyCycles = 12*200;  // 12 seconds
		
		AutonExtra() { stop(); }
		
		void stop() {
			printf("Stop AutonExtended\n");
			runExtended = false;
		}
		void start() {
			printf("Start AutonExtended\n");
			runExtended = true;
			additionalCyclesInTeleOp = k_autonomousExtraCycles;
		}
		void reset() {
			printf("Reset AutonExtended\n");
			runExtended = false;
			additionalCyclesInTeleOp = 0;
			cyclesCompleted = 0;
			ranInTeleOp = 0;
		}
		void safetyCheck() {
			if (++cyclesCompleted == k_safetyCycles)
				start();
		}
		void decrementRemainingCycles() {
			if (additionalCyclesInTeleOp > 0) {
				printf("%d\n", additionalCyclesInTeleOp);
				if (--additionalCyclesInTeleOp==0)
				{
					stop();
					printf("Last Extended Autonomous Cycle");
				}
			}
		}
	} m_autonExtra;

	
	class motionDetector {
	protected:
		bool initialized;
		static const float kThreshold = 0.05;	//allowable motion of joystick
	public:
		motionDetector(){ reset(); }
		void reset() { initialized=false; }
		bool moved(float v) {
			static float firstValue;
			if (!initialized) {
				initialized=true;
				firstValue = v;
				return false;
			}
			return fabs(v-firstValue) > kThreshold;
		}
	} m_motionDetect_driverY;

	static const float k_period = 5.0e-3;	//200Hz;
//	static const float k_period = 5.178e-3; //if using Victors to sync at 193.12 Hz (5 periods : : 3 Victor PWM output cycles)
// (Victor period empirically measured to be 8.63 ms)

	
	
	//*****************LCD (including Scroll) definitions*********************************
	static const int LCDSLOT = DIO_1_SLOT;
	static const int SCROLLSLOT = DIO_1_SLOT;
	//static const int SCROLLUP = 13;
	//static const int SCROLLDN = 14;
	I2C_LCD *m_lcd; // LCD

	//*****************AirText declarations ***********************************************
//	AirText *m_air;

	// local variables to count the number of peridoic loops performed
	UINT32 m_periodicLoops;
	UINT32 m_priorPacketNumber; // keeep track of the most recent packet number from the DS
	UINT8 m_dsPacketsReceivedInPreviousSecond; // This is updated once/second in CommonNewData() for easier sync with printing & LCD
	
	bool recordPacketCount;
//	UINT32 m_PacketCount;	//counts the total number of packets received

	void print2LCD(int line, const char* format, ...) {
		va_list args;

		char buffer[40];

		va_start(args, format);
		vsprintf(buffer, format, args);
		m_lcd->print(line, buffer);
		m_dsLCD->print(line + 1, buffer);
		if (line == 0) //print twice on the driver station lcd
			m_dsLCD->print(0, buffer);
		va_end(args);
	}

public:
	MoonkeyBot() :
		m_brakeLeft(LRTConnections::kDIOBrakeLeft),
				m_brakeRight(LRTConnections::kDIOBrakeRight) {
		printf("MoonkeyBot Starting\n");
		
		// Initialize various gains/settings
		m_gyroPGain = kDefaultGyroPGain;
		m_maxBrake = kDefaultMaxBrake;
		m_maxAccel = kDefaultMaxAccel;
		
		
		m_periodicLoops = 0;
		SetPeriod(k_period);
		m_serviceState.enabled = false;

		printf("Creating logger\n");
		m_logger = Logger::GetInstance();
		printf("Logger Created\n");

		m_ds = DriverStation::GetInstance();
		
		toDo = kGoStraightAndSpinLeft; //default
		
		m_autonomousLeft = new DigitalInput(LRTConnections::kDIOAutonomousLeft);
		m_autonomousSpinOrCorner = new DigitalInput(LRTConnections::kDIOAutonomousSpinOrCorner);
		m_autonomousRight = new DigitalInput(LRTConnections::kDIOAutonomousRight);
		m_switchbox = new Switchbox(m_autonomousLeft, m_autonomousRight, m_autonomousSpinOrCorner);
		
	
		printf("toDo = %d\n",toDo);
				
		m_dsLCD = LRTDriverStationLCD::GetInstance();

		m_lcd
				= new I2C_LCD(LCDSLOT,LRTConnections::kDIOScrollUp,LRTConnections::kDIOScrollDn);

//		m_air = new AirText(DIO_1_SLOT);
		//
		//		m_air->SetDelay_mS(7);
		//		m_air->print(0, "GO 846");
		//		m_air->print(1, "Moonkey");
		//		m_air->print(2, "SCORE");
		//		m_air->print(3, "WE LOVE MARK");
		//		m_air->print(4, "FUNKY");
		//		m_air->print(5, "LUNACY");
		//		m_air->print(6, "FIRST");

		m_compass = new Compass(DIO_1_SLOT);

		//
		//		m_driveSlewLimited_ESC[kLeft] = new Jaguar(1);
		//		m_driveSlewLimited_ESC[kRight] = new Jaguar(2);
//		printf("Init Left Current Sensor\n");
//		m_currentLeft = new CurrentSensor(
//				LRTConnections::kAnalogCurrentSenseLeft,
//				LRTConnections::kDIOCurrentSenseResetLeft
//		);
//		printf("Init Right Current Sensor\n");
//		m_currentRight = new CurrentSensor(
//				LRTConnections::kAnalogCurrentSenseRight,
//				LRTConnections::kDIOCurrentSenseResetRight
//		);
//		printf("Fin Current Sensor Init\n");
		//		drive[kLeft] = new SlewLimitedDrive(m_driveSlewLimited_ESC[kLeft], m_driveSlewLimited_Current[kLeft]);
		//		drive[kRight] = new SlewLimitedDrive(m_driveSlewLimited_ESC[kRight], m_driveSlewLimited_Current[kRight]);

		m_encoders[kLeft] = new LRTEncoder(
				DIO_1_SLOT, LRTConnections::kDIOEncoderLeftA,
				DIO_1_SLOT, LRTConnections::kDIOEncoderLeftB,
				false, Encoder::k1X
		);
		m_encoders[kLeft]->SetDistancePerPulse(LRTConnections::kDistancePerPulse);
		m_encoders[kLeft]->SetMinRate(2.);

		m_encoders[kLeft]->Start();
		m_encoders[kRight] = new LRTEncoder(
				DIO_1_SLOT, LRTConnections::kDIOEncoderRightA,
				DIO_1_SLOT, LRTConnections::kDIOEncoderRightB,
				true, Encoder::k1X
		);
		m_encoders[kRight]->SetDistancePerPulse(LRTConnections::kDistancePerPulse);
		m_encoders[kRight]->SetMinRate(2.);
		m_encoders[kRight]->Start();

#if defined(LRT_2008_ROBOT)
		m_driveLeftESC = new Victor(LRTConnections::kPWMDriveLeft);
		m_driveRightESC = new Victor(LRTConnections::kPWMDriveRight);
#elif defined(LRT_2009_ROBOT)
		m_driveLeftESC = new Jaguar(LRTConnections::kPWMDriveLeft);
		m_driveRightESC = new Jaguar(LRTConnections::kPWMDriveRight);
#endif
		
		m_driveLeft = new TorqueLimitedMotorOutput(m_driveLeftESC, m_encoders[kLeft], &m_brakeLeft);
		m_driveRight = new TorqueLimitedMotorOutput(m_driveRightESC, m_encoders[kRight], &m_brakeRight);
		m_driveLeft->SetMaxAccel(m_maxAccel);
		m_driveRight->SetMaxAccel(m_maxAccel);
		m_driveLeft->SetMaxBrake(m_maxBrake);
		m_driveRight->SetMaxBrake(m_maxBrake);
		//USER: m_driveLeft->Set(...)
		//USER: m_driveLeft->UpdateOutput();  // (100Hz or so)
			// m_driveLeftESC->Set(); => Jaguars
		
//		m_arcadeDrive = new ArcadeDrive(*m_driveLeftESC, *m_driveRightESC, false);
//						= new DBSDrive(*m_driveLeft, *m_driveRight, m_brakeLeft, m_brakeRight, kMaxBraking);
		
		m_dbsDrive = new DBSDrive(*m_driveLeft, *m_driveRight, m_brakeLeft, m_brakeRight, kMaxBraking);
		m_arcadeDrive = new ArcadeDrive(*m_driveLeft, *m_driveRight, false);
//		m_driveMethod = m_dbsDrive;
		m_driveMethod = m_arcadeDrive;

		m_AccelX[kLeft] = new Accelerometer(ANALOG_1_SLOT,LRTConnections::kLEFT_ACCEL_X);
		m_AccelY[kLeft] = new Accelerometer(ANALOG_1_SLOT,LRTConnections::kLEFT_ACCEL_Y);
		m_AccelX[kRight] = new Accelerometer(ANALOG_1_SLOT,LRTConnections::kRIGHT_ACCEL_X);
		m_AccelY[kRight] = new Accelerometer(ANALOG_1_SLOT,LRTConnections::kRIGHT_ACCEL_Y);

		m_AccelX[kLeft]->SetZero(2.455);
		m_AccelY[kLeft]->SetZero(2.455);
		m_AccelX[kRight]->SetZero(2.455);
		m_AccelY[kRight]->SetZero(2.455);

		m_AccelX[kLeft]->SetSensitivity(0.7425); // Sensitvity = 0.7425 Volts/G
		m_AccelY[kLeft]->SetSensitivity(0.7425);

		m_AccelX[kRight]->SetSensitivity(0.7425); // Sensitvity = 0.7425 Volts/G
		m_AccelY[kRight]->SetSensitivity(0.7425);

		m_gyro = new Gyro(ANALOG_1_SLOT,LRTConnections::kGyro);

		m_gyro->SetSensitivity(kGyroSensitivity);
		
		
		m_anaTest = new AnalogChannel(ANALOG_1_SLOT,LRTConnections::kAnalogTest);


		//		m_robotDrive = new RobotDrive(LRTConnections::kPWMDriveLeft, LRTConnections::kPWMDriveRight);
		//		m_robotDrive->SetInvertedMotor(RobotDrive::kRearRightMotor, true);
		//		robot_drive = new RobotDrive(m_drive[0], m_drive[1]);


		//Update the motors at least every 100ms.
		GetWatchdog().SetExpiration(100);
	}
	
	//*********************************************************************************************************
	~MoonkeyBot() {
		//		delete m_robotDrive;
		delete m_dbsDrive;
		delete m_arcadeDrive;
		delete m_driveLeftESC;
		delete m_driveRightESC;
		delete m_driveLeft;
		delete m_driveRight;
		delete m_encoders[kLeft];
		delete m_encoders[kRight];
		delete m_lcd;
//		delete m_air;
		delete m_compass;
		delete m_AccelX[kLeft];
		delete m_AccelY[kLeft];
		delete m_AccelX[kRight];
		delete m_AccelY[kRight];
		delete m_gyro;
		
		delete m_anaTest;

		
		delete m_autonomousLeft;
		delete m_autonomousSpinOrCorner;
		delete m_autonomousRight;
		delete m_switchbox;
//		delete m_currentLeft;
//		delete m_currentRight;

		//		for (int i=0; i<2; ++i) {
		//			delete m_drive[i];
		//			delete m_driveSlewLimited_Current[i];
		//			delete m_driveSlewLimited_ESC[i];
		//		}
	}

	//	void CommonContinuous() {
	//		GetWatchdog().SetEnabled(false);
	//		printf("Stealing control\n");
	//		printf("System ticks per second: %d\n", sysClkRateGet());
	//		while(1) { // steal control
	//			m_currentLeft->ResetPulse();
	//			taskDelay(1);
	//		}
	//	}
	//*********************************************************************************************************

	// 200hz, all modes
		/*********************************************************************************
		 * NOTE:  anything placed here will be called on each iteration of the periodic   *
		 * loop.  Assuming the default 200 Hz loop speed, code should only really be      *
		 * placed here for I/O that can respond at a 200 Hz rate.  (e.g. the Jaguar speed *
		 * controllers                                                                    *
		 * *******************************************************************************/
	void CommonPeriodic() {
		GetWatchdog().Feed();
		++m_periodicLoops;
		
		m_logger->startNextCycle();
		//		Logger::log1hz("1hz logging test\n");
		//		Logger::log1hz("SlewAdjust: %f => %f\n", m_controls.GetSlewAdjust(), m_numLoopsToReachFullPower);


		m_lcd->ScrollUpdateLCD();
				
		if(m_controls.GetGyroResetTestButton())
			m_gyro->Reset();
		

		float turn = (m_driveRight->Get() - m_driveLeft->Get())/2;
		const float kTurnThreshold = 0.10;
		float maxAccelMultiplier = 1.0;
		if (fabs(turn) > kTurnThreshold) {
			maxAccelMultiplier = 1.5;
		}
		maxAccelMultiplier = 1.0; //TEMP: this is temporary
		m_driveLeft->UpdateOutput(maxAccelMultiplier);
		m_driveRight->UpdateOutput(maxAccelMultiplier);
//		if (m_periodicLoops % 2 == 0) {
//			float left = m_driveLeft->Get(), right = m_driveRight->Get();
//			printf("Out: l=%f, r=%f\n", left, right);
//		}
	
		if ((m_periodicLoops % 2) == 0) {
			// put 100 Hz Victor control here
			
			// must go after drive..->UpdateOutput() because TorqueLimitedMotorOutput sets the brakes.
			m_brakeLeft.UpdateOutput();
			m_brakeRight.UpdateOutput();
		}
		if ((m_periodicLoops %4) == 0) {
			// put 50 Hz Servo control here (i.e. Hitec HS-322HD)

		}

		DoCurrentSensors();
		DoLCD();
		if (EverySecond()){
			PrintStats();
			recordPacketCount = true;
		}
	}
	//*********************************************************************************************************
	bool EverySecond(void) { return EverySecond(0); }
	bool EverySecond(int shift)
	{
		return 0 == ((m_periodicLoops+shift) % (UINT32)GetLoopsPerSec()); //This is unncessary computation
	}
	bool EveryHalfSecond(void) { return EveryHalfSecond(0); }
	bool EveryHalfSecond(int shift)
	{
		//GetLoopsPerSec() may be odd, so we can't simply module divide on 1/2 without causing drift.
		const UINT32 n = (UINT32) GetLoopsPerSec(); 
		const UINT32 remainder = (m_periodicLoops + shift) % n;
		return (remainder == 0 || remainder == n/2);
	}
	//*********************************************************************************************************
	void PrintStats(void) {
		if (0) {
			printf("Uptime:  %d secs", (UINT32) (m_periodicLoops
					/ GetLoopsPerSec()) );
			printf("  DS Packets:  %u", m_dsPacketsReceivedInPreviousSecond);
		}
		// 1 Hz
//			printf("GetFPGATime = %d\n", GetFPGATime());

		//			printf("  Enc: %d / %d\n", m_encoders[kLeft]->Get(),m_encoders[kRight]->Get());
		//			printf("  Bearing:  %.1f degrees\n", m_compass->GetBearing());
		//			printf("  Right X:  %.2f Y: %.2f\n",
		//					m_AccelX[kRight]->GetAcceleration(),
		//					m_AccelY[kRight]->GetAcceleration());
		//			printf("  Leftt X:  %.2f Y: %.2f\n",
		//					m_AccelX[kLeft]->GetAcceleration(),
		//					m_AccelY[kLeft]->GetAcceleration());
		//			printf("  Gyro angle:  %.1f degrees\n", m_gyro->GetAngle());

		//			printf("x = %d, y = %d\n", m_x, m_y);
		//			printf("dx = %d %02x, dy = %d %02x\n", dx, dx, dy, dy);
		//			printf("sysClkRateGet %d\n", sysClkRateGet());
		//
		//			printf("Accel delta = %lf\n", m_Accel->GetX_Accel());
		//
		//			printf("Accelerometer x:  %f  y:  %f\n",m_rightX->GetAcceleration(), m_rightY->GetAcceleration());


		//			float current_velocity_ips = (m_encoders[kLeft]->GetRate()
		//					+m_encoders[kRight]->GetRate())/2.;

					//			current_velocity_ips = 70.;

		//			if (current_velocity_ips>=50.) {
		//				m_air->SetDelay_mS((UINT8)(600./current_velocity_ips+0.5));
		//				m_air->StartAir(nStrCnt++);
		//				if (nStrCnt>5)
		//					nStrCnt = 0;
		//			} else {
		//				m_air->StopAir();
		//
		//			}
	}
	//*********************************************************************************************************
	void DoLCD(void) {
		const int k_LCDShift = 25; //move lcd update away from cycle 0 where everything else is printing
		const UINT32 LCD_HowOften = (UINT32)GetLoopsPerSec()/2;
		const UINT32 LCD_HowMany = 1+LCD_HowOften/88;
		
		if (EverySecond()) {			
			print2LCD(1, "Uptime:  %d secs", (UINT32) (m_periodicLoops / GetLoopsPerSec()));
			print2LCD(2, "DS Packets:  %u", m_dsPacketsReceivedInPreviousSecond);
		}
	//	if ((m_periodicLoops+k_LCDShift) % LCD_HowOften == 0) {
			if (EveryHalfSecond(k_LCDShift)) {
			print2LCD(3, "Battery: %4.2f V", m_ds->GetBatteryVoltage());
			print2LCD(4, "Bearing:  %.0f deg", m_compass->GetBearing());
			print2LCD(5, "R.Ac: %.2f %.2f",
					m_AccelX[kRight]->GetAcceleration(),
					m_AccelY[kRight]->GetAcceleration());
			print2LCD(6, "L.Ac: %.2f %.2f", m_AccelX[kLeft]->GetAcceleration(),
					m_AccelY[kLeft]->GetAcceleration());
			print2LCD(7, "G. Angle: %.1f deg", m_gyro->GetAngle());
			print2LCD(8, "L.Enc: %.0f in", m_encoders[kLeft]->GetDistance());
			print2LCD(9, " V: %+4.0f ips %+4.0fpc",
					m_encoders[kLeft]->GetRate(),
					m_encoders[kLeft]->GetNormalizedRate()*100
			);
			print2LCD(10, "R.Enc: %.0f in", m_encoders[kRight]->GetDistance());
			print2LCD(11, " V: %+4.0f ips %+4.0fpc",
					m_encoders[kRight]->GetRate(),
					m_encoders[kRight]->GetNormalizedRate()*100
			);
		}

		if ((m_periodicLoops+k_LCDShift+1) % LCD_HowOften == 0)
			m_lcd->LCDBeginUpdate(); //Moves LCD data to a 2nd buffer, ready to be transmitted

		m_lcd->LCDUpdate(LCD_HowMany); //Sends a few buffered bytes each cycle

		if (m_periodicLoops % ((UINT32)GetLoopsPerSec() / 2) == 0) {
			//2 Hz
			//			printf("Digital In DS: ");
			//			for(int i =1; i<=8;i++)
			//				printf("%d %d; ",i, (int) m_ds->GetDigitalIn(i));
			//			printf("\n");


			m_dsLCD->LCDUpdate();
		}
	}
		//*********************************************************************************************************
	void DoCurrentSensors(void) {
		// current sensor code
		//			m_currentLeft->Measure();
		//			m_currentRight->Measure();
		//			currentSumLeft += m_currentLeft->GetCurrent();
		//			currentSumRight += m_currentRight->GetCurrent();

		if ((m_periodicLoops %5) == 0) {
			m_currentElapsedTime = GetFPGATime() - m_currentStartTime;
			//				currentSumLeft /= m_currentElapsedTime;
			//				currentSumLeft *= 25000; //interval between measurements (uS)
			//				currentSumRight /= m_currentElapsedTime;
			//				currentSumRight *= 25000;
			//				if (0 && (m_periodicLoops %200) == 0) {
			//					printf("CurrentL: %f\nCurrentR: %f\n", currentSumLeft,
			//							currentSumRight);
			//				}
			if (m_controls.GetLoggingButton()) {
				// log at 40Hz
				Logger::GetInstance()->file() << GetFPGATime() << "\t";
				//					Logger::GetInstance()->file() << currentSumLeft<<"\t";
				//					Logger::GetInstance()->file() << currentSumRight<<"\t";
				Logger::GetInstance()->file() << m_controls.GetY()<<endl;
			}
			m_currentStartTime = GetFPGATime();
			//				currentSumLeft = currentSumRight = 0;
		}

	}
	//*********************************************************************************************************
	void ServiceModeInit() {
		m_serviceState.value = 0;
	}
	//*********************************************************************************************************
	void ServiceMode() {
		static int count = 0;
		count++;
		
		if (count % 50 == 0)
			printf("Service Mode\n");
		if (false)	
			AutonomousTest();
		else {
			//		static int count = 0;
			if (m_controls.GetExitServiceMode()) {
				printf("Exiting Service Mode!\n");
				m_serviceState.enabled = false;
				return;
			}

			DebouncedJoystick *stick = m_controls.GetDriverStick();
			bool multiplier = stick->IsButtonDown(10);
			int step = (multiplier ? 3 : 1);
			bool changed = false;
			if (stick->IsButtonJustPressed(7)) { // up
				m_serviceState.value += step;
				changed=true;
			} else if (stick->IsButtonJustPressed(9)) { // down
				m_serviceState.value -= step;
				changed=true;
			} else if (stick->IsButtonJustPressed(11)) { // max
				m_serviceState.value = 128;
				changed=true;
			} else if (stick->IsButtonJustPressed(8)) { // min
				m_serviceState.value = -128;
				changed=true;
			} else if (stick->IsButtonJustPressed(12)) { // zero
				m_serviceState.value = 0;
				changed=true;
			}
			if (changed) {
				printf("SERVICE MODE: output %d\n", m_serviceState.value);
			}
			m_driveLeft->Set(m_serviceState.value/128.0f);
			m_driveRight->Set(m_serviceState.value/128.0f);
			if(count % 50 == 0){
				printf("Right Encoder speed=%f\n",m_encoders[kRight]->GetRate());
				printf("Left Encoder speed=%f\n",m_encoders[kLeft]->GetRate());
			}
//			m_conveyors.SetShooter(m_controls.GetShooterSpeed(), 4.0
//					* (m_controls.GetOperatorThrottle() + 1.)/2.);
//
//			m_conveyors.ApplyOutputs();
		}
	}
	//*********************************************************************************************************
	void DisabledNewData() {
//		switch (m_controls.GetAutonoumousOptions()) {
//		case LRTDriverControls::kGoCornerLeft:
//			toDo = this->kGoCornerLeft;
//			print2LCD(0, "Auton: 45Left");
//			break;
//		case LRTDriverControls::kGoCornerRight:
//			toDo = this->kGoCornerRight;
//			print2LCD(0, "Auton: 45Right");
//			break;
//		case LRTDriverControls::kGoAllLeft:
//			toDo = this->kGoCornerLeft;
//			print2LCD(0, "Auton: 90Left");
//			break;
//		case LRTDriverControls::kGoAllRight:
//			toDo = this->kGoCornerRight;
//			print2LCD(0, "Auton: 90Right");
//			break;
//		case LRTDriverControls::kGoStraight:
//			toDo = this->kGoStraight;
//			print2LCD(0, "Auton: FWD");
//			break;
//		case LRTDriverControls::kNoButton: // doesn't do anything
//			break;
//		}

//		m_numLoopsToReachFullPower = m_controls.GetSlewAdjust() * 800;
		
		

		if (!m_serviceState.enabled && m_controls.GetEnterServiceMode()) {
			printf("Entering Service Mode!\n");
			m_serviceState.enabled = true;
			ServiceModeInit();
		} else if (m_serviceState.enabled && m_controls.GetExitServiceMode()) {
			printf("Exiting Service Mode!\n");
			m_serviceState.enabled = false;
		}
	}
	//*********************************************************************************************************
	void CommonNewData() {
		static UINT32 packetsReceivedInCurrentSecond = 0;
		
		++packetsReceivedInCurrentSecond;
		if (recordPacketCount){
			m_dsPacketsReceivedInPreviousSecond = packetsReceivedInCurrentSecond;
			packetsReceivedInCurrentSecond = 0;
			recordPacketCount = false;
		}

		//Driver Station LCD Scrolling
		if (m_controls.GetHatScrollUp() && !m_scrollUpWasPressed) {
			m_dsLCD->ScrollUpLCD();
			m_scrollUpWasPressed = true;
		} else if (!m_controls.GetHatScrollUp() && m_scrollUpWasPressed) {
			m_scrollUpWasPressed = false;
		}
		if (m_controls.GetHatScrollDown() && !m_scrollDownWasPressed) {
			m_dsLCD->ScrollDownLCD();
			m_scrollDownWasPressed = true;
		} else if (!m_controls.GetHatScrollDown() && m_scrollDownWasPressed) {
			m_scrollDownWasPressed = false;
		}
		
		
		
		
				
	}
	
	//*********************************************************************************************************
	void DisabledInit(void) {
		print2LCD(0, "DISABLED");
		m_numAutonLoops = 0;
		
		//extended autonomous cycles should be stopped if we ever ran teleop and then were disabled
		if (m_autonExtra.ranInTeleOp)
			m_autonExtra.stop();

	}

	//*********************************************************************************************************
	void TeleopInit(void) {
		print2LCD(0, "TELE-OPERATED");
		m_conveyors.Init();
		m_teleState.pickupState = 1;
		
		
		
		m_autonExtra.ranInTeleOp = true;
		
		UINT32 startTime;
//		
		FileSave f;
		f.PrintVariables();
		startTime = GetFPGATime();
		f.Read();
		printf("Elapsed read time: %d", GetFPGATime() - startTime);
		f.ClearVariables();
		startTime = GetFPGATime();
		f.Write();
		printf("Elapsed write time: %d", GetFPGATime() - startTime);
		f.PrintVariables();
	}
	
	//*********************************************************************************************************
	// 50hz, teleop
	void TeleopNewData() {
		static int count = 0;
		++count;
		if (m_controls.GetTractionOverride()) {
			m_driveLeft->SetMaxAccel(1);
			m_driveRight->SetMaxAccel(1);
			m_driveLeft->SetTorqueLimitEnabled(false);
			m_driveRight->SetTorqueLimitEnabled(false);
			if (count % 50 == 0)
				printf("TESTING: override traction\n");
		} else {
//			float maxbrake = 0.123;
//			float maxaccel = 0.099;
			
			//TODO Figure out maxbrake and maxaccel, uncomment below for testing
//			maxbrake = (m_controls.GetDriverThrottle()+1)/2.;
//			maxbrake = maxbrake*0.5 + 0.0;
//			maxaccel = (m_controls.GetOperatorThrottle()+1)/2.;
//			maxaccel = maxaccel*0.3 + 0.0;
			if(float accel = m_controls.GetMaxAccel()>=0.){
				m_maxAccel = accel;
				m_driveLeft->SetMaxAccel(m_maxAccel);
				m_driveRight->SetMaxAccel(m_maxAccel);
			}
			
			if(float brake = m_controls.GetMaxBrake()>=0.){
				m_maxBrake = brake;
				m_driveLeft->SetMaxBrake(m_maxBrake);
				m_driveRight->SetMaxBrake(m_maxBrake);		
			}
//			
//			m_driveLeft->SetMaxAccel(maxaccel);
//			m_driveRight->SetMaxAccel(maxaccel);
//			m_driveLeft->SetMaxBrake(maxbrake);
//			m_driveRight->SetMaxBrake(maxbrake);
			m_driveLeft->SetTorqueLimitEnabled(true);
			m_driveRight->SetTorqueLimitEnabled(true);
			if (count % 50 == 0)
//				printf("TESTING: Set max accel=%f brake=%f\n", maxaccel, maxbrake);
				printf("TESTING: Set max accel=%f brake=%f\n", m_maxAccel, m_maxBrake);
		}
		// ToDo: Remember to hardwire the gyroPGain in MoonkeyBot!!
		//It won't get set in autonomous from here [DG]!!
//		m_gyroPGain = (m_controls.GetDriverThrottle()+1.)/2;	// gyroPGain on (0.,1.)
//		m_gyroPGain *= 0.5;
//		m_gyroPGain = 0.2;
		
		if(float gain = m_controls.GetGyroPGain()>=0. ){
			m_gyroPGain = gain;
		}
			
		if(count%50==0)
		{
			printf("m_gyroPGain = %.3f\n",m_gyroPGain);
		}

		
		m_controls.Update();

		if (m_serviceState.enabled) {
			ServiceMode();
			return;
		}

		//		printf("TeleopNewData Throttle %f\n", (m_controls.GetOperatorThrottle()+1)/2);
		// Handle controls
		//		printf(" Y: %f, X: %f\n", m_controls.GetY(), m_controls.GetX());
		//		printf(" BtnLiftOff: %d\n", m_controls.GetLiftOffButton());

		//		int brakeAmt = m_controls.GetY()*8;
		////		printf("%d\n", brakeAmt);
		//		m_brakeLeft.Set(brakeAmt);
		//		return;

//		if (m_controls.GetDriveModeButton()) {
//			m_driveMethod = m_arcadeDrive;
//			Logger::log1hz("Arcade drive\n");
//			m_brakeLeft.Set(0);
//			m_brakeRight.Set(0);
//		} else {
//			m_driveMethod = m_dbsDrive;
//			//			Logger::log1hz("DBS drive\n");
//		}

		m_driveMethod->ArcadeDrive(m_controls.GetY(), m_controls.GetTwist());
//		if (count % 10 == 0) {
//			float left = m_driveLeftESC->Get(), right = m_driveRightESC->Get();
//			printf("Final MotorOut: L=%+6.3f ; R=%+6.3f\n", left, right);
//		}

		//m_robotDrive->ArcadeDrive(m_controls.GetY(), m_controls.GetX());
		//		printf(" Shooter: %f, PickupOn %d, LiftOn %d\n", m_controls.GetShooterAxis(), m_controls.GetPickupOnButton(), m_controls.GetLiftOnButton());
		//		printf(" PickupOff %d, LiftOff %d\n", m_controls.GetShooterAxis(), m_controls.GetPickupOffButton(), m_controls.GetLiftOffButton());

		if (m_controls.GetShootHighSpeedButton()) {
			m_conveyors.Shoot(LRTConveyors::kShooterHighSpeed);
		} else if (m_controls.GetShootLowSpeedButton()) {
			m_conveyors.Shoot(LRTConveyors::kShooterLowSpeed);
		} else if (m_controls.GetShootWaterfallButton()) {
			m_conveyors.Shoot(LRTConveyors::kShooterWaterfallSpeed);
		} else {
			m_conveyors.NotShooting(); // FIXME: ugly hack
			if (m_controls.GetPreloadButton()) {
				m_conveyors.PreloadBalls();
			}
		}

		if (m_controls.GetPickupOnButton())
			m_teleState.pickupState = 1;
		else if (m_controls.GetPickupOffButton())
			m_teleState.pickupState = 0;
		if (m_controls.GetPickupRevButton()) { //  momentary UNJAM button
		//			m_conveyors.SetLiftOff(); // needed?? may interfere with other stuff
			m_conveyors.SetPickup(-1);
		} else {
			m_conveyors.SetPickup(m_teleState.pickupState);
		}

		m_conveyors.ApplyOutputs();

	}

	
	//*********************************************************************************************************
	void AutonomousGoFwdAndSpin(int dirToSpin) {
		static float out;
		++m_numAutonLoops;

		switch (m_autonState) {
		case 0:
			out = (float) m_numAutonLoops/100.;
			if (out>1.)
				out = 1.;
			m_driveMethod->ArcadeDrive(out, 0.0);
			if (getDistance()>8*12.) { //Went 9 ft with coasting
				//					m_driveMethod->ArcadeDrive(0.,0.);
				m_autonState++;
			}
			break;
		case 1:
			//				m_driveMethod->ArcadeDrive(0.,0.);
			if (dirToSpin == kLeft)
				m_driveMethod->ArcadeDrive(0., 1.);
			else
				m_driveMethod->ArcadeDrive(0., -1.);
			break;
		}
	}
	//*********************************************************************************************************
	void AutonomousTurnThenSpin(int dirToTurn) {
		static float speed = 0.;
		static bool turnInPlace = false;
		bool pivot = false;
		float angle = 60.;
		if(dirToTurn == kLeft) angle = -60.;
		float error;
		float bearing;
		
		static const int timeToTurn = Ticks(1.0);
		static const int timeFwd = timeToTurn + Ticks(2.0);
		
		++m_numAutonLoops;

		switch (m_autonState) {
		case 0:
			speed = 1.;
			bearing = angle;
			pivot = true;
			if(m_numAutonLoops>timeToTurn)
				m_autonState++;
			break;
		case 1:
			pivot = false;
			speed = 1.;
			bearing = m_gyro->GetAngle();
			if(m_numAutonLoops>timeFwd)
				m_autonState++;
			break;
		case 2:
			turnInPlace = true;
			if (dirToTurn == kRight)
				m_driveMethod->ArcadeDrive(0., 1.);
			else
				m_driveMethod->ArcadeDrive(0., -1.);
			break;
		}
		error = bearing-m_gyro->GetAngle();
		
//			if(m_numAutonLoops%4 == 0)
//				printf("m_gyroPGain = %.3f, error = %.3f\n",m_gyroPGain,error);
						
		float turn = m_gyroPGain* -error;
		float maxTurn = .7;
		turn = LRTUtil::clamp(turn, -maxTurn,maxTurn);
		if (pivot)
			speed = fabs(turn);
		if(!turnInPlace)
		m_driveMethod->ArcadeDrive(speed,turn);	// Need to negate turn to get same sense as ArcadeDrive
	}
	
	//*********************************************************************************************************
	void AutonomousTurn(int dir, int timeToTurn) {
		float out = 0.;
		++m_numAutonLoops;
		switch (m_autonState) {
		case 0:
			if (m_numAutonLoops < timeToTurn) {
				out = m_numAutonLoops/100 +0.20;
				if (out>1.)
					out=1.;
				if(dir == kLeft)
					m_driveMethod->ArcadeDrive(out, out);
				else
					m_driveMethod->ArcadeDrive(out, -out);
			} else {
				m_autonState++;
				m_numAutonLoops = 0;
			}
			break;
		case 1:
			out = (float) m_numAutonLoops/100.;
			if (out>1.)
				out = 1.;
			m_driveMethod->ArcadeDrive(out, 0.0);
			if (getDistance()>12*12.) { //Went 9 ft with coasting
				//					m_driveMethod->ArcadeDrive(0.,0.);
				m_autonState++;
				m_numAutonLoops = 0;
			}
			break;
		case 2:
//				out = m_numAutonLoops/100 +0.30;
//				if (out>1.)
//					out=1.;
			if(dir == kLeft)
				m_driveMethod->ArcadeDrive(0, -1);
			else
				m_driveMethod->ArcadeDrive(0, 1);
			break;
		}
	}
	
	//*********************************************************************************************************
	void AutonomousLoopTheLoop(int dir) {
		float bearing;
		float error;
		float speed;
		float angle = 360.0;
		if(dir==kLeft) angle = -360.0;
		bool pivot = false;
		
		static const int timeForward = Ticks(0.5);	// 0.5 sec *ticks per sec
//		static const int timeToSpin = timeForward+2400;	// 12 seconds
		static int timeToDrive; // = timeToSpin+800;	// 4 seconds
		
		++m_numAutonLoops;
		switch (m_autonState) {
		
		case 0:
			bearing = 0.;
			speed = 1.;
			if (m_numAutonLoops > timeForward) {
				m_autonState++;					
			}
			break;
			
		case 1:
			
			bearing = angle;
			pivot = true;
			error = bearing-m_gyro->GetAngle();
			if (fabs(error) < 5) { // within 5 degrees
				m_autonState++;
				timeToDrive = m_numAutonLoops + 800; // 4 seconds
			}
			break;

		case 2:
			
			bearing = angle;
			speed = 1.;
			
		
			
			if (m_numAutonLoops > timeToDrive) {
				m_autonState++;					
			}
			break;
			
		case 3:
			speed = 0.;
			bearing = m_gyro->GetAngle();
			
			break;
		}
		error = bearing-m_gyro->GetAngle();
		
//			if(m_numAutonLoops%4 == 0)
//				printf("m_gyroPGain = %.3f, error = %.3f\n",m_gyroPGain,error);
						
		float turn = m_gyroPGain* -error;
		float maxTurn = .7;
		turn = LRTUtil::clamp(turn, -maxTurn,maxTurn);
		if (pivot)
			speed = fabs(turn);
		m_driveMethod->ArcadeDrive(speed,turn);	// Need to negate turn to get same sense as ArcadeDrive
	}
	
	void AutonomousDriveForwardThenGyroAngle(float angle) {
			float bearing;
			float error;
			float speed;
			
			static const int timeForward = 100;	// 0.5 sec *ticks per sec
//			static const int timeToDrive = timeForward+800;	// 4 seconds
			static const int timeToDrive = timeForward+3200;	// some # of seconds
			
			++m_numAutonLoops;
			switch (m_autonState) {
			
			case 0:
				bearing = 0.;
				speed = 1.;
				if (m_numAutonLoops > timeForward) {
					m_autonState++;					
				}
				break;
				
			case 1:
				
				bearing = angle;
				speed = 1.;
				
			
				
				if (m_numAutonLoops > timeToDrive) {
					m_autonState++;					
				}
				break;
			case 2:
				speed = 0.;
				bearing = m_gyro->GetAngle();
				
				break;
			}
			error = bearing-m_gyro->GetAngle();
			
//			if(m_numAutonLoops%4 == 0)
//				printf("m_gyroPGain = %.3f, error = %.3f\n",m_gyroPGain,error);
							
			m_driveMethod->ArcadeDrive(speed,m_gyroPGain* -error);	// Need to negate turn to get same sense as ArcadeDrive
		}
	
	/*
	 * AutonomousCornerToCorner turns 45 degrees, drives to the neighboring corner, and turns into that corner [dl]
	 */
	void AutonomousCornerToCorner(int dir) { 
			static const int timeForward = Ticks(0.5);	// TEMP is 0.5 sec fwd enough?
			static const int timeToDrive = timeForward+Ticks(2.5);	// 2.5 seconds
			static const int finalTurnDuration = Ticks(2.0);
			static const float distToDrive = 2*12.0; // TEMP need to go probably ~8ft on actual field
			
			float angle = 45;
			if (dir == kLeft)
				angle = -angle;
			
			static float bearing;
			static float speed;
			float error;
			static int timer;
			static float targetDistance;
			static bool pivot;
			
			++m_numAutonLoops;
			switch (m_autonState) {
			case 0: //initialize (print auton mode and set pivot false, not true until final turn)
				pivot = false;
				printf("Corner to corner:");
				printf(dir==kLeft ? "left\n" : "right\n");
				m_autonState++;
			case 1: //go fwd a certain amount
				bearing = 0.; 
				speed = 1.;
				if (m_numAutonLoops > timeForward) {
					m_autonState++;					
				}
				break;
			case 2: //turn 45 degrees (left or right depending on dir), taking timeToDrive amount of time to do so
				bearing = angle;
				speed = 1.;
				if (m_numAutonLoops > timeToDrive) {
					m_autonState++;					
				}
				break;
			case 3: //go straight a certain amount (until targetDistance is reached, where targetDistance = distance already traveled+remaining dist)
				targetDistance = getDistance() + distToDrive;
				printf("targetDistance %f\n currentDistance %f\n", targetDistance, getDistance());
				m_autonState++;
				break;
			case 4: //only for testing, print to make sure it has traversed correct distance
				printf("currentDistance %f\n", getDistance());
				if (getDistance() > targetDistance)
					m_autonState++;
				break;
			case 5: //initializing variables for final turn
				printf("finalDistance %f\n", getDistance());
				timer = finalTurnDuration;
				bearing *= 2;
				pivot = true;
				m_autonState++;
				break;
			case 6:
				if (--timer < 0) //completing final turn
					m_autonState++;
				break;
			case 7:
				speed = 0.;
				bearing = m_gyro->GetAngle();
				
				break;
			}
			error = bearing-m_gyro->GetAngle();
			
//			if(m_numAutonLoops%4 == 0)
//				printf("m_gyroPGain = %.3f, error = %.3f\n",m_gyroPGain,error);
			
			float turn = m_gyroPGain* -error;
			if (pivot)
				speed = fabs(turn);
			m_driveMethod->ArcadeDrive(speed,turn);	// Need to negate turn to get same sense as ArcadeDrive
		}

	
	void AutonomousNewData(void) {
		//look for stick motion and abort extended Autonomous cycles.
		if (!IsAutonomous() && m_motionDetect_driverY.moved(m_controls.GetY()))
		{
			m_autonExtra.stop();
			printf("Y stick moved; Auto Extra aborted\n");
		}
	}

	//*********************************************************************************************************	
	void AutonomousInit() {
		m_autonExtra.reset();
		m_motionDetect_driverY.reset();

		
		m_encoders[kLeft]->Reset();
		m_encoders[kRight]->Reset();
		m_autonState = 0;
		m_numAutonLoops = 0;
		
		m_conveyors.SetAllOff();
		m_conveyors.ApplyOutputs();
		
		// reset gyro angle to a bearing of 0 degrees at start of autonomous
		m_gyro->Reset();
		
		
//		bool leftSwitch = !m_autonomousLeft->Get();
//		bool rightSwitch = !m_autonomousRight->Get();
//		bool ninetyDegreeSwitch = !m_autonomousSpinOrCorner->Get();
//		// note: switches are LOW when ON,  HIGH when OFF
//		printf(" L %d / NINETY %d / R %d \n", leftSwitch, ninetyDegreeSwitch, rightSwitch);
//		if (leftSwitch && !rightSwitch)
//		{
//			if (!m_autonomousSpinOrCorner->Get()){ 
//				toDo = kGoAllLeft;
//				printf("Go All Left\n");
//			}
//			else{
//				toDo = kGoCornerLeft;
//				printf("Go Corner Left\n");
//			}
//		}
//		else if (!leftSwitch && rightSwitch) {
//			if (!m_autonomousSpinOrCorner->Get()){
//				toDo = kGoAllRight;
//				printf("Go All Right\n");
//			}
//			else{
//				toDo = kGoCornerRight;
//				printf("Go Corner Right\n");
//			}
//		}
//		else if (leftSwitch && rightSwitch) {
//			toDo = kGoStraightAndSpinRight;
//		} else { // !left && !righ
//			toDo = kGoStraightAndSpinLeft;
//		}
		
		toDo = m_switchbox->GetTask();
		
		
		printf("Auton started\n");
		switch (toDo) {
		case kGoStraightAndSpinLeft:
			print2LCD(0, "AUTON FWD SPIN LEFT");
			break;
		case kGoStraightAndSpinRight:
			print2LCD(0, "AUTON FWD SPIN RIGHT");
			break;
		case kGoCornerLeft:
			print2LCD(0, "AUTON TURN LEFT");
			break;
		case kGoCornerRight:
			print2LCD(0, "AUTON TURN RIGHT");
			break;
		case kCornerToCornerLeft:
			print2LCD(0, "AUTON CORNER LEFT");
			break;
		case kCornerToCornerRight:
			print2LCD(0, "AUTON CORNER RIGHT");
			break;
		default:
			print2LCD(0, "AUTON PROBLEM");
			break;
		}
		//print2LCD(0,"%d %d %d",m_ds->GetDigitalIn(3), m_ds->GetDigitalIn(4), m_ds->GetDigitalIn(5));
	}
	
	//*********************************************************************************************************

	float getDistance() {
		return (  m_encoders[kLeft]->GetDistance()
				+ m_encoders[kRight]->GetDistance()) / 2;
	}
	
	//*********************************************************************************************************
	void AutonomousTest() {
			float out = 0.;
					++m_numAutonLoops;
					int time = 50; //determined experimentally, should turn 45
					if (m_periodicLoops % 50 == 0)
						print2LCD(0, "T=%d", time);
					
					
					switch (m_autonState) {
					case 0:
						if (m_numAutonLoops < time) {
							out = m_numAutonLoops/100 +0.30;
							if (out>1.)
								out=1.;
							m_driveMethod->ArcadeDrive(out, -out);
						} else {
							m_autonState++;
							m_numAutonLoops = 0;
						}
						break;
					case 1:
						out = (float) m_numAutonLoops/100.;
						if (out>1.)
							out = 1.;
						m_driveMethod->ArcadeDrive(out, 0.0);
						if (getDistance()>8*12.) { //Went 9 ft with coasting
							//					m_driveMethod->ArcadeDrive(0.,0.);
							m_autonState++;
							m_numAutonLoops = 0;
						}
						break;
					case 2:
						if (m_numAutonLoops < time) {
							out = m_numAutonLoops/100 +0.30;
							if (out>1.)
								out=1.;
							m_driveMethod->ArcadeDrive(0, -out);
						} else {
							m_autonState++;
							m_numAutonLoops = 0;
						}
						break;
					}
		}

	
	//*********************************************************************************************************
	// This may be called from outside Autonomous Mode, for extended run time!
	void AutonomousPeriodic() {

		// First, allow Autonomous running under Teleop
		if (!IsAutonomous())	//Running extended cycles in TeleOp
			m_autonExtra.decrementRemainingCycles();

		m_autonExtra.safetyCheck();	//Authorizes extended autonomous cycles in teleop when enough cycles are completed.
	
		
		
		
//		if (m_autonomousLoops % 50 == 0)
//			printf("DS State: %d\n", autoTaskState);
		

		
//toDo=kGoStraightAndSpinLeft;
//toDo=kGoCornerLeft;
		switch (toDo) {
			case kGoStraightAndSpinLeft:
				AutonomousGoFwdAndSpin(kLeft);
				break;
			case kGoStraightAndSpinRight:
				AutonomousGoFwdAndSpin(kRight);
				break;
//			case kLoopTheLoopLeft:
//				AutonomousLoopTheLoop(kLeft);
//				break;
//			case kLoopTheLoopRight:
//				AutonomousLoopTheLoop(kRight);
//				break;
			case kGoCornerLeft:
				AutonomousTurnThenSpin(kLeft);
	//			AutonomousTurn(kLeft);
	//			AutonomousTurn(kLeft, 125);
				break;
			case kGoCornerRight:
				AutonomousTurnThenSpin(kRight);
//				AutonomousTurn(kRight, 125);
				break;
//			case kGoAllLeft:
//				AutonomousTurn(kLeft, 200);
//				break;
//			case kGoAllRight:
//				AutonomousTurn(kRight, 200);
//				break;
			case kCornerToCornerLeft:
				AutonomousCornerToCorner(kLeft);
				break;
			case kCornerToCornerRight:
				AutonomousCornerToCorner(kRight);
				break;
			default:
				break;
		}
//			AutonomousGoFwd();
	}
	//*********************************************************************************************************

	
	/*
	 * IsAutonomousExtended() Return true to continue to run as autonomous.
	 * We know we are running in Teleop in this call,
	 * because IsAutonomous() returned false
	 */ 
	bool IsAutonomousExtended()
	{
//		if (m_autonExtra.runExtended)
//			printf("AutoExtended %d\n", m_autonExtra.runExtended);
		return m_autonExtra.runExtended;
	}
};


//*********************************************************************************************************
START_ROBOT_CLASS(MoonkeyBot)
;
