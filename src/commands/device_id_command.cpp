#include "commands/device_id_command.h"

#include "config.h"
#include "diagnostics.h"
#include "interface/usb.h"
#include "util/log.h"
#include "config.h"
#include "pb_decode.h"
#include <payload/payload.h>
#include "signals.h"
#include <can/canutil.h>
#include <bitfield/bitfield.h>
#include <limits.h>

using openxc::interface::usb::sendControlMessage;
using openxc::util::log::debug;
using openxc::config::getConfiguration;
using openxc::payload::PayloadFormat;
using openxc::signals::getCanBuses;
using openxc::signals::getCanBusCount;
using openxc::signals::getSignals;
using openxc::signals::getSignalCount;
using openxc::signals::getCommands;
using openxc::signals::getCommandCount;
using openxc::can::lookupBus;
using openxc::can::lookupSignal;

namespace can = openxc::can;
namespace payload = openxc::payload;
namespace config = openxc::config;
namespace diagnostics = openxc::diagnostics;
namespace usb = openxc::interface::usb;
namespace uart = openxc::interface::uart;
namespace pipeline = openxc::pipeline;

bool openxc::commands::handleDeviceIdCommmand() {
    // TODO move getDeviceId to openxc::platform, allow each platform to
    // define where the device ID comes from.
    uart::UartDevice* uart = &getConfiguration()->uart;
    size_t deviceIdLength = strnlen(uart->deviceId, sizeof(uart->deviceId));
    if(deviceIdLength > 0) {
        usb::sendControlMessage(&getConfiguration()->usb,
                (uint8_t*)uart->deviceId, strlen(uart->deviceId));

        openxc_VehicleMessage message = {0};
        message.has_type = true;
        message.type = openxc_VehicleMessage_Type_COMMAND_RESPONSE;
        message.has_command_response = true;
        message.command_response.has_type = true;
        message.command_response.type = openxc_ControlCommand_Type_DEVICE_ID;
        message.command_response.has_message = true;
        memset(message.command_response.message, 0,
                sizeof(message.command_response.message));
        strncpy(message.command_response.message, uart->deviceId,
                deviceIdLength);
        pipeline::publish(&message, &getConfiguration()->pipeline);
    }
    return true;
}
