#ifndef CONFIGURABLE_H_
#define CONFIGURABLE_H_

#include "../General.h"

/*!
 * \brief The base class for all classes that read values from the configuration system.
 */
class Configurable
{
public:
	/*!
	 * \brief registers the configurable as a listener with the Config class.
	 */
    Configurable();
    virtual ~Configurable();

    /*!
     * \brief Called when the configuration file is updates/loaded. 
     */
    virtual void Configure() = 0;
};

#endif //CONFIGURABLE_H_
