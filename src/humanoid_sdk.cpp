#include "humanoid_sdk.h"

using namespace humanoid_sdk;

HumanoidSDK::HumanoidSDK() : is_running(true), communication_thread(&HumanoidSDK::communication, this) {
    timer_management.add_timer([this]() {
        send_cmd_with_data(CMD_HEART, nullptr, 0);
    }, std::chrono::milliseconds(500));

    register_cmd_callback(CMD_ECHO_REQUEST, [this](const std::vector<uint8_t> &data) {
        send_cmd_with_data(CMD_ECHO_RESPONSE, data.data(), data.size());
    });

    register_cmd_callback(CMD_CONSOLE_OUTPUT, [this](const std::vector<uint8_t> &data) {
        std::string out_str((const char*)data.data(), data.size());
        console_output_stream << out_str;
        // fmt::print("{}", out_str);
    });
}

std::string HumanoidSDK::scan_robot() {
    std::vector<serial::PortInfo> devices_found = serial::list_ports();

    for (const serial::PortInfo &port_info: devices_found) {
        if (port_info.vid == 0x0483 && port_info.pid == 0x5740) {
            return port_info.port;
        }
    }

    return {};
}

void HumanoidSDK::connect(const std::string &port_name) {
    auto timeout = serial::Timeout::simpleTimeout(200);

    serial_port.setPort(port_name);
    serial_port.setBaudrate(921600);
    serial_port.setTimeout(timeout);
    serial_port.setBytesize(serial::eightbits);
    serial_port.setParity(serial::parity_none);
    serial_port.setStopbits(serial::stopbits_one);
    serial_port.setFlowcontrol(serial::flowcontrol_none);

    serial_port.open();
}

void HumanoidSDK::communication() {
    unpack_data_t unpack_data_obj;
    protocol_initialize_unpack_object(&unpack_data_obj);

    while (is_running) {
        if (!serial_port.isOpen()) {
            auto serial_name = scan_robot();
            if (!serial_name.empty()) {
                try {
                    connect(serial_name);
                } catch (serial::IOException &e) {
                    fmt::print("Cannot open serial: {}, {}\n", serial_name, e.what());
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
        else {
            try {
                uint8_t ch;
                if (serial_port.read(&ch, 1)) {
                    if (protocol_unpack_byte(&unpack_data_obj, ch)) {
                        dispatch_frame(unpack_data_obj.cmd_id, unpack_data_obj.data, unpack_data_obj.data_len);
                    }
                }
            } catch (serial::IOException &e) {
                handle_serial_error(e);
            }
        }
    }
}

HumanoidSDK::~HumanoidSDK() {
    is_running = false;
    communication_thread.join();
}

void HumanoidSDK::dispatch_frame(uint16_t cmd_id, const uint8_t *p_data, uint16_t len) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);

    std::vector<uint8_t> data(p_data, p_data + len);

    if (callback_map.count(cmd_id) > 0) {
        callback_map[cmd_id](data);
    }

}

bool HumanoidSDK::is_connected() {
    return serial_port.isOpen();
}

size_t HumanoidSDK::send_cmd_with_data(uint16_t cmd_id, const uint8_t *p_data, uint16_t len) {
    static uint8_t frame_buffer[PROTOCOL_FRAME_MAX_SIZE];

    if (len > PROTOCOL_DATA_MAX_SIZE)
        len = PROTOCOL_DATA_MAX_SIZE;

    uint32_t frame_size = protocol_pack_data_to_buffer(cmd_id, p_data, len, frame_buffer);

    if (serial_port.isOpen()) {
        try {
            //FIXME: write operation takes about 100ms in windows.
            serial_port.write(frame_buffer, frame_size);
        } catch (serial::IOException &e) {
            handle_serial_error(e);
            return 0;
        }
    }

    return len;
}

void HumanoidSDK::register_cmd_callback(uint16_t cmd_id, ReceivedCallback callback) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    callback_map[cmd_id] = std::move(callback);
}

void HumanoidSDK::remove_cmd_callback(uint16_t cmd_id) {
    std::lock_guard<std::mutex> lock(callback_map_mutex);
    callback_map.erase(cmd_id);
}

bool HumanoidSDK::rpc_call(uint16_t request_cmd_id, const void *request_data, uint16_t request_size,
                           uint16_t response_cmd_id, void *response_data, const std::chrono::milliseconds &timeout) {
    std::mutex rpc_mutex;
    std::condition_variable rpc_condition_variable;

    register_cmd_callback(response_cmd_id, [&](const std::vector<uint8_t> &data) {
        {
            std::lock_guard<std::mutex> lock(rpc_mutex);
            memcpy(response_data, data.data(), data.size());
        }
        rpc_condition_variable.notify_one();
    });

    send_cmd_with_data(request_cmd_id, (uint8_t*)request_data, request_size);

    std::cv_status ret;
    {
        std::unique_lock<std::mutex> lk(rpc_mutex);
        ret = rpc_condition_variable.wait_for(lk, timeout);
    }

    remove_cmd_callback(response_cmd_id);

    if (ret == std::cv_status::timeout) {
        return false;
    }
    return true;
}

