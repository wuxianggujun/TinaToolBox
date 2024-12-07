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

        void setContent(const QString &content);

        bool save();

        bool reload();

        bool saveAs(const QString &newPath);

        [[nodiscard]] QString content() const;

        [[nodiscard]] bool isModified() const;

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

    signals:
        void documentModified();

        void documentSaved();

        void documentRenamed(const QString &newFilePath);

    private:
        QString filePath_;
        QFileInfo fileInfo_;
        Type type_;
        QString content_;
        bool isModified_ = false;

        bool saveToFile(const QString &path);

        bool loadFromFile(const QString &path);

        [[nodiscard]] Type determineType(const QString &extension) const;
    };
}
