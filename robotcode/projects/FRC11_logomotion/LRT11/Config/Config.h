#ifndef CONFIG_H_
#define CONFIG_H_

#include "../General.h"
#include "Configurable.h"
#include "../Util/Util.h"
#include "../Util/Console.h"
#include "../Util/Profiler.h"
#include <string>
#include <fstream>
#include <map>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <list>

typedef struct ConfigVal
{
    string val;
    list<string>::iterator positionInFile;
};

/*!
 * \brief A Singleton that stores and retrieves configuration values from a file.
 * */
class Config
{
public:
	/*!
	 * \brief number of analog dials available to the programmer.
	 * */
    const static int kNumAnalogAssignable = 4;

    virtual ~Config();
    /*!
     * \brief The accessor method for the global instance of this class. 
     * */
    static Config& GetInstance();

    /*!
     * \brief Updates all the values from the configuration file. This may overwrites changes you made. 
     * */
    void Load();
    
    /*!
     * \brief Saves all the values to the configuration file.
     * */
    void Save();

    /*!
     * \brief Updates the values of the values linked to assignable dials.
     * */
    void UpdateAssignableDials();

    /*!
     * \brief Retreives the value of the given config value. You <B>must</B> specify a default value.
     */
    template <typename T> T Get(string section, string key, T defaultValue);
    
    /*!
     * \brief Sets the value of the given config value. This does not save it to the file until you call Save().
     */
    template <typename T> void Set(string section, string key, T val);

    /*!
     * \brief register a listener that is notified when configure all is called.
     */
    static void RegisterConfigurable(Configurable* configurable);
    
    /*!
     * \brief Call the Configure() methods of all the registered configurables.
     */
    static void ConfigureAll();
    
    /*!
     * \brief updates the file if it has been changed since the last time it has been loaded.
     */
    void CheckForFileUpdates();

private:
    Config();
    static Config* instance;
    time_t configLastReadTime_;

    const static string CONFIG_FILE_PATH;
    const static string COMMENT_DELIMITERS;

    static vector<Configurable*> configurables;

    list<string> *configFile; // line by line list of the config file

    map<string, string> configData;
    typedef map<string, map<string, ConfigVal> > config;
    config* newConfigData;
    map<string, list<string>::iterator > *sectionsMap;

    bool ValueExists(string section, string key);

    map<string, string> tload(string path);
    void LoadFile(string path);
    void SaveToFile(string path);

    DriverStation& ds;
    float ScaleAssignableAnalogValue(float value, int analogIndex);
    string analogAssignmentKeys[kNumAnalogAssignable];
    string analogAssignmentSections[kNumAnalogAssignable];
    float analogAssignmentScaleMin[kNumAnalogAssignable];
    float analogAssignmentScaleMax[kNumAnalogAssignable];

    static bool hasRun;
    string buildNumKey, runNumKey, buildTimeKey;

    DISALLOW_COPY_AND_ASSIGN(Config);
};


#endif