bool HumanoidSDK::read_uid(std::string &uid) {
    cmd_read_uid_response_t res;
    if (rpc_call(CMD_READ_UID_REQUEST, nullptr, 0, CMD_READ_UID_RESPONSE, &res, std::chrono::milliseconds(100))) {
        uid = std::string((char *) res.uid, 12);
        return true;
    }
    return false;
}

bool HumanoidSDK::read_temperature(float &temperature) {
    cmd_read_temperature_response_t res;
    if (rpc_call(CMD_READ_TEMPERATURE_REQUEST, nullptr, 0, CMD_READ_TEMPERATURE_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        temperature = res.temperature;
        return true;
    }
    return false;
}

bool HumanoidSDK::linear_actuator_set_target(uint8_t id, uint16_t target, LinearActuatorFeedback &feedback) {
    cmd_linear_actuator_set_target_t req;
    req.id = id;
    req.target = target;
    cmd_linear_actuator_feedback_t res;
    if (rpc_call(CMD_LINEAR_ACTUATOR_SET_TARGET_REQUEST, &req, sizeof(req), CMD_LINEAR_ACTUATOR_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        linear_actuator_response_to_feedback(res, feedback);
        return true;
    }
    return false;
}

bool HumanoidSDK::linear_actuator_follow(uint8_t id, uint16_t target, LinearActuatorFeedback &feedback) {
    cmd_linear_actuator_follow_t req;
    req.id = id;
    req.target = target;
    cmd_linear_actuator_feedback_t res;
    if (rpc_call(CMD_LINEAR_ACTUATOR_FOLLOW_REQUEST, &req, sizeof(req), CMD_LINEAR_ACTUATOR_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        linear_actuator_response_to_feedback(res, feedback);
        return true;
    }
    return false;
}

bool HumanoidSDK::linear_actuator_enable(uint8_t id, LinearActuatorFeedback &feedback) {
    cmd_linear_actuator_enable_t req;
    req.id = id;
    cmd_linear_actuator_feedback_t res;
    if (rpc_call(CMD_LINEAR_ACTUATOR_ENABLE_REQUEST, &req, sizeof(req), CMD_LINEAR_ACTUATOR_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        linear_actuator_response_to_feedback(res, feedback);
        return true;
    }
    return false;
}

void HumanoidSDK::linear_actuator_response_to_feedback(cmd_linear_actuator_feedback_t &res,
                                                       LinearActuatorFeedback &feedback) {
    feedback.id = res.id;
    feedback.target_position = res.target_position;
    feedback.current_position = res.current_position;
    feedback.temperature = res.temperature;
    feedback.force_sensor = res.force_sensor;
    feedback.error_code = res.error_code;
    feedback.internal_data1 = res.internal_data1;
    feedback.internal_data2 = res.internal_data2;
}

bool HumanoidSDK::linear_actuator_stop(uint8_t id, LinearActuatorFeedback &feedback) {
    cmd_linear_actuator_stop_t req;
    req.id = id;
    cmd_linear_actuator_feedback_t res;
    if (rpc_call(CMD_LINEAR_ACTUATOR_STOP_REQUEST, &req, sizeof(req), CMD_LINEAR_ACTUATOR_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        linear_actuator_response_to_feedback(res, feedback);
        return true;
    }
    return false;
}

bool HumanoidSDK::linear_actuator_pause(uint8_t id, LinearActuatorFeedback &feedback) {
    cmd_linear_actuator_pause_t req;
    req.id = id;
    cmd_linear_actuator_feedback_t res;
    if (rpc_call(CMD_LINEAR_ACTUATOR_PAUSE_REQUEST, &req, sizeof(req), CMD_LINEAR_ACTUATOR_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        linear_actuator_response_to_feedback(res, feedback);
        return true;
    }
    return false;
}

bool HumanoidSDK::linear_actuator_save_parameters(uint8_t id, LinearActuatorFeedback &feedback) {
    cmd_linear_actuator_save_parameters_t req;
    req.id = id;
    cmd_linear_actuator_feedback_t res;
    if (rpc_call(CMD_LINEAR_ACTUATOR_SAVE_PARAMETERS_REQUEST, &req, sizeof(req), CMD_LINEAR_ACTUATOR_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        linear_actuator_response_to_feedback(res, feedback);
        return true;
    }
    return false;
}

bool HumanoidSDK::linear_actuator_query_state(uint8_t id, LinearActuatorFeedback &feedback) {
    cmd_linear_actuator_query_state_t req;
    req.id = id;
    cmd_linear_actuator_feedback_t res;
    if (rpc_call(CMD_LINEAR_ACTUATOR_QUERY_STATE_REQUEST, &req, sizeof(req), CMD_LINEAR_ACTUATOR_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        linear_actuator_response_to_feedback(res, feedback);
        return true;
    }
    return false;
}

bool HumanoidSDK::linear_actuator_clear_error(uint8_t id, LinearActuatorFeedback &feedback) {
    cmd_linear_actuator_clear_error_t req;
    req.id = id;
    cmd_linear_actuator_feedback_t res;
    if (rpc_call(CMD_LINEAR_ACTUATOR_CLEAR_ERROR_REQUEST, &req, sizeof(req), CMD_LINEAR_ACTUATOR_RESPONSE, &res,
                 std::chrono::milliseconds(100))) {
        linear_actuator_response_to_feedback(res, feedback);
        return true;
    }
    return false;
}

bool HumanoidSDK::write_console(const std::string &s) {
    auto *ptr = (uint8_t*)s.data();
    size_t len = s.size();
    size_t send_size;
    while(len > 0) {
        send_size = send_cmd_with_data(CMD_WRITE_CONSOLE, ptr, len);
        len -= send_size;
        ptr += send_size;
    }
    return true;
}

bool HumanoidSDK::console_output(std::string &s) {
    s = console_output_stream.str();
    console_output_stream.str(std::string());
    return true;
}

bool HumanoidSDK::set_maestro_channel(uint8_t channel, uint16_t target) {
    cmd_set_maestro_channel_t msg;
    msg.channel = channel;
    msg.target = target;
    send_cmd_with_data(CMD_SET_MAESTRO_CHANNEL, (uint8_t*)&msg, sizeof(msg));
    return true;
}

bool HumanoidSDK::set_maestro_all_channel(const std::vector<uint16_t>& targets) {
    cmd_set_maestro_all_channel_t msg;
    for (size_t i = 0; i < std::min<size_t>(targets.size(), 24); ++i) {
        msg.targets[i] = targets[i];
    }
    send_cmd_with_data(CMD_SET_MAESTRO_ALL_CHANNEL, (uint8_t*)&msg, sizeof(msg));
    return true;
}

bool HumanoidSDK::linear_actuator_set_target_silent(uint8_t id, uint16_t target) {
    cmd_linear_actuator_set_target_silent_t msg;
    msg.id = id;
    msg.target = target;
    send_cmd_with_data(CMD_LINEAR_ACTUATOR_SET_TARGET_SILENT, (uint8_t*)&msg, sizeof(msg));
    return true;
}

bool HumanoidSDK::linear_actuator_follow_silent(uint8_t id, uint16_t target) {
    cmd_linear_actuator_follow_silent_t msg;
    msg.id = id;
    msg.target = target;
    send_cmd_with_data(CMD_LINEAR_ACTUATOR_FOLLOW_SILENT, (uint8_t*)&msg, sizeof(msg));
    return true;
}

bool HumanoidSDK::linear_actuator_broadcast_targets(const std::vector<uint8_t> &ids,
                                                        const std::vector<uint16_t> &targets) {
    cmd_linear_actuator_broadcast_targets_t msg;
    size_t cnt = std::min<size_t>(std::min<size_t>(ids.size(), targets.size()), 10);
    for (size_t i = 0; i < cnt; ++i) {
        msg.num = cnt;
        msg.ids[i] = ids[i];
        msg.targets[i] = targets[i];
    }
    send_cmd_with_data(CMD_LINEAR_ACTUATOR_BROADCAST_TARGETS, (uint8_t*)&msg, sizeof(msg));
    return true;
}

bool HumanoidSDK::linear_actuator_broadcast_follows(const std::vector<uint8_t> &ids, const std::vector<uint16_t> &targets) {
    cmd_linear_actuator_broadcast_follows_t msg;
    size_t cnt = std::min<size_t>(std::min<size_t>(ids.size(), targets.size()), 10);
    for (size_t i = 0; i < cnt; ++i) {
        msg.num = cnt;
        msg.ids[i] = ids[i];
        msg.targets[i] = targets[i];
    }
    send_cmd_with_data(CMD_LINEAR_ACTUATOR_BROADCAST_FOLLOWS, (uint8_t*)&msg, sizeof(msg));
    return true;
}

void HumanoidSDK::handle_serial_error(serial::IOException &e) {
    fmt::print(stderr, "Serial port error: {}\n", e.what());
    try {
        serial_port.close();
    } catch (serial::IOException& ee) {
        fmt::print(stderr, "Close serial port error: {}\n", ee.what());
    }
}

