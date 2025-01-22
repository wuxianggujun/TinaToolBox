#include "TTBScriptEngine.hpp"
#include <iostream>
#include <filesystem>

namespace TinaToolBox {

class TTBScriptEngine::Impl {
public:
    std::shared_ptr<ExcelHandler> excelHandler;
    std::shared_ptr<ExcelScriptInterpreter> interpreter;
    std::string lastError;
    std::map<std::string, std::string> currentConfig;
    ConfigUpdateCallback configCallback;
    ProgressCallback progressCallback;
    
    Impl() {
        excelHandler = std::make_shared<ExcelHandler>();
        interpreter = std::make_shared<ExcelScriptInterpreter>(excelHandler);
    }

    void reportProgress(const std::string& message, int progress) {
        if (progressCallback) {
            progressCallback(message, progress);
        }
    }

    void updateConfig(const std::map<std::string, std::string>& config) {
        currentConfig = config;
        if (configCallback) {
            configCallback(currentConfig);
        }
    }
};

TTBScriptEngine::TTBScriptEngine() : pimpl(std::make_unique<Impl>()) {}

TTBScriptEngine::~TTBScriptEngine() = default;

TTBScriptEngine::Error TTBScriptEngine::createScript(
    const std::string& filename,
    const std::map<std::string, std::string>& config,
    const std::string& script,
    bool encrypt)
{
    try {
        pimpl->reportProgress("Generating encryption key...", 0);
        auto key = TTBFile::generateKey();

        pimpl->reportProgress("Creating TTB file...", 20);
        bool success;
        if (encrypt) {
            success = TTBFile::createEncrypted(filename, config, script, key,
                                             EncryptionFlags::AllEncrypted);
        } else {
            success = TTBFile::create(filename, config, script);
        }

        if (!success) {
            pimpl->lastError = "Failed to create TTB file";
            return Error::FILE_CREATE_ERROR;
        }

        pimpl->reportProgress("TTB file created successfully", 100);
        pimpl->updateConfig(config);
        return Error::SUCCESS;
    }
    catch (const std::exception& e) {
        pimpl->lastError = e.what();
        return Error::FILE_CREATE_ERROR;
    }
}

TTBScriptEngine::Error TTBScriptEngine::executeScript(
    const std::string& filename,
    const AESKey& key)
{
    try {
        if (!std::filesystem::exists(filename)) {
            pimpl->lastError = "File not found: " + filename;
            return Error::FILE_LOAD_ERROR;
        }

        pimpl->reportProgress("Loading TTB file...", 0);
        
        std::unique_ptr<TTBFile> ttbFile;
        if (key.empty()) {
            ttbFile = TTBFile::load(filename);
        } else {
            ttbFile = TTBFile::loadEncrypted(filename, key);
        }

        if (!ttbFile) {
            pimpl->lastError = "Failed to load TTB file";
            return Error::FILE_LOAD_ERROR;
        }

        pimpl->reportProgress("Setting up interpreter...", 30);
        const auto& config = ttbFile->getConfig();
        pimpl->interpreter->setInitialConfig(config);
        pimpl->updateConfig(config);

        pimpl->reportProgress("Executing script...", 50);
        auto scriptContent = ttbFile->getScript();
        auto result = pimpl->interpreter->executeScript(scriptContent);

        if (result != ExcelScriptInterpreter::ErrorCode::SUCCESS) {
            pimpl->lastError = pimpl->interpreter->getLastError();
            return Error::SCRIPT_EXECUTION_ERROR;
        }

        pimpl->reportProgress("Updating configuration...", 80);
        pimpl->updateConfig(pimpl->interpreter->getAllConfig());

        pimpl->reportProgress("Script executed successfully", 100);
        return Error::SUCCESS;
    }
    catch (const std::exception& e) {
        pimpl->lastError = e.what();
        return Error::SCRIPT_EXECUTION_ERROR;
    }
}

std::string TTBScriptEngine::getLastError() const {
    return pimpl->lastError;
}

void TTBScriptEngine::setConfigUpdateCallback(ConfigUpdateCallback callback) {
    pimpl->configCallback = std::move(callback);
}

void TTBScriptEngine::setProgressCallback(ProgressCallback callback) {
    pimpl->progressCallback = std::move(callback);
}

const std::map<std::string, std::string>& TTBScriptEngine::getCurrentConfig() const {
    return pimpl->currentConfig;
}

bool TTBScriptEngine::validateTTBFile(const std::string& filename) const {
    try {
        if (!std::filesystem::exists(filename)) {
            return false;
        }

        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            return false;
        }

        TTBHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));

        return header.magic == TTB_MAGIC && header.version == TTB_VERSION;
    }
    catch (...) {
        return false;
    }
}

} // namespace TinaToolBox 