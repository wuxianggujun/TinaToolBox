//
// Created by wuxianggujun on 2025/1/23.
//

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept> // for exceptions

namespace TinaToolBox
{
    class TTBPacker
    {
    public:
        static void createExecutable(const std::string& ttbFile, const std::string& exeFile);

    private:
        static std::vector<uint8_t> loadMinimalExeTemplate();
        static size_t findDataSegmentOffset(const std::vector<uint8_t>& exeTemplate);
        static std::vector<uint8_t> compressTTBData(const std::string& ttbFile);
        static void injectData(std::vector<uint8_t>& exeTemplate, size_t dataOffset,
                               const std::vector<uint8_t>& ttbData);
        static void updatePEHeaders(std::vector<uint8_t>& exeTemplate, size_t ttbDataSize);
        static void writeExecutable(const std::vector<uint8_t>& exeTemplate, const std::string& exeFile);
    };
} // TinaToolBox
