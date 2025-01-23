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
#include <zlib.h>

#include "TTBPacker.hpp"
#include "TTBScriptEngine.hpp"
#include "TTBResourceExtractor.hpp"
#include "TTBTemporaryFile.hpp"
#include "TTBResourceHandle.hpp"

namespace TinaToolBox
{
    // 定义函数指针类型
    typedef BOOL (WINAPI *CHECKSUM_MAPPED_FILE)(PVOID BaseAddress, DWORD FileLength, PDWORD HeaderSum, PDWORD CheckSum);

    TTBPacker::Error TTBPacker::packToExecutable(
        const std::string& templatePath,
        const std::string& ttbPath,
        const std::string& outputPath)
    {
        try {
            reportProgress("Reading TTB file...", 0);
            
            // 1. 读取TTB文件
            std::ifstream ttbFile(ttbPath, std::ios::binary);
            if (!ttbFile) {
                lastError_ = "Failed to open TTB file: " + ttbPath;
                return Error::FILE_LOAD_ERROR;
            }

            ttbFile.seekg(0, std::ios::end);
            size_t ttbSize = ttbFile.tellg();
            ttbFile.seekg(0);
            std::vector<uint8_t> ttbData(ttbSize);
            ttbFile.read(reinterpret_cast<char*>(ttbData.data()), ttbSize);
            ttbFile.close();

            reportProgress("Compressing TTB data...", 20);
            
            // 2. 压缩TTB数据
            auto compressedData = compressData(ttbData);
            if (compressedData.empty()) {
                return Error::COMPRESSION_FAILED;
            }

            reportProgress("Creating output file...", 40);
            
            // 3. 复制模板文件到输出路径
            if (!std::filesystem::copy_file(templatePath, outputPath, 
                std::filesystem::copy_options::overwrite_existing)) {
                lastError_ = "Failed to create output file: " + outputPath;
                return Error::FILE_CREATE_ERROR;
            }

            reportProgress("Updating resources...", 60);
            
            // 4. 更新资源
            if (!updateResource(outputPath, compressedData)) {
                return Error::RESOURCE_UPDATE_ERROR;
            }

            reportProgress("Packing completed successfully", 100);
            return Error::SUCCESS;
        }
        catch (const std::exception& e) {
            lastError_ = std::string("Error during packing: ") + e.what();
            return Error::FILE_CREATE_ERROR;
        }
    }

    TTBPacker::Error TTBPacker::extractAndExecute(const std::string& exePath)
    {
        try {
            reportProgress("Extracting TTB data...", 0);
            
            // 1. 提取压缩的TTB数据
            TTBResourceExtractor extractor;
            auto compressedData = extractor.extractTTBData();
            if (compressedData.empty()) {
                lastError_ = "No TTB data found in executable";
                return Error::NO_TTB_DATA;
            }

            reportProgress("Decompressing data...", 30);
            
            // 2. 解压数据
            auto uncompressedData = decompressData(compressedData);
            if (uncompressedData.empty()) {
                return Error::DECOMPRESSION_FAILED;
            }

            reportProgress("Creating temporary file...", 50);
            
            // 3. 创建临时文件
            TTBTemporaryFile tempFile("ttb_script_");
            {
                std::ofstream file(tempFile.path(), std::ios::binary);
                if (!file) {
                    lastError_ = "Failed to create temporary file";
                    return Error::FILE_CREATE_ERROR;
                }
                file.write(reinterpret_cast<const char*>(uncompressedData.data()), 
                          uncompressedData.size());
            }

            reportProgress("Loading TTB file...", 70);
            
            // 4. 加载TTB文件
            std::unique_ptr<TTBFile> ttbFile;
            AESKey defaultKey = {
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
            };

            if (TTBFile::isEncrypted(tempFile.path().string())) {
                ttbFile = TTBFile::loadEncrypted(tempFile.path().string(), defaultKey);
            } else {
                ttbFile = TTBFile::load(tempFile.path().string());
            }

            if (!ttbFile) {
                lastError_ = "Failed to load TTB file";
                return Error::FILE_LOAD_ERROR;
            }

            reportProgress("Executing script...", 90);
            
            // 5. 执行脚本
            TTBScriptEngine engine;
            engine.setProgressCallback(progressCallback_);
            
            auto result = engine.executeScript(tempFile.path().string(), defaultKey, ttbFile.get());

            if (result != TTBScriptEngine::Error::SUCCESS) {
                lastError_ = engine.getLastError();
                return Error::FILE_LOAD_ERROR;
            }

            reportProgress("Script executed successfully", 100);
            return Error::SUCCESS;
        }
        catch (const std::exception& e) {
            lastError_ = std::string("Error during extraction: ") + e.what();
            return Error::FILE_LOAD_ERROR;
        }
    }

