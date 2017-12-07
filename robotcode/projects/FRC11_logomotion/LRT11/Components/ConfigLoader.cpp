#include "ConfigLoader.h"
#include "..\Config\Config.h"
#include "..\Util\AsynchronousPrinter.h"
#include "..\ActionData\ConfigAction.h"

ConfigLoader::ConfigLoader()
    : Component()
    , config(Config::GetInstance())
{

}

ConfigLoader::~ConfigLoader()
{

}

void ConfigLoader::Output()
{
    if(action.config->load)
    {
        AsynchronousPrinter::Printf("Loading Configuration\n");
        config.Load();
    }
    if(action.config->save)
    {
        AsynchronousPrinter::Printf("Saving Configuration\n");
        config.Save();
    }
    if(action.config->apply)
    {
        AsynchronousPrinter::Printf("Applying Configuration\n");
        config.ConfigureAll();
    }
}

string ConfigLoader::GetName()
{
    return "ConfigLoader";
}
