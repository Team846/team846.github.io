#include "Configurable.h"
#include "Config.h"

Configurable::Configurable()
{
    Config::RegisterConfigurable(this);
}

Configurable::~Configurable()
{

}
