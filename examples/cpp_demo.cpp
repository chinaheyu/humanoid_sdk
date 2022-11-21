#include <iostream>
#include <chrono>
#include "humanoid_sdk.h"


int main(int argc, char* argv[]) {
    humanoid_sdk::HumanoidSDK& sdk = humanoid_sdk::HumanoidSDK::get_instance();

    while (true) {
        if(sdk.is_connected()) {
            auto st = std::chrono::high_resolution_clock::now();
            std::string uid;
            bool ret = sdk.read_uid(uid);
            auto et = std::chrono::high_resolution_clock::now();
            if(ret) {
                std::cout << (std::chrono::duration<double>(et - st).count() * 1000.0) << " ms" << std::endl;
            } else {
                std::cout << "Timeout." << std::endl;
            }
        } else {
            std::cout << "Disconnected." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}