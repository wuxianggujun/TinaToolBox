#pragma once
#include <QFileInfo>
#include <QObject>
#include <QString>

namespace TinaToolBox {
    class Document : public QObject {
        Q_OBJECT

    public:
        enum Type {
            UNKNOWN,
            SCRIPT,
            TEXT,
            PDF
        };

        explicit Document(const QString &filePath);
        
        [[nodiscard]] QString filePath() const;

        [[nodiscard]] QString fileName() const;

        [[nodiscard]] Type type() const;

        [[nodiscard]] bool isScript() const;

        [[nodiscard]] QString typeToString() const;

        [[nodiscard]] bool exists() const;

        [[nodiscard]] qint64 size() const;

        [[nodiscard]] QDateTime lastModified() const;


        [[nodiscard]] bool isReadable() const;

        [[nodiscard]] bool isWritable() const;
    
    private:
        QString filePath_;
        QFileInfo fileInfo_;
        Type type_;
        
        [[nodiscard]] Type determineType(const QString &extension) const;
    };
}
