#ifndef TINA_TOOL_BOX_PYTHON_SCRIPT_MANAGER_HPP
#define TINA_TOOL_BOX_PYTHON_SCRIPT_MANAGER_HPP

#include <io.h>
#include <fcntl.h>
#include "Python_wrapper.hpp"
#include <pyconfig.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <SimpleIni.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>


class PythonScriptManager {
public:
    PythonScriptManager();

    ~PythonScriptManager();

    bool initialize(const std::string& venvSitePackagesPath);

    bool addScript(const std::string &scriptPath, const std::string &depsPath);

    bool runScript(const std::string &scriptPath, const std::string &configPath);

    // Get the script directory
    std::filesystem::path getScriptDirectory() const;

private:
    void clearPyConfig(PyConfig* config,PyStatus* status);
    
    std::unordered_map<std::string, std::string> readIni(const std::string &fileName);
    
    PyObject* createCustomStream(spdlog::level::level_enum level);

    bool activateVirtualEnvironment(const std::string &scriptPath);
    
    std::filesystem::path scriptDirectory_;
    bool isInitialized_;
};

#endif  