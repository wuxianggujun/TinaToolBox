#include <iostream>
#include "TTBPacker.hpp"

using namespace TinaToolBox;

int main(int argc, char* argv[]) {
    try {
        TTBPacker packer;
        
        // 设置进度回调
        packer.setProgressCallback([](const std::string& message, int progress) {
            std::cout << "[" << progress << "%] " << message << std::endl;
        });

        // 提取并执行TTB文件
        auto result = packer.extractAndExecute(argv[0]);
        
        if (result != TTBPacker::Error::SUCCESS) {
            std::cerr << "Failed to execute TTB script: " << packer.getLastError() << std::endl;
            return 1;
        }

        std::cout << "Script executed successfully" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
} 