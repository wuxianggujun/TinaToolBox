#include "PythonScriptManager.hpp"
#include <stdexcept>
#include <iostream>

PythonScriptManager::PythonScriptManager() : isInitialized_(false) {
    scriptDirectory_ = std::filesystem::current_path() / "scripts";
    if (!std::filesystem::exists(scriptDirectory_)) {
        std::filesystem::create_directory(scriptDirectory_);
    }
}

PythonScriptManager::~PythonScriptManager() {
    if (isInitialized_) {
        Py_Finalize();
    }
}

bool PythonScriptManager::initialize(const std::string &venvSitePackagesPath) {
    if (isInitialized_) {
        return true;
    }
    
    PyStatus status;
    
    PyConfig config;
    PyConfig_InitPythonConfig(&config);
    config.isolated = 1;
    config.use_environment = 0;
    config.site_import = 1;
    config.user_site_directory = 1;
    config.verbose = 1;
    
    // 设置命令行参数
    int argc = 0;
    char **argv = nullptr;
    status = PyConfig_SetBytesArgv(&config, argc, argv);
    if (PyStatus_Exception(status)) {
        PyConfig_Clear(&config);
        return false;
    }

    // 设置虚拟环境的路径
    if (!venvSitePackagesPath.empty()) {
        wchar_t* venvSitePackagesPathW = Py_DecodeLocale(venvSitePackagesPath.c_str(), nullptr);
        if (venvSitePackagesPathW == nullptr) {
            PyErr_SetString(PyExc_MemoryError, "Failed to decode venv site packages path");
            PyConfig_Clear(&config);
            return false;
        }
        status = PyConfig_SetString(&config, &config.base_prefix, venvSitePackagesPathW);
        PyMem_RawFree(venvSitePackagesPathW);
        if (PyStatus_Exception(status)) {
            PyConfig_Clear(&config);
            return false;
        }
    }

    // 初始化Python
    status = Py_InitializeFromConfig(&config);
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);
        return false;
    }
    PyConfig_Clear(&config);
    
    isInitialized_ = true;

    return true;
}

bool PythonScriptManager::addScript(const std::string &scriptPath, const std::string &depsPath) {
    if (!isInitialized_) {
        if (!initialize("")) {
            return false;
        }
    }

    std::filesystem::path fullPath(scriptPath);
    if (!std::filesystem::exists(fullPath)) {
        spdlog::error("Script file does not exist: {}", scriptPath);
        return false;
    }

    std::filesystem::path targetPath = scriptDirectory_ / fullPath.filename();
    if (fullPath != targetPath) {
        std::filesystem::copy_file(fullPath, targetPath,
                                   std::filesystem::copy_options::overwrite_existing);
    }

    if (!depsPath.empty()) {
        std::string command = "pip install -r " + depsPath;
        int result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Failed to install dependencies" << std::endl;
            return false;
        }
    }

    return true;
}


std::filesystem::path PythonScriptManager::getScriptDirectory() const {
    return scriptDirectory_;
}

void PythonScriptManager::clearPyConfig(PyConfig *config, PyStatus *status) {
    PyConfig_Clear(config);
    if (PyStatus_IsExit(*status)) {
        Py_ExitStatusException(*status);
    }
}

std::unordered_map<std::string, std::string> PythonScriptManager::readIni(const std::string &fileName) {
    std::unordered_map<std::string, std::string> settings;
    CSimpleIniA ini;
    ini.SetUnicode();

    ini.LoadFile(fileName.c_str());

    ini.SetValue(nullptr, "param1", "default_value1");
    ini.SetValue(nullptr, "param2", "default_value2");
    ini.SetValue(nullptr, "log_path", "default_output.log");

    settings["param1"] = ini.GetValue(nullptr, "param1", "default_value1");
    settings["param2"] = ini.GetValue(nullptr, "param2", "default_value2");
    settings["log_path"] = ini.GetValue(nullptr, "log_path", "default_output.log");

    return settings;
}


static PyObject *PySpdlogWriter_write(PyObject *self, PyObject *args) {
    const char *data;
    Py_ssize_t len;
    if (!PyArg_ParseTuple(args, "s#:write", &data, &len)) {
        return nullptr;
    }
    PyObject *levelObj = PyObject_GetAttrString(self, "level");
    if (!levelObj) {
        PyErr_SetString(PyExc_AttributeError, "CustomStream object has no 'level' attribute");
        return nullptr;
    }
    long level = PyLong_AsLong(levelObj);
    Py_DECREF(levelObj);
    if (level == -1 && PyErr_Occurred()) {
        return nullptr;
    }
    static auto logger = spdlog::basic_logger_mt("stdout_logger", "script.log");
    logger->log(static_cast<spdlog::level::level_enum>(level), "{}", std::string(data, len));
    Py_RETURN_NONE;
}

