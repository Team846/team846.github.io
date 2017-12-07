/**
 * @file	2CANPlugin\APILogger.h
 *
 * @brief	Helper class for logging API calls and returns to a log file. 
 */

#ifndef APILOGGER__H_
#define APILOGGER__H_

#include <fstream.h>

class APILogger
{
	public:

		/**
		 * @brief	Constructor. 
		 *
		 * @param	name	File name to create in cRIO's root FTP directory. 
		 */
		APILogger(const char * name)
		{
			m_of.open(name);
		}

		/**
		 * @brief	Writes data to the log file. 
		 *
		 * @param	line	line of text to write. 
		 */

		void Write(const char * line)
		{
			if(!m_of.is_open())
				return;
			m_of.write(line,strlen(line));
		}
		
	private:
		ofstream m_of;
};

#endif // APILOGGER__H_
