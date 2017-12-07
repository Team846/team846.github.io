//Author: Brian Axelrod and David Liu (2010)

#ifndef LRT_CONFIG_H_
#define LRT_CONFIG_H_

#include "WPILib.h"
#include <map>
#include <string>

class LRTConfig : public SensorBase {
	
public:
	const static int kNumAnalogAssignable = 4;
	
	virtual ~LRTConfig() { }
	static LRTConfig& GetInstance ();
	
	bool Load(std::string path = "/LRTConfig.txt");
	bool Save(std::string path = "/LRTConfig.txt");
	
	/*std::string Get(std::string key);
	int GetInt(std::string key);
	float GetFloat(std::string key);
	double GetDouble(std::string key);
	char GetChar(std::string key);
	bool GetBool(std::string key);
	
	void Set(std::string key, std::string val);
	void SetInt(std::string key, int val);
	void SetFloat(std::string key, float val);
	void SetDouble(std::string key, double val);
	void SetChar(std::string key, char val);
	void SetBool(std::string key, bool val);*/

	float ScaleAssignableAnalogValue(float value, int analogIndex);
	void UpdateAssignableControls(float analog[kNumAnalogAssignable]);

	template <typename T>
	T Get(std::string key);
	template <typename T>
	T Get(std::string key, T defaultValue);
	template <typename T>
	void Set(std::string key, T val);
	template <typename T>
	// val is the value to associate with key if it doesn't 
	// already have a value. if it does have a value which is 
	// equal to notSetVal, it will be considered "not set" and 
	// replaced with val
	bool SetIfNotSet(std::string key, T val, T notSetVal);

protected:
	LRTConfig();
	
private:
	static LRTConfig *m_instance;

	DISALLOW_COPY_AND_ASSIGN(LRTConfig);
	
	std::map<std::string, std::string> m_configdata;
	void Log (std::string key, std::string oldval, std::string newval);
	std::map<std::string, std::string> tload (std::string path);
	
	std::string m_analogAssignments[kNumAnalogAssignable];
	float m_analogAssignmentScaleMin[kNumAnalogAssignable];
	float m_analogAssignmentScaleMax[kNumAnalogAssignable];
};


#endif