static PyMethodDef PySpdlogWriter_methods[] = {
    {"write", PySpdlogWriter_write, METH_VARARGS, "Write data to spdlog."},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject PySpdlogWriterType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "PythonScriptManager.PySpdlogWriter", /* tp_name */
    sizeof(PyObject), /* tp_basicsize */
    0, /* tp_itemsize */
    0, /* tp_dealloc */
    0, /* tp_vectorcall_offset */
    0, /* tp_getattr */
    0, /* tp_setattr */
    0, /* tp_reserved */
    0, /* tp_repr */
    0, /* tp_as_number */
    0, /* tp_as_sequence */
    0, /* tp_as_mapping */
    0, /* tp_hash */
    0, /* tp_call */
    0, /* tp_str */
    0, /* tp_getattro */
    0, /* tp_setattro */
    0, /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT, /* tp_flags */
    "PySpdlogWriter objects", /* tp_doc */
    0, /* tp_traverse */
    0, /* tp_clear */
    0, /* tp_richcompare */
    0, /* tp_weaklistoffset */
    0, /* tp_iter */
    0, /* tp_iternext */
    PySpdlogWriter_methods, /* tp_methods */
    0, /* tp_members */
    0, /* tp_getset */
    0, /* tp_base */
    0, /* tp_dict */
    0, /* tp_descr_get */
    0, /* tp_descr_set */
    0, /* tp_dictoffset */
    0, /* tp_init */
    0, /* tp_alloc */
    0, /* tp_new */
};


PyObject *PythonScriptManager::createCustomStream(spdlog::level::level_enum level) {
    if (!PySpdlogWriterType.tp_new) {
        PySpdlogWriterType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&PySpdlogWriterType) < 0) {
            return nullptr;
        }
    }
    PyObject *customStream = PyType_GenericNew(&PySpdlogWriterType, NULL, NULL);
    if (customStream) {
        PyObject *levelObj = PyLong_FromLong(static_cast<long>(level));
        if (!levelObj) {
            Py_DECREF(customStream);
            return nullptr;
        }
        if (PyObject_SetAttrString(customStream, "level", levelObj) < 0) {
            Py_DECREF(customStream);
            Py_DECREF(levelObj);
            return nullptr;
        }
    }
    return customStream;
}


bool PythonScriptManager::runScript(const std::string &scriptPath, const std::string &configPath) {
    if (!isInitialized_ && !initialize("")) {
        return false;
    }

    std::unordered_map<std::string, std::string> params = readIni(configPath);
    std::string log_path = params["log_path"];
    std::string param1 = params["param1"];
    std::string param2 = params["param2"];

    auto logger = spdlog::basic_logger_mt("stdout_logger", log_path);
    logger->set_pattern("[%H:%M:%S %z] [%n] [%l] %v");

    PyObject *custom_stdout = createCustomStream(spdlog::level::info);
    PyObject *custom_stderr = createCustomStream(spdlog::level::err);

    PyObject *sys_module = PyImport_ImportModule("sys");
    if (!sys_module) {
        PyErr_Print();
        return false;
    }

    PyObject_SetAttrString(sys_module, "stdout", custom_stdout);
    PyObject_SetAttrString(sys_module, "stderr", custom_stderr);

    PyObject *sys_argv = PySys_GetObject("argv");

    PyList_Append(sys_argv, PyUnicode_FromString(scriptPath.c_str()));
    PyList_Append(sys_argv, PyUnicode_FromString(param1.c_str()));
    PyList_Append(sys_argv, PyUnicode_FromString(param2.c_str()));

    FILE *file = fopen(scriptPath.c_str(), "r");
    if (!file) {
        std::cerr << "Failed to open script: " << scriptPath << std::endl;
        return false;
    }

    PyObject *module = PyImport_AddModule("__main__");
    if (!module) {
        fclose(file);
        std::cerr << "Failed to create main module" << std::endl;
        return false;
    }
    PyObject *dict = PyModule_GetDict(module);
    if (PyRun_File(file, scriptPath.c_str(), Py_file_input, dict, dict) == NULL) {
        PyErr_Print();
        fclose(file);
        return false;
    }
    fclose(file);

    return true;
}

bool PythonScriptManager::activateVirtualEnvironment(const std::string &sitePackagesPath) {
    PyObject *sys_module = PyImport_ImportModule("sys");
    if (!sys_module) {
        PyErr_Print();
        return false;
    }

    PyObject *sys_path = PyObject_GetAttrString(sys_module, "path");
    if (!sys_path) {
        PyErr_Print();
        Py_DECREF(sys_module);
        return false;
    }

    PyObject *site_packages = PyUnicode_FromString(sitePackagesPath.c_str());
    if (!site_packages) {
        PyErr_Print();
        Py_DECREF(sys_path);
        Py_DECREF(sys_module);
        return false;
    }

    int inserted = PyList_Insert(sys_path, 0, site_packages);
    if (inserted != 0) {
        PyErr_Print();
        Py_DECREF(sys_path);
        Py_DECREF(sys_module);
        Py_DECREF(site_packages);
        return false;
    }

    Py_DECREF(site_packages);
    Py_DECREF(sys_path);
    Py_DECREF(sys_module);
    return true;
}
