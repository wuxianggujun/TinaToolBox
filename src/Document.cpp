#include "Document.hpp"
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    Document::Document(const QString &filePath) : filePath_(filePath), fileInfo_(filePath) {
        QString extension = fileInfo_.suffix().toLower();
        type_ = determineType(extension);
        spdlog::debug("Created document: {}, type: {}",
                      filePath.toStdString(),
                      static_cast<int>(type_));
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
