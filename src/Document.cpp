#include "Document.hpp"
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    Document::Document(const QString &filePath) : filePath_(filePath), fileInfo_(filePath) {
        QString extension = fileInfo_.suffix().toLower();
        type_ = determineType(extension);
        spdlog::debug("Created document: {}, type: {}",
                      filePath.toStdString(),
                      static_cast<int>(type_));

        bool result = loadFromFile(filePath);
        if (!result) {
            spdlog::error("Failed to load document: {}", filePath.toStdString());
        }
    }

    void Document::setContent(const QString &content) {
        if (content_ != content) {
            content_ = content;
            isModified_ = true;
            emit documentModified();
        }
    }

    bool Document::save() {
        if (!isModified_) {
            return true;
        }
        if (saveToFile(filePath_)) {
            isModified_ = false;
            emit documentSaved();
            return true;
        }
        return false;
    }

    bool Document::reload() {
        if (loadFromFile(filePath_)) {
            isModified_ = false;
            emit documentModified(); // 通知内容已更改
            return true;
        }
        return false;
    }

    bool Document::saveAs(const QString &newPath) {
        if (saveToFile(newPath)) {
            QString oldPath = filePath_;
            filePath_ = newPath;
            fileInfo_ = QFileInfo(newPath);
            isModified_ = false;
            emit documentRenamed(newPath);
            emit documentSaved();
            return true;
        }
        return false;
    }

    QString Document::content() const {
        return content_;
    }

    bool Document::isModified() const {
        return isModified_;
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

    bool Document::saveToFile(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            spdlog::error("Failed to save file: {}", path.toStdString());
            return false;
        }

        QTextStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        out.setEncoding(QStringConverter::Utf8);
#else
        out.setCodec("UTF-8");
#endif
        out << content_;
        return true;
    }

    bool Document::loadFromFile(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            spdlog::error("Failed to load file: {}", path.toStdString());
            return false;
        }

        QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        in.setEncoding(QStringConverter::Utf8);
#else
        in.setCodec("UTF-8");
#endif
        content_ = in.readAll();
        return true;
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
