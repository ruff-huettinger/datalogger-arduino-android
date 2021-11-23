/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <mbed.h>
#include "ble/BLE.h"
#include "gap/Gap.h"
#include "gap/AdvertisingDataParser.h"

using namespace mbed;
using namespace ble;

inline void print_error(ble_error_t error, const char *msg)
{
    printf("%s: ", msg);
    switch (error)
    {
    case BLE_ERROR_NONE:
        printf("BLE_ERROR_NONE: No error");
        break;
    case BLE_ERROR_BUFFER_OVERFLOW:
        printf("BLE_ERROR_BUFFER_OVERFLOW: The requested action would cause a buffer overflow and has been aborted");
        break;
    case BLE_ERROR_NOT_IMPLEMENTED:
        printf("BLE_ERROR_NOT_IMPLEMENTED: Requested a feature that isn't yet implement or isn't supported by the target HW");
        break;
    case BLE_ERROR_PARAM_OUT_OF_RANGE:
        printf("BLE_ERROR_PARAM_OUT_OF_RANGE: One of the supplied parameters is outside the valid range");
        break;
    case BLE_ERROR_INVALID_PARAM:
        printf("BLE_ERROR_INVALID_PARAM: One of the supplied parameters is invalid");
        break;
    case BLE_STACK_BUSY:
        printf("BLE_STACK_BUSY: The stack is busy");
        break;
    case BLE_ERROR_INVALID_STATE:
        printf("BLE_ERROR_INVALID_STATE: Invalid state");
        break;
    case BLE_ERROR_NO_MEM:
        printf("BLE_ERROR_NO_MEM: Out of Memory");
        break;
    case BLE_ERROR_OPERATION_NOT_PERMITTED:
        printf("BLE_ERROR_OPERATION_NOT_PERMITTED");
        break;
    case BLE_ERROR_INITIALIZATION_INCOMPLETE:
        printf("BLE_ERROR_INITIALIZATION_INCOMPLETE");
        break;
    case BLE_ERROR_ALREADY_INITIALIZED:
        printf("BLE_ERROR_ALREADY_INITIALIZED");
        break;
    case BLE_ERROR_UNSPECIFIED:
        printf("BLE_ERROR_UNSPECIFIED: Unknown error");
        break;
    case BLE_ERROR_INTERNAL_STACK_FAILURE:
        printf("BLE_ERROR_INTERNAL_STACK_FAILURE: internal stack faillure");
        break;
    }
    printf("\r\n");
}
