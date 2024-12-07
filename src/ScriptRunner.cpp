#include "ScriptRunner.hpp"
#include "Document.hpp"
#include <QFileInfo>
#include <QTimer>
#include <spdlog/spdlog.h>


namespace TinaToolBox {

    ScriptRunner::ScriptRunner(QObject *parent):QObject(parent),isRunning_(false) {
    }
    
    bool ScriptRunner::canRun(const std::shared_ptr<Document> &document) const {
        if (!document) {
            return false;
        }
        return document->isScript();
    }

    void ScriptRunner::run(const std::shared_ptr<Document>& document) {
        if (!canRun(document)) {
            emit error(tr("无法运行非脚本文件"));
            return;
        }
        if (isRunning_) {
            emit error(tr("脚本已在运行中"));
            return;
        }

        try {
            QString filePath = document->filePath();

            isRunning_ = true;
            emit started();

              
            // TODO: 在这里添加实际的脚本执行逻辑
            spdlog::info("开始运行脚本: {}", filePath.toStdString());

            // 这里是示例，实际应该异步执行
            QTimer::singleShot(100, this, [this]() {
                isRunning_ = false;
                emit finished();
            });
        }catch (const std::exception &e) {
            isRunning_ = false;
            emit error(QString("运行脚本时发生错误: %1").arg(e.what()));
        }
        
    }

    void ScriptRunner::stop() {
        if (!isRunning_) {
            return;
        }

        try {
            // TODO: 实现脚本停止逻辑
            spdlog::info("停止脚本执行");
            isRunning_ = false;
            emit finished();
        }
        catch (const std::exception& e) {
            emit error(tr("停止脚本时发生错误: %1").arg(e.what()));
        }
    }

    bool ScriptRunner::isRunning() const {
        return isRunning_;
    }
}
