/**
 * @file	2CANPlugin\PluginInit.cpp
 *
 * @brief	Implements the 2CAN Plugin interface for the FRC Robot.
 */

#include "2CAN.h"
#include "CANInterfacePlugin.h"
#include "stdio.h"
#include "Task.h"
#include <taskLib.h>

/**
 * @brief	Setting this to 1 will update 2CAN on robot state, making it
 *								aware of disable/teleop/auton. 
 * Enabling this has the following prerequisites 
 * 1) Plugin must be added to StartupDlls AFTER FRC_UserProgram so 
 *									that it can interact with WPILIB 
 * 2) Project will need DriverStation.h include from latest WPILIB. 
 */
#define SEND_ROBOT_INFO_TO_2CAN		(0)

#if SEND_ROBOT_INFO_TO_2CAN == 1
	#include "DriverStation.h"
#endif

namespace n2CANBridgePlugin
{
	C2CAN *g_2can = NULL;
	void Monitor2CanTask();
	CTask monitorTask ("2CANMonitor",(FUNCPTR)Monitor2CanTask);
}
using namespace n2CANBridgePlugin;

extern "C"
{
	/**
	 * @fn	INT32 FRC_2CANPlugIn_StartupLibraryInit()
	 *
	 * @brief	This function should construct the needed object and register it
	 * 			with the JaguarCANDriver.
	 * 			
	 * 			If named with the convention <module name>_StartupLibraryInit
	 * 			then this function will be called on load when listed in
	 * 			StartupDlls in "/ni-rt.ini" on the cRIO.  This can also be called
	 * 			from the console or as the entry-point for the debugger. 
	 *
	 * @return	zero. 
	 */

	INT32 FRC_2CANPlugIn_StartupLibraryInit()
	{
		// Always print a version of some sort.
		printf("FRC_2CANPlugIn was compiled from SVN revision %s\n", SVN_REV);

		FRC_NetworkCommunication_JaguarCANDriver_registerInterface(
				g_2can = new C2CAN() );

		monitorTask.Start();
		return 0;
	}
   
	/**
	 * @fn	INT32 FRC_2CANPlugIn_StartupLibraryCleanup()
	 *
	 * @brief	This function should unregister the plugin and cleanup any
	 * 			allocated memory. This can be called from the target console to
	 * 			free UDP ports for following debug sessions. 
	 *
	 * @return	zero. 
	 */ 

	INT32 FRC_2CANPlugIn_StartupLibraryCleanup()
	{
		printf("FRC_2CANPlugIn_StartupLibraryCleanup() - SVN revision %s\r\n",SVN_REV);
		//unregister plugin so that we can safely delete it
		FRC_NetworkCommunication_JaguarCANDriver_registerInterface(NULL);
		if(g_2can)
		{
			// just in case task ended because debugger has terminated 
			// free network resources
			g_2can->Close();
			//delete it
			delete g_2can;
			g_2can = 0;
		}
		return 0;
	}
}

void n2CANBridgePlugin::Monitor2CanTask()
{
#if SEND_ROBOT_INFO_TO_2CAN == 1
	DriverStation *m_ds = 0;
	DriverStation* (*GetDriverStation)(void) = 0;
	GetDriverStation = DriverStation::GetInstance;
	if(GetDriverStation)
		m_ds = GetDriverStation();
#endif

	while(true)    
	{
		taskDelay(100);				// replace with signal later
		if(g_2can->HasBeenClosed())	//
		{
			// 2CAN task has ended so lets shut everything down
			FRC_2CANPlugIn_StartupLibraryCleanup();
			break;//leave
		}
#if SEND_ROBOT_INFO_TO_2CAN == 1
		if(m_ds)
		{
			if(m_ds->IsDisabled())
				g_2can->SetRobotState(0); // slow blink green
			if(m_ds->IsEnabled())
				g_2can->SetRobotState(1); // fast green blink
			if(m_ds->IsAutonomous())
				g_2can->SetRobotState(2); // fast green blink
		}
		else
		{
			g_2can->SetRobotState(0); // slow blink green
		}
#endif // SEND_ROBOT_INFO_TO_2CAN == 1
		
	}
}
