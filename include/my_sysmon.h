#pragma once

#define SysMon_TtaskDefault 10000

extern bool v_b_SysMon_info_to_mqtt;
extern bool v_b_SysMon_info_to_rsdebug;

void SysMon_Init(void);
void cb_ut_sysmon(void);
