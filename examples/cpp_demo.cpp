#include <iostream>
#include <chrono>
#include "humanoid_sdk.h"


int main(int argc, char* argv[]) {
    humanoid_sdk::HumanoidSDK& sdk = humanoid_sdk::HumanoidSDK::get_instance();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "connect: " << std::boolalpha << sdk.is_connected() << std::endl;
    }

    return 0;
}