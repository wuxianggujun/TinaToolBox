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

        // 文档状态
        enum class State {
            Opening, // 打开中
            Ready, // 准备就绪
            Saving, // 保存中
            Closing, // 关闭中
            Error // 错误
        };

        Q_ENUM(State); // 使状态可以用于QT的源对象系统

    public:
        explicit Document(const QString &filePath);

        ~Document() override;

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

        State getState() const;

        QString stateString() const;

        bool isRead() const;

        bool hasError() const;

        QString lastError() const;

    signals:
        void stateChanged(State newState);

        void errorOccurred(const QString &error);

    protected:
        void setState(State newState);

        void setError(const QString &error);

    private:
        State state_;
        QString lastError_;
        QString filePath_;
        QFileInfo fileInfo_;
        Type type_;

        [[nodiscard]] Type determineType(const QString &extension) const;
    };
}
