#include "Document.hpp"
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    Document::Document(const QString &filePath) : filePath_(filePath), fileInfo_(filePath) {
        QString extension = fileInfo_.suffix().toLower();
        type_ = determineType(extension);

        // 检查文件是否存在和可访问
        if (!exists()) {
            setError(tr("File does not exist: %1").arg(filePath));
            return;
        }

        if (!isReadable()) {
            setError(tr("File is not readable: %1").arg(filePath));
        }

        // 文件检测通过，设置为就绪状态
        setState(State::Ready);
    }

    Document::~Document() {
    }

    Document::State Document::getState() const {
        return state_;
    }

    QString Document::stateString() const {
        switch (state_) {
            case State::Opening: return "Opening";
            case State::Ready: return "Ready";
            case State::Error: return "Error";
            default: return "Unknown";
        }
    }

    bool Document::isRead() const {
        return state_ == State::Ready;
    }

    bool Document::hasError() const {
        return state_ == State::Error;
    }

    QString Document::lastError() const {
        return lastError_;
    }

    void Document::setState(State newState) {
        if (state_ != newState) {
            state_ = newState;
            spdlog::debug("Document {} state changed: {} -> {}",
                          filePath_.toStdString(),
                          stateString().toStdString(),
                          stateString().toStdString());

            emit stateChanged(state_);
        }
    }

    void Document::setError(const QString &error) {
        lastError_ = error;
        setState(State::Error);
        emit errorOccurred(error);
        spdlog::error("Document {} error: {}", filePath_.toStdString(), error.toStdString());
    }


    QString Document::filePath() const {
        return filePath_;
    }

    QString Document::fileName() const {
        return fileInfo_.fileName();
    }

    Document::Type Document::type() const {
        return type_;
    }

    bool Document::isScript() const {
        return type_ == SCRIPT;
    }

    QString Document::typeToString() const {
        if (type_ == SCRIPT) {
            return "script";
        }
        if (type_ == TEXT) {
            return "text";
        }
        if (type_ == PDF) {
            return "pdf";
        }
        return "unknown";
    }

    bool Document::exists() const {
        return fileInfo_.exists();
    }

    qint64 Document::size() const {
        return fileInfo_.size();
    }

    QDateTime Document::lastModified() const {
        return fileInfo_.lastModified();
    }

    bool Document::isReadable() const {
        return fileInfo_.isReadable();
    }

    bool Document::isWritable() const {
        return fileInfo_.isWritable();
    }

    Document::Type Document::determineType(const QString &extension) const {
        if (extension == "ttb")
            return SCRIPT;
        if (extension == "pdf")
            return PDF;
        if (QStringList({"txt", "md", "json", "xml", "yaml", "yml"}).contains(extension)) {
            return TEXT;
        }
        return UNKNOWN;
    }
}
