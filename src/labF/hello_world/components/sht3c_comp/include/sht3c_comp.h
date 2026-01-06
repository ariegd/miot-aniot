#pragma once
#include "driver/i2c_master.h"

// La función de inicio ahora recibe el bus handle
void shtc3_start(i2c_master_bus_handle_t bus_handle);