    std::vector<uint8_t> TTBPacker::compressData(const std::vector<uint8_t>& data)
    {
        if (data.empty()) {
            lastError_ = "Input data is empty";
            return {};
        }

        uLong sourceLen = data.size();
        uLong destLen = compressBound(sourceLen);
        std::vector<Bytef> compressedData(destLen);
        
        z_stream zs = {0};
        zs.zalloc = Z_NULL;
        zs.zfree = Z_NULL;
        zs.opaque = Z_NULL;
        
        if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            lastError_ = "Failed to initialize compression";
            return {};
        }

        zs.next_in = const_cast<Bytef*>(data.data());
        zs.avail_in = sourceLen;
        zs.next_out = compressedData.data();
        zs.avail_out = destLen;

        std::vector<Bytef> temp;
        while (true) {
            int ret = deflate(&zs, Z_FINISH);
            
            if (ret == Z_STREAM_END) {
                break;
            }
            
            if (ret == Z_BUF_ERROR) {
                size_t currentSize = compressedData.size();
                temp = compressedData;
                compressedData.resize(currentSize * 2);
                std::copy(temp.begin(), temp.end(), compressedData.begin());
                
                zs.next_out = compressedData.data() + zs.total_out;
                zs.avail_out = compressedData.size() - zs.total_out;
                continue;
            }
            
            if (ret < 0) {
                lastError_ = std::string("Compression error: ") + 
                            (zs.msg ? zs.msg : "Unknown error");
                deflateEnd(&zs);
                return {};
            }
        }

        deflateEnd(&zs);
        compressedData.resize(zs.total_out);
        return compressedData;
    }

    std::vector<uint8_t> TTBPacker::decompressData(const std::vector<uint8_t>& data)
    {
        if (data.empty()) {
            lastError_ = "Input data is empty";
            return {};
        }

        std::vector<uint8_t> uncompressedData(data.size() * 10);
        
        z_stream zs = {0};
        zs.zalloc = Z_NULL;
        zs.zfree = Z_NULL;
        zs.opaque = Z_NULL;
        
        if (inflateInit2(&zs, 15 + 16) != Z_OK) {
            lastError_ = "Failed to initialize decompression";
            return {};
        }

        zs.next_in = const_cast<Bytef*>(data.data());
        zs.avail_in = static_cast<uInt>(data.size());
        zs.next_out = uncompressedData.data();
        zs.avail_out = static_cast<uInt>(uncompressedData.size());

        int ret = inflate(&zs, Z_FINISH);
        if (ret != Z_STREAM_END) {
            lastError_ = std::string("Decompression error: ") + 
                        (zs.msg ? zs.msg : "Unknown error");
            inflateEnd(&zs);
            return {};
        }

        inflateEnd(&zs);
        uncompressedData.resize(zs.total_out);
        return uncompressedData;
    }

    bool TTBPacker::updateResource(const std::string& exePath, const std::vector<uint8_t>& compressedData)
    {
        try {
            TTBResourceHandle handle(exePath);
            
            if (!UpdateResourceW(handle.get(),
                               L"TTB_DATA",
                               MAKEINTRESOURCEW(1),
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                               reinterpret_cast<LPVOID>(const_cast<uint8_t*>(compressedData.data())),
                               static_cast<DWORD>(compressedData.size()))) {
                DWORD error = GetLastError();
                lastError_ = "Failed to update resource. Error code: " + std::to_string(error);
                return false;
            }

            handle.commit();
            return true;
        }
        catch (const std::exception& e) {
            lastError_ = std::string("Error during resource update: ") + e.what();
            return false;
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
                                  const std::string& outputPath)
    {
        std::ofstream file(outputPath, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Failed to create output file: " + outputPath);
        }

        file.write(reinterpret_cast<const char*>(exeTemplate.data()), exeTemplate.size());
        if (!file)
        {
            throw std::runtime_error("Failed to write output file: " + outputPath);
        }
    }
} // TinaToolBox
