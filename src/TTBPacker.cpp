//
// Created by wuxianggujun on 2025/1/23.
//

#include <iostream>
#include <windows.h>
#include <imagehlp.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "TtbPacker.hpp"
#include <zlib.h>

namespace TinaToolBox
{
    // 定义函数指针类型
    typedef BOOL (WINAPI *CHECKSUM_MAPPED_FILE)(PVOID BaseAddress, DWORD FileLength, PDWORD HeaderSum, PDWORD CheckSum);

    void TTBPacker::createExecutable(const std::string& ttbFile, const std::string& exeFile)
    {
        try
        {
            // 1. 加载模板EXE
            auto exeTemplate = loadMinimalExeTemplate();

            // 2. 查找数据段偏移
            size_t dataOffset = findDataSegmentOffset(exeTemplate);

            // 3. 压缩TTB数据
            auto compressedData = compressTTBData(ttbFile);

            // 4. 注入压缩后的数据
            injectData(exeTemplate, dataOffset, compressedData);

            // 5. 更新PE头部
            updatePEHeaders(exeTemplate, compressedData.size());

            // 6. 写入新的可执行文件
            writeExecutable(exeTemplate, exeFile);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::string("Failed to create executable: ") + e.what());
        }
    }

    std::vector<uint8_t> TTBPacker::loadMinimalExeTemplate()
    {
        // 从资源目录加载模板文件
        std::filesystem::path templatePath = std::filesystem::current_path() / "resources" / "template.exe";

        if (!std::filesystem::exists(templatePath))
        {
            throw std::runtime_error("Template EXE not found: " + templatePath.string());
        }

        std::ifstream file(templatePath, std::ios::binary | std::ios::ate);
        if (!file)
        {
            throw std::runtime_error("Failed to open template EXE: " + templatePath.string());
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        {
            throw std::runtime_error("Failed to read template EXE");
        }

        return buffer;
    }

    size_t TTBPacker::findDataSegmentOffset(const std::vector<uint8_t>& exeTemplate)
    {
        // 在PE文件中查找TTB_DATA资源
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)exeTemplate.data();
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        {
            throw std::runtime_error("Invalid DOS signature");
        }

        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(exeTemplate.data() + dosHeader->e_lfanew);
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
        {
            throw std::runtime_error("Invalid NT signature");
        }

        // 获取资源目录
        DWORD resourceRVA = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
        if (resourceRVA == 0)
        {
            throw std::runtime_error("No resource directory found");
        }

        // 遍历资源目录查找TTB_DATA资源
        // 这里需要将RVA转换为文件偏移
        // 为简化示例，这里返回一个固定偏移
        // 在实际实现中，你需要遍历PE文件的节表来找到正确的文件偏移
        return resourceRVA;
    }

    std::vector<uint8_t> TTBPacker::compressTTBData(const std::string& ttbFile)
    {
        std::ifstream file(ttbFile, std::ios::binary | std::ios::ate);
        if (!file)
        {
            throw std::runtime_error("Failed to open TTB file: " + ttbFile);
        }

        std::streamsize inputSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> input(inputSize);
        if (!file.read(reinterpret_cast<char*>(input.data()), inputSize))
        {
            throw std::runtime_error("Failed to read TTB file");
        }

        // 压缩数据
        std::vector<uint8_t> output(compressBound(inputSize));
        z_stream zs = {0};
        deflateInit(&zs, Z_BEST_COMPRESSION);

        zs.next_in = input.data();
        zs.avail_in = input.size();
        zs.next_out = output.data();
        zs.avail_out = output.size();

        deflate(&zs, Z_FINISH);
        deflateEnd(&zs);

        output.resize(zs.total_out);
        return output;
    }

    void TTBPacker::injectData(std::vector<uint8_t>& exeTemplate,
                              size_t dataOffset,
                              const std::vector<uint8_t>& ttbData)
    {
        // 更新资源大小
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)exeTemplate.data();
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(exeTemplate.data() + dosHeader->e_lfanew);

        // 找到资源节
        PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
        for (WORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
        {
            if (strcmp((char*)sectionHeader->Name, ".rsrc") == 0)
            {
                // 更新节的大小
                sectionHeader->SizeOfRawData = ttbData.size();
                sectionHeader->Misc.VirtualSize = ttbData.size();
                break;
            }
            sectionHeader++;
        }

        // 注入数据
        if (dataOffset + ttbData.size() > exeTemplate.size())
        {
            exeTemplate.resize(dataOffset + ttbData.size());
        }
        std::copy(ttbData.begin(), ttbData.end(), exeTemplate.begin() + dataOffset);
    }

    void TTBPacker::updatePEHeaders(std::vector<uint8_t>& exeTemplate, size_t ttbDataSize)
    {
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)exeTemplate.data();
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(exeTemplate.data() + dosHeader->e_lfanew);

        // 更新资源目录大小
        ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = ttbDataSize;

        // 动态加载imagehlp.dll
        HMODULE hImageHlp = LoadLibraryA("imagehlp.dll");
        if (!hImageHlp) {
            throw std::runtime_error("Failed to load imagehlp.dll");
        }

        // 获取CheckSumMappedFile函数地址
        CHECKSUM_MAPPED_FILE pfnCheckSum = (CHECKSUM_MAPPED_FILE)GetProcAddress(hImageHlp, "CheckSumMappedFile");
        if (!pfnCheckSum) {
            FreeLibrary(hImageHlp);
            throw std::runtime_error("Failed to get CheckSumMappedFile function");
        }

        // 更新文件校验和
        ntHeaders->OptionalHeader.CheckSum = 0;  // 重新计算校验和
        DWORD headerSum = 0;
        DWORD checkSum = 0;

        if (!pfnCheckSum(exeTemplate.data(), exeTemplate.size(), &headerSum, &checkSum)) {
            FreeLibrary(hImageHlp);
            throw std::runtime_error("CheckSumMappedFile failed");
        }

        ntHeaders->OptionalHeader.CheckSum = checkSum;
        FreeLibrary(hImageHlp);
    }

    void TTBPacker::writeExecutable(const std::vector<uint8_t>& exeTemplate,
                                  const std::string& exeFile)
    {
        std::ofstream file(exeFile, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to create output file: " + exeFile);
        }

        if (!file.write(reinterpret_cast<const char*>(exeTemplate.data()),
                       exeTemplate.size()))
        {
            throw std::runtime_error("Failed to write output file");
        }
    }
} // TinaToolBox
