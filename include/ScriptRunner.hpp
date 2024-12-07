#pragma once

#include <QObject>
#include <memory>

namespace TinaToolBox {

    class Document;
    
    class ScriptRunner : public QObject {
        Q_OBJECT
    public:
        explicit ScriptRunner(QObject* parent = nullptr);

        bool canRun(const std::shared_ptr<Document> &document) const;
        void run(const std::shared_ptr<Document>& document);
        void stop();
        bool isRunning() const;

        signals:
        void started();
        void finished();
        void error(const QString& message);

    private:
        bool isRunning_;
    };

    
}
