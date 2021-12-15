#include "BleRepeater.h"

/* Uncomment and add this to BLERoles.h
#define BLE_ROLE_OBSERVER 1
#define BLE_ROLE_BROADCASTER 1
#define BLE_FEATURE_GATT_SERVER 1 
#define BLE_FEATURE_GATT_CLIENT 1
#define BLE_ROLE_PERIPHERAL 1
#define BLE_ROLE_CENTRAL 1
*/

// toDo: handle the gap callbacks for both client and server
// toDo: continue transfering from "main_old.cpp" to new OO-Project

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    thread_sleep_for(1000);

    BleRepeater rep;
    rep.init();
}

void loop() {}