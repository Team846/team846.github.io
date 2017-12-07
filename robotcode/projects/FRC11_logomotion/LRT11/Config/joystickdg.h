// joystick control mappings
// D.Giandomenico
//#include "Joystick.h"

#if 0
#include "DriverStation.h"
#include <queue>

#ifndef joystickdg_h_
#define joystickdg_h_

#include "../Util/Util.h"
#warning ProcessedInputs is compiling



class driverstation_button
{
protected:
    int const USBPort_;
    int const button_no_;
    int keymap_index_; //index of the key into the raw bits from the driverstation
    const char* const name_;
public:
    int Keymap_Index()
    {
        return keymap_index_;
    }
    driverstation_button(int USBPort, int button_no, char* name)
        : USBPort_(USBPort)
        , button_no_(button_no)
        , name_(name ? name : "")
    {
        //keymap and other data that may require
        //initialization is done in Validate_ButtonOrDie()
        //so we may debug when main code is running
        //and force user to Validate by not initializing here.

        //insert into list of buttons.
        next_button_ = button_list_;
        button_list_ = this;
    }
    ~driverstation_button() {}

    static int Initalize_All_Buttons(); //returns zero status if no error.
    //use as Initalize() || Die().

protected:
    int Validate_Button();
    driverstation_button* next_button_;
    static driverstation_button* button_list_;
};



namespace DriverStationButtons
{
    namespace joystick
    {
        enum joystick
        {
            kTrigger = 1, kThumb = 2, kThumbLeft = 3, kThumbRight = 4,
            kButton5, kButton6, kButton7,   //  5  6  7  Left side
            kButton8, kButton9, kButton10,  // 10  9  8  button layout

            kButton11, kButton12, kButton13, // 13 12 11  Right side
            kButton14, kButton15, kButton16, // 14 15 16  button layout
        };
        //joystick

//      0
//      |s
//  0 --+-- 16383     RZ 0-255 clockwise
//      |
//    16383
//
//      0
//      |   (Slider)
//     255
    } //joystick

    static driverstation_button example(0, 3, "example");
    static driverstation_button trigger(1, 1, "trigger");
    static driverstation_button     hat(1, 2, "hat");
    static driverstation_button    lift(1, 3, "lift");
}

//How can we have different button mappings depending on the mode?
//Would like a base mapping, and then special assignments.
//We don't want butons defined in a class because they can't be initialized at the time they are delcard.
//Can we use namespaces?
//Should we add a mode to the key assignment, e,g, disabled|auto|service|teleop?
//Could pass a static driverstation to create maps, or create maps based on above flags.



#endif //joystickdg_h_
#endif //0
