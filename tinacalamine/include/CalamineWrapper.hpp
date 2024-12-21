#ifndef CALAMINEWRAPPER_H
#define CALAMINEWRAPPER_H

#include <memory>

class CalamineWrapper {
public:
    CalamineWrapper();
    ~CalamineWrapper();
    int read_a_value() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// 直接在头文件中定义 new_calamine
inline std::unique_ptr<CalamineWrapper> new_calamine() {
    return std::make_unique<CalamineWrapper>();
}

#endif // CALAMINEWRAPPER_H