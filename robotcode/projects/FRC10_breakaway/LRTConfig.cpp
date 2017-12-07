//Author: Brian Axelrod and David Liu (2010)

#include "LRTConfig.h"
#include <string>
#include <fstream>
#include <sstream>
#include "LRTUtil.h"
#include "LRTConsole.h"
#include "Profiler.h"
#include "AsynchronousPrinter.h"

using namespace std;

LRTConfig* LRTConfig::m_instance = NULL;

LRTConfig& LRTConfig::GetInstance ()
{
	if (m_instance == NULL)
	{
		m_instance = new LRTConfig();	
	}
	return *m_instance;
}

LRTConfig::LRTConfig() {
	for (int i = 0; i < kNumAnalogAssignable; ++i) {
		string keyname = "assignable." + LRTUtil::toString<int>(i);
		m_analogAssignments[i] = "none";
		m_analogAssignmentScaleMin[i] = 0;
		m_analogAssignmentScaleMax[i] = 1;
	}
	
	Load();
	
	AddToSingletonList();
}

bool LRTConfig::Load(string path)
{
	ProfiledSection pf("Config.Load");
	m_configdata = tload(path);
	
	for (int i = 0; i < kNumAnalogAssignable; ++i) {
		string keyname = "assignable." + LRTUtil::toString<int>(i);
		m_analogAssignments[i] = Get<string>(keyname + ".name");
		m_analogAssignmentScaleMin[i] = Get<float>(keyname + ".scaleMin");
		m_analogAssignmentScaleMax[i] = Get<float>(keyname + ".scaleMax");
	}
	return true;
}

map<string, string> LRTConfig::tload(string path)
{
	map<string, string> ret;
	
	ifstream fin(path.c_str());
	if (!fin.is_open())//if the file does not exist then return false
		return ret;

	string key, val;
	while (getline(fin, key, '=')&&
			getline(fin, val, '\n'))
	{
		{
			ProfiledSection pf("tload-set");
			ret[key] = val;
		}
		AsynchronousPrinter::Printf("Cfg:%s=%s\n", key.c_str(), val.c_str());
	}
	fin.close();
	return ret;
}

void LRTConfig::Log(string key, string oldval, string newval)
{
	ofstream log ("/log.txt", ios::app);
	log << key << " changed from " << oldval << " to " << newval <<'\n';
	log.close();
}

bool LRTConfig::Save(string path)
{
	ofstream fout(path.c_str());
	if (!fout.is_open())//could not create file in that path
		return false;
//	map <string, string> temp = tload(path);
	for( map<string, string>::const_iterator it = m_configdata.begin(); it != m_configdata.end(); it++ )
	{
//		if (temp[it->first] != it->second) {
//			printf ("*************LOGGING*** %s: %s to %s\n", it->first, temp[it->first], it->second);
////			Log (it->first, temp[it->first], it->second);
//		}
		fout << it->first << "=" << it->second<< "\n";
	}
	
	fout.close();
	return true;	
}

template <typename T>
T LRTConfig::Get(string key)
{
	stringstream strstream(m_configdata[key]);
	T ret;
	strstream >> ret;
	
	// [dcl]: Cause default values to be set, in the case of blank parameters.
	Set(key, ret);
	
	return ret;
}
template float LRTConfig::Get<float>(string key);
template bool LRTConfig::Get<bool>(string key);
template double LRTConfig::Get<double>(string key);
template string LRTConfig::Get<string>(string key);
template int LRTConfig::Get<int>(string key);

template <typename T>
T LRTConfig::Get(string key, T defaultValue)
{
	if (m_configdata.find(key) == m_configdata.end()) {
		Set(key, defaultValue);
		return defaultValue;
	}
	
	return Get<T>(key);
}
template float LRTConfig::Get<float>(string key, float defaultValue);
template bool LRTConfig::Get<bool>(string key, bool defaultValue);
template double LRTConfig::Get<double>(string key, double defaultValue);
template string LRTConfig::Get<string>(string key, string defaultValue);
template int LRTConfig::Get<int>(string key, int defaultValue);

template <typename T>
void LRTConfig::Set(string key, T val)
{
	m_configdata[key] = LRTUtil::toString<T>(val);
}
template void LRTConfig::Set<float>(string key, float val);
template void LRTConfig::Set<bool>(string key, bool val);
template void LRTConfig::Set<double>(string key, double val);
template void LRTConfig::Set<string>(string key, string val);
template void LRTConfig::Set<int>(string key, int val);

template <typename T>
bool LRTConfig::SetIfNotSet(std::string key, T val, T notSetVal)
{
	// Get will set if not set
	T newVal = Get( key, val );
	
	// if get returns the value which represents "not set", the new value 
	// should still be set
	if( newVal == notSetVal ) {
		Set( key, val );
		return true;
	}
	
	return newVal == val;
}
template bool LRTConfig::SetIfNotSet<float>(string key, float val, float notSetVal);
template bool LRTConfig::SetIfNotSet<bool>(string key, bool val, bool notSetVal);
template bool LRTConfig::SetIfNotSet<double>(string key, double val, double notSetVal);
template bool LRTConfig::SetIfNotSet<string>(string key, string val, string notSetVal);
template bool LRTConfig::SetIfNotSet<int>(string key, int va, int notSetVall);

float LRTConfig::ScaleAssignableAnalogValue(float value, int analogIndex) {
	return LRTUtil::rescale<float>(value, 0, 5,
			m_analogAssignmentScaleMin[analogIndex], m_analogAssignmentScaleMax[analogIndex]);
}

void LRTConfig::UpdateAssignableControls(float analog[kNumAnalogAssignable])
{
	for (int i = 0; i < kNumAnalogAssignable; ++i) {
		if (m_analogAssignments[i].length() && m_analogAssignments[i] != "none") {
			ProfilerHelper pf;
			pf.Start("UpdateAssignable.Scale");
			float newValue = ScaleAssignableAnalogValue(analog[i], i);
			pf.Finish();
			pf.Start("UpdateAssignable.Print");
			LRTConsole::GetInstance().PrintMultipleTimesPerSecond(0.25, "Assgn:%s= %f\n", m_analogAssignments[i].c_str(), newValue);
			pf.Finish();
			pf.Start("UpdateAssignable.Set");
			Set(m_analogAssignments[i], LRTUtil::toString<float>( newValue ));
			pf.Finish();
		}
	}
}
