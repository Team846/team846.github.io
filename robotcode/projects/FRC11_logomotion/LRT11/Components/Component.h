#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "..\General.h"
#include "..\ActionData.h"
#include <string>
#include <list>
using namespace std;

class Component;

typedef struct ComponentData
{
    bool RequiresEnabledState;
    const static int NO_DS_DISABLE_DIO = -1;
    int DS_DIOToDisableComponent;
};
typedef pair < Component*, ComponentData> ComponentWithData;

/*
 * \brief the abstract class upon which all components are based. Contains a reference to the ActionData  and defines an output method that is called once every time the main loop iterates through. 
 */
class Component
{
protected:
	/*!
	 * a reference to the ActionData class which contains the commands for all the components.
	 */
    ActionData& action;
public:
    Component()
        :  action(ActionData::GetInstance()) { }

    /*!
     * \brief called every time the main loop iterates.
     */
    virtual void Output() = 0;
    
    /*!
     * \brief returns the name of the component which is helpful for debugging and profiling.
     */
    virtual string GetName() = 0;
    
    /*!
     * \brief the Factory method that constructs all the components. This makes it so that the main loop does not have to know about the individual components.
     * \return a list structs with components and information about the components. The information about the component includes whether the output method should be called if the robot is disabled as well as which digital io on the driverstation should disable this component. 
     */
    static list < ComponentWithData >* CreateComponents();

private:
    static ComponentData createComponentData(bool RequiresEnabledState, int DIO);

};


#endif
