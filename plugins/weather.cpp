#include "Core.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"
#include <vector>
#include <string>
#include "modules/World.h"

using std::vector;
using std::string;
using namespace DFHack;

bool locked = false;
unsigned char locked_data[25];

command_result weather (color_ostream &out, vector <string> & parameters);

DFHACK_PLUGIN("weather");

DFhackCExport command_result plugin_init ( color_ostream &out, std::vector <PluginCommand> &commands)
{
    commands.push_back(PluginCommand(
        "weather", "Print the weather map or change weather.",
        weather, false,
        "  Prints the current weather map by default.\n"
        "Options:\n"
        "  snow   - make it snow everywhere.\n"
        "  rain   - make it rain.\n"
        "  clear  - clear the sky.\n"
    ));
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown ( color_ostream &out )
{
    return CR_OK;
}

command_result weather (color_ostream &con, vector <string> & parameters)
{
    int val_override = -1;
    bool lock = false;
    bool unlock = false;
    bool snow = false;
    bool rain = false;
    bool clear = false;
    for(size_t i = 0; i < parameters.size();i++)
    {
        if(parameters[i] == "rain")
            rain = true;
        else if(parameters[i] == "snow")
            snow = true;
        else if(parameters[i] == "clear")
            clear = true;
        else if(parameters[i] == "lock")
            lock = true;
        else if(parameters[i] == "unlock")
            unlock = true;
        else
        {
            val_override = atoi(parameters[i].c_str());
            if(val_override == 0)
                return CR_WRONG_USAGE;
        }
    }
    if(lock && unlock)
    {
        con << "Lock or unlock? DECIDE!" << std::endl;
        return CR_FAILURE;
    }
    int cnt = 0;
    cnt += rain;
    cnt += snow;
    cnt += clear;
    if(cnt > 1)
    {
        con << "Rain, snow or clear sky? DECIDE!" << std::endl;
        return CR_FAILURE;
    }
    bool something = lock || unlock || rain || snow || clear || val_override != -1;

    CoreSuspender suspend;

    DFHack::World * w = Core::getInstance().getWorld();
    if(!w->wmap)
    {
        con << "Weather support seems broken :(" << std::endl;
        return CR_FAILURE;
    }
    if(!something)
    {
        // paint weather map
        con << "Weather map (C = clear, R = rain, S = snow):" << std::endl;
        for(int y = 0; y<5;y++)
        {
            for(int x = 0; x<5;x++)
            {
                switch((*w->wmap)[x][y])
                {
                case CLEAR:
                    con << "C ";
                    break;
                case RAINING:
                    con << "R ";
                    break;
                case SNOWING:
                    con << "S ";
                    break;
                default:
                    con << (int) (*w->wmap)[x][y] << " ";
                    break;
                }
            }
            con << std::endl;
        }
    }
    else
    {
        // weather changing action!
        if(rain)
        {
            con << "Here comes the rain." << std::endl;
            w->SetCurrentWeather(RAINING);
        }
        if(snow)
        {
            con << "Snow everywhere!" << std::endl;
            w->SetCurrentWeather(SNOWING);
        }
        if(clear)
        {
            con << "Suddenly, sunny weather!" << std::endl;
            w->SetCurrentWeather(CLEAR);
        }
        if(val_override != -1)
        {
            con << "I have no damn idea what this is... " << val_override << std::endl;
            w->SetCurrentWeather(val_override);
        }
        // FIXME: weather lock needs map ID to work reliably... needs to be implemented.
    }
    return CR_OK;
}
