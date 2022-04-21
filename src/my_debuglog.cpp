#include <ESP8266WiFi.h>
#include "device_def.h"
#include "my_debuglog.h"
#include "my_scheduler.h"
#include "main.h"
#include "my_EEPROM.h"

// unsigned long baud = 115200; // If we set baud = 0, it will disable Serial1 output
// uint16_t port = 23;          // If we set port = 0, it will disable telnet server
// bool insertTimestamp = true; // We can enable/disable time stamping
// int16_t tzcorr = 0;          // Number of seconds to correct the timestamp.

RSDebug Debug(115200);
// RSDebug Debug; // по умолчанию скорость 115200

void init_sdebuglog(bool force)
{
    // Initialize RSDebug
    if (force)
        Debug.setSdebugEnabled(true); // if you wants Serial1 echo - only recommended if ESP is plugged in USB
    else
    {
        LoadInMemorySettingsSys();
        Debug.setSdebugEnabled(g_p_sys_settings_ROM->RSDebug_SDebug);
    }
}

void init_rdebuglog()
{
    LoadInMemorySettingsSys();
    ut_debuglog.setInterval(g_p_sys_settings_ROM->RSDebug_Ttask);
    Debug.setRdebugEnabled(g_p_sys_settings_ROM->RSDebug_RDebug);
    // Initialize RSDebug
    if (Debug.isRdebugEnabled())
    {
        Debug.begin(OTA_NAME, Debug.VERBOSE);
        Debug.setResetCmdEnabled(true);
        // Enable the reset command
        // Debug.showColors(true);         // Colors
        // Project commands
        // String helpCmd = "bench1 - Benchmark 1\n";
        // helpCmd.concat("bench2 - Benchmark 2");
        // Debug.setHelpProjectsCmds(helpCmd);
        // Debug.setCallBackProjectCmds(&processCmdRSDebug);
        // End of setup - show IP
        // Serial1.println("* Arduino RSDebug Library");
        // Serial1.println("*");
        // Serial1.print("* WiFI connected. IP address: ");
        // Serial1.println(WiFi.localIP());
        // Serial1.println("*");
        // Debug.setRdebugEnabled(true);
        if (!ut_debuglog.isEnabled())
            ut_debuglog.enable();
    }
    else
    {
        Debug.disconnectClient();
        Debug.stop();
        if (ut_debuglog.isEnabled())
            ut_debuglog.disable();
    }
}

void cb_ut_debuglog()
{
    Debug.handle();
}
