#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "humanoid_sdk.h"

namespace py = pybind11;
using namespace pybind11::literals;

using humanoid_sdk::LinearActuatorFeedback;

class LinearActuator {
public:
    LinearActuator() : sdk(humanoid_sdk::HumanoidSDK::get_instance()) { }

    LinearActuatorFeedback set_target(uint8_t id, uint16_t target) {
        LinearActuatorFeedback feedback;
        if(!sdk.linear_actuator_set_target(id, target, feedback)) {
            throw std::runtime_error("Timeout");
        }
        return feedback;
    }

    LinearActuatorFeedback follow(uint8_t id, uint16_t target) {
        LinearActuatorFeedback feedback;
        if(!sdk.linear_actuator_follow(id, target, feedback)) {
            throw std::runtime_error("Timeout");
        }
        return feedback;
    }

    LinearActuatorFeedback enable(uint8_t id) {
        LinearActuatorFeedback feedback;
        if(!sdk.linear_actuator_enable(id, feedback)) {
            throw std::runtime_error("Timeout");
        }
        return feedback;
    }

    LinearActuatorFeedback stop(uint8_t id) {
        LinearActuatorFeedback feedback;
        if(!sdk.linear_actuator_stop(id, feedback)) {
            throw std::runtime_error("Timeout");
        }
        return feedback;
    }

    LinearActuatorFeedback pause(uint8_t id) {
        LinearActuatorFeedback feedback;
        if(!sdk.linear_actuator_pause(id, feedback)) {
            throw std::runtime_error("Timeout");
        }
        return feedback;
    }

    LinearActuatorFeedback save_parameters(uint8_t id) {
        LinearActuatorFeedback feedback;
        if(!sdk.linear_actuator_save_parameters(id, feedback)) {
            throw std::runtime_error("Timeout");
        }
        return feedback;
    }

    LinearActuatorFeedback query_state(uint8_t id) {
        LinearActuatorFeedback feedback;
        if(!sdk.linear_actuator_query_state(id, feedback)) {
            throw std::runtime_error("Timeout");
        }
        return feedback;
    }

    LinearActuatorFeedback clear_error(uint8_t id) {
        LinearActuatorFeedback feedback;
        if(!sdk.linear_actuator_clear_error(id, feedback)) {
            throw std::runtime_error("Timeout");
        }
        return feedback;
    }

    void set_target_silent(uint8_t id, uint16_t target) {
        sdk.linear_actuator_set_target_silent(id, target);
    }

    void follow_silent(uint8_t id, uint16_t target) {
        sdk.linear_actuator_follow_silent(id, target);
    }

    void broadcast_targets(const std::vector<uint8_t>& ids, const std::vector<uint16_t>& targets) {
        sdk.linear_actuator_broadcast_targets(ids, targets);
    }

    void broadcast_follows(const std::vector<uint8_t>& ids, const std::vector<uint16_t>& targets) {
        sdk.linear_actuator_broadcast_follows(ids, targets);
    }

private:
    humanoid_sdk::HumanoidSDK& sdk;
};

class Maestro {
public:
    Maestro() : sdk(humanoid_sdk::HumanoidSDK::get_instance()) { }

    void set_channel(uint8_t channel, uint16_t target) {
        sdk.set_maestro_channel(channel, target);
    }

    void set_all_channel(const std::vector<uint16_t>& targets) {
        sdk.set_maestro_all_channel(targets);
    };

private:
    humanoid_sdk::HumanoidSDK& sdk;
};

class HumanoidSDK {
public:
    HumanoidSDK() : sdk(humanoid_sdk::HumanoidSDK::get_instance()) { }

    LinearActuator linear_actuator;

    Maestro maestro;

    bool is_connected() {
        return sdk.is_connected();
    }

    py::bytes read_uid() {
        std::string uid;
        if(!sdk.read_uid(uid)) {
            throw std::runtime_error("Timeout");
        }
        return uid;
    }

