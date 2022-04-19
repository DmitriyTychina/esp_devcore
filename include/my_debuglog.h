#ifndef my_debuglog_h
#define my_debuglog_h
#include <Arduino.h>
#include "RSDebug.h"

#define RSDebug_TtaskDefault 249

extern RSDebug Debug;

void init_sdebuglog(bool force = false);
void init_rdebuglog();
void cb_my_debuglog();

inline void my_debuglog_print_free_memory()
{
    rsdebugDnfln("Free memory: %u", ESP.getFreeHeap());
};

    // // debuglog на Serial1 -указать в #define ser Serial //// ESP8266 - io2 buildin led blue
    // #define log_ 1
    // #if log_

    // #define ser Serial
    // // extern bool enable_log = true;
    // // #define logenable() all_param_RAM.log = true
    // // #define logdisable() all_param_RAM.log = false
    // #define loginit() ser.begin(115200)
    // // #define islogenable() all_param_RAM.log

#endif // my_debuglog_h
