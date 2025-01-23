//
// Created by wuxianggujun on 2025/1/23.
//

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <memory>
#include "TTBFile.hpp"

namespace TinaToolBox
{
    class TTBPacker
    {
    public:
        // 错误码
        enum class Error {
            SUCCESS,
            NO_TTB_DATA,
            COMPRESSION_FAILED,
            DECOMPRESSION_FAILED,
            FILE_CREATE_ERROR,
            FILE_LOAD_ERROR,
            RESOURCE_UPDATE_ERROR
        };

        TTBPacker() = default;
        ~TTBPacker() = default;

        // 打包TTB文件到可执行文件
        Error packToExecutable(const std::string& templatePath,
                             const std::string& ttbPath,
                             const std::string& outputPath);

        // 从可执行文件中提取并执行TTB文件
        Error extractAndExecute(const std::string& exePath);

        // 获取最后一次错误信息
        std::string getLastError() const { return lastError_; }

        // 设置进度回调
        using ProgressCallback = std::function<void(const std::string&, int)>;
        void setProgressCallback(ProgressCallback callback) { progressCallback_ = std::move(callback); }

    private:
        // 压缩TTB数据
        std::vector<uint8_t> compressData(const std::vector<uint8_t>& data);
        
        // 解压TTB数据
        std::vector<uint8_t> decompressData(const std::vector<uint8_t>& data);
        
        // 更新资源
        bool updateResource(const std::string& exePath, const std::vector<uint8_t>& compressedData);

        // 加载最小可执行文件模板
        std::vector<uint8_t> loadMinimalExeTemplate();

        // 在PE文件中查找数据段偏移
        size_t findDataSegmentOffset(const std::vector<uint8_t>& exeTemplate);

        // 注入数据到PE文件
        void injectData(std::vector<uint8_t>& exeTemplate,
                       size_t dataOffset,
                       const std::vector<uint8_t>& ttbData);

        // 更新PE文件头
        void updatePEHeaders(std::vector<uint8_t>& exeTemplate, size_t ttbDataSize);

        // 写入可执行文件
        void writeExecutable(const std::vector<uint8_t>& exeTemplate,
                           const std::string& outputPath);

        // 报告进度
        void reportProgress(const std::string& message, int progress) {
            if (progressCallback_) {
                progressCallback_(message, progress);
            }
        }

        std::string lastError_;
        ProgressCallback progressCallback_;
    };
} // TinaToolBox