    float read_temperature() {
        float temperature;
        if(!sdk.read_temperature(temperature)) {
            throw std::runtime_error("Timeout");
        }
        return temperature;
    }

    void write_console(const std::string &s) {
        sdk.write_console(s);
    }

    std::string console_output() {
        std::string s;
        sdk.console_output(s);
        return s;
    }

private:
    humanoid_sdk::HumanoidSDK& sdk;
};

PYBIND11_MODULE(py_humanoid_sdk, m) {
    m.doc() = "humanoid_sdk python wrapper.";

    py::class_<HumanoidSDK>(m, "HumanoidSDK")
        .def(py::init<>())
        .def_readonly("linear_actuator", &HumanoidSDK::linear_actuator)
        .def_readonly("maestro", &HumanoidSDK::maestro)
        .def("is_connected", &HumanoidSDK::is_connected)
        .def("read_uid", &HumanoidSDK::read_uid)
        .def("read_temperature", &HumanoidSDK::read_temperature)
        .def("write_console", &HumanoidSDK::write_console, "s"_a)
        .def("console_output", &HumanoidSDK::console_output);

    py::class_<LinearActuator>(m, "LinearActuator")
        .def(py::init<>())
        .def("set_target", &LinearActuator::set_target, "id"_a, "target"_a)
        .def("follow", &LinearActuator::follow, "id"_a, "target"_a)
        .def("enable", &LinearActuator::enable, "id"_a)
        .def("stop", &LinearActuator::stop, "id"_a)
        .def("pause", &LinearActuator::pause, "id"_a)
        .def("save_parameters", &LinearActuator::save_parameters, "id"_a)
        .def("query_state", &LinearActuator::query_state, "id"_a)
        .def("clear_error", &LinearActuator::clear_error, "id"_a)
        .def("set_target_silent", &LinearActuator::set_target_silent, "id"_a, "target"_a)
        .def("follow_silent", &LinearActuator::follow_silent, "id"_a, "target"_a)
        .def("broadcast_targets", &LinearActuator::broadcast_targets, "ids"_a, "targets"_a)
        .def("broadcast_follows", &LinearActuator::broadcast_follows, "ids"_a, "targets"_a);

    py::class_<Maestro>(m, "Maestro")
        .def(py::init<>())
        .def("set_channel", &Maestro::set_channel, "channel"_a, "target"_a)
        .def("set_all_channel", &Maestro::set_all_channel, "targets"_a);

    py::class_<LinearActuatorFeedback>(m, "LinearActuatorFeedback")
        .def_readwrite("id", &LinearActuatorFeedback::id)
        .def_readwrite("target_position", &LinearActuatorFeedback::target_position)
        .def_readwrite("current_position", &LinearActuatorFeedback::current_position)
        .def_readwrite("temperature", &LinearActuatorFeedback::temperature)
        .def_readwrite("force_sensor", &LinearActuatorFeedback::force_sensor)
        .def_readwrite("error_code", &LinearActuatorFeedback::error_code)
        .def_readwrite("internal_data1", &LinearActuatorFeedback::internal_data1)
        .def_readwrite("internal_data2", &LinearActuatorFeedback::internal_data2)
        .def("__str__", [](const LinearActuatorFeedback &feedback) {
            return fmt::format("Linear actuator feedback\n"
                               "id: {:d}\n"
                               "target_position: {:d}\n"
                               "current_position: {:d}\n"
                               "temperature: {:d}℃\n"
                               "force_sensor: {:d}g\n"
                               "error_code: {:02x}\n"
                               "internal_data1: {:04x}\n"
                               "internal_data2: {:04x}", feedback.id, feedback.target_position,
                               feedback.current_position, feedback.temperature, feedback.force_sensor,
                               feedback.error_code, feedback.internal_data1, feedback.internal_data2);
        });

}
