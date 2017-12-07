#include "LRTAuton.h"
#include "LRTKicker.h"

LRTAuton::LRTAuton(SpeedController &left, SpeedController &right,
		LRTKicker &kicker) :
	m_escLeft(left), m_escRight(right), m_autonTask("Autonomous task",
		(FUNCPTR) CallAutonTask), m_kicker(kicker), m_prefix("LRTAuton."),
		m_config(LRTConfig::GetInstance()) {
	ApplyConfig();
}

void LRTAuton::StartAuton() {
	m_autonTask.Start( (UINT32) this );
}

void LRTAuton::StopAuton() {
	m_autonTask.Stop();
}

void LRTAuton::ApplyConfig() {
	m_movementTimeBetweenKicks = m_config.Get<float>(m_prefix
			+ "movementTimeBetweenKicks", 1.5);
	m_movementTimeInCloseZone = m_config.Get<float>(m_prefix
			+ "movementTimeInCloseZone", 4.0);
	m_waitTimeForKick = m_config.Get<float>(m_prefix
			+ "waitTimeForKick", 1.0);
	m_isInCloseZone = m_config.Get<bool>(m_prefix + "isInCloseZone", 1);
	m_numBallsInZone = m_config.Get<int>(m_prefix + "numBallsInZone", 0); 
}

void LRTAuton::SetIsInCloseZone( bool in ) {
	m_isInCloseZone = in;
	m_config.Set<bool>(m_prefix + "isInCloseZone", in);
}

bool LRTAuton::GetIsInCloseZone() {
	return m_isInCloseZone;
}

void LRTAuton::AutonTask() {
	if (m_isInCloseZone) {		
		AsynchronousPrinter::Printf( "LRTAuton::AutonTask moving forward to score ball\n" );
		
		m_escLeft.Set( 1.0 );
		m_escRight.Set( 1.0 );

		Wait(m_movementTimeInCloseZone);
		AsynchronousPrinter::Printf( "LRTAuton::AutonTask stopping movement\n" );
		
		m_escLeft.Set( 0.0 );
		m_escRight.Set( 0.0 );
	}
	else {
		for( int i = 0; i < m_numBallsInZone; i++ ) {
			AsynchronousPrinter::Printf( "LRTAuton::AutonTask moving forward to kick ball\n" );			
			m_escLeft.Set( 1.0 );
			m_escRight.Set( 1.0 );

			Wait( m_movementTimeBetweenKicks );
			
			AsynchronousPrinter::Printf( "LRTAuton::AutonTask kicking and stopping movement\n" );			
			m_escLeft.Set( 0.0 );
			m_escRight.Set( 0.0 );
			
			m_kicker.Release();
			Wait( m_waitTimeForKick );
		}
	}
}
