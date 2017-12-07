#if 0
/*
 * DriverStationButtons.cpp
 *
 *  Created on: Jun 8, 2011
 *      Author: David
 */

//#include "DriverStationButtons.h"
#include "../Config/joystickdg.h" //TODO: Rename this header -dg

driverstation_button::driverstation_button* driverstation_button::button_list_ = NULL;
/*
 * Validates assigned values for all buttons.
 * Do this after creation of the buttons in the main code
 * since buttons may be static variables.
 * Calculates the keymap_index into the raw driver station data.
 */
int
driverstation_button::Initalize_All_Buttons()
{
    bool all_buttons_valid = true;
    driverstation_button* b;
    for(b = driverstation_button::button_list_; b != NULL; b = b->next_button_)
    {
        int error_status = b->Validate_Button();
        if(0 != error_status)
        {
            all_buttons_valid = false;
            continue;
        }
        //complete initialization
        b->keymap_index_ = (b->USBPort_ - 1) << 4 | (b->button_no_ - 1);
    }
    return all_buttons_valid ? 0 : 1;   //return status=0 on no errors.
}

/*
 * Validate_Button()
 * Ensure each button assignment is within range.
 * Do this check at initialization.
 */
int
driverstation_button::Validate_Button()
{
    //range of buttons is 1-12 (see WPLib joystick.h/.c etc.) -dg
    if(USBPort_ > 0 && USBPort_ < 4 && button_no_ > 0 && button_no_ <= 12)
    {
        printf("Key assignment out of range. USBPort=%d, Button=%d. Name=%.20s ",
                USBPort_, button_no_, name_);
        return 1;
    }
    return 0;
}

driverstation_button key1(1, 2, "funny");


//Joystick buttons -- Thrustmaster T.16000m

namespace DRIVER_INPUT
{
    const int joy1 = 0x0000;
    const int joy2 = 0x0100;
    const int kButton1 = joy1 | 00;
}
typedef queue<int>  INTQUEUE;
typedef queue<driverstation_button>  EventQueue;

EventQueue theQueue;

class DriverInputs
{
public:
    DriverInputs();
    ~DriverInputs();

    bool ButtonDown(driverstation_button& button);
    bool ButtonPressed(driverstation_button& button);
    bool ButtonReleased(driverstation_button& button);

    void Update();
private:
    DriverStation* ds_;
    struct driverstationRawData
    {
        UINT64 down_;
        UINT64 changed_;
    } raw_buttons_;
    int DriverInputs::ButtonIndexMap(UINT16 button_id);

};


void DriverInputs::Update()
{
    //save the buttons so we can see which changed later.
    UINT64 old_buttons = raw_buttons_.down_;

    UINT64 temp = 0;

    temp  = static_cast<UINT16>(ds_->GetStickButtons(3));
    temp <<= 16;
    temp |= static_cast<UINT16>(ds_->GetStickButtons(2));
    temp <<= 16;
    temp |= static_cast<UINT16>(ds_->GetStickButtons(1));
    temp <<= 16;
    temp |= static_cast<UINT16>(ds_->GetStickButtons(0));

    raw_buttons_.down_ = temp;
    raw_buttons_.changed_ = raw_buttons_.down_ ^ old_buttons;
    //should add these to the event queue. -dg TODO

    //empty queue
    while(!theQueue.empty())
        theQueue.pop();
    theQueue.push(DriverStationButtons::example);
    theQueue.front();
    theQueue.back();
    theQueue.size();

    UINT64 raw_button_down_events = raw_buttons_.changed_ ^ raw_buttons_.down_;
    for(int i = 0;  0 != raw_button_down_events; (++i, raw_button_down_events >>= 1)) //a button has been pressed or released.
    {
        if(0x1 & raw_button_down_events)
            theQueue.push(DriverStationButtons::example); //TODO: need to add the actual event
    }
}
//x& x - 1
//
//0111000100
//0111000011

//Returns true if button is down
bool DriverInputs::ButtonDown(driverstation_button& button)
{
    int key_index = button.Keymap_Index();
    return 0x1 & (raw_buttons_.down_ >> key_index);
}

//Returns true for one cycle when a button is pressed.
bool DriverInputs::ButtonPressed(driverstation_button& button)
{
    int key_index = button.Keymap_Index();
    return 0x1 & ((raw_buttons_.changed_ & raw_buttons_.down_) >> key_index);
}

//Returns true for one cycle when button is released.
bool DriverInputs::ButtonReleased(driverstation_button& button)
{
    int key_index = button.Keymap_Index();
    return 0x1 & ((raw_buttons_.changed_ & ~raw_buttons_.down_) >> key_index);
}
#endif //0
