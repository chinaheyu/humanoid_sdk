#ifndef __HUMANOID_SDK_HPP__
#define __HUMANOID_SDK_HPP__

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <sstream>
#include "serial/serial.h"
#include "protocol_lite.h"
#include "timer.h"
#include "fmt/format.h"
#include "protocol_definition.h"

namespace humanoid_sdk {

    using ReceivedCallback = std::function<void(const std::vector<uint8_t> &)>;

    struct LinearActuatorFeedback {
        uint8_t id;
        uint16_t target_position;
        uint16_t current_position;
        uint8_t temperature;
        uint16_t force_sensor;
        uint8_t error_code;
        uint16_t internal_data1;
        uint16_t internal_data2;
    };

    class HumanoidSDK {

    public:
        static HumanoidSDK& get_instance()
        {
            static HumanoidSDK instance;
            return instance;
        }

        HumanoidSDK(HumanoidSDK const&) = delete;
        void operator=(HumanoidSDK const&) = delete;

        ~HumanoidSDK();

        bool is_connected();

        bool read_uid(std::string &uid);

        bool read_temperature(float &temperature);

        bool linear_actuator_set_target(uint8_t id, uint16_t target, LinearActuatorFeedback &feedback);

        bool linear_actuator_follow(uint8_t id, uint16_t target, LinearActuatorFeedback &feedback);

        bool linear_actuator_enable(uint8_t id, LinearActuatorFeedback &feedback);

        bool linear_actuator_stop(uint8_t id, LinearActuatorFeedback &feedback);

        bool linear_actuator_pause(uint8_t id, LinearActuatorFeedback &feedback);

        bool linear_actuator_save_parameters(uint8_t id, LinearActuatorFeedback &feedback);

        bool linear_actuator_query_state(uint8_t id, LinearActuatorFeedback &feedback);

        bool linear_actuator_clear_error(uint8_t id, LinearActuatorFeedback &feedback);

        bool write_console(const std::string &s);

        bool console_output(std::string& s);

        bool set_maestro_channel(uint8_t channel, uint16_t target);

        bool set_maestro_all_channel(const std::vector<uint16_t>& targets);

        bool linear_actuator_set_target_silent(uint8_t id, uint16_t target);

        bool linear_actuator_follow_silent(uint8_t id, uint16_t target);

        bool linear_actuator_broadcast_targets(const std::vector<uint8_t>& ids, const std::vector<uint16_t>& targets);

        bool linear_actuator_broadcast_follows(const std::vector<uint8_t>& ids, const std::vector<uint16_t>& targets);

    private:
        HumanoidSDK();

        std::atomic<bool> is_running;
        serial::Serial serial_port;
        std::thread communication_thread;
        TimerManagement timer_management;
        std::mutex callback_map_mutex;
        std::unordered_map<uint16_t, ReceivedCallback> callback_map;
        std::stringstream console_output_stream;

        std::string scan_robot();

        void connect(const std::string &port_name);

        void communication();

        void handle_serial_error(serial::IOException &e);

        void dispatch_frame(uint16_t cmd_id, const uint8_t *p_data, uint16_t len);

        size_t send_cmd_with_data(uint16_t cmd_id, const uint8_t *p_data, uint16_t len);

        void register_cmd_callback(uint16_t cmd_id, ReceivedCallback callback);

        void remove_cmd_callback(uint16_t cmd_id);

        bool rpc_call(uint16_t request_cmd_id,
                      const void *request_data,
                      uint16_t request_size,
                      uint16_t response_cmd_id,
                      void *response_data,
                      const std::chrono::milliseconds &timeout);

        void linear_actuator_response_to_feedback(cmd_linear_actuator_feedback_t& res, LinearActuatorFeedback& feedback);

    };
}

#endif // __HUMANOID_SDK_HPP__
