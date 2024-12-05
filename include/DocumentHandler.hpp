#ifndef TINA_TOOL_BOX_DOCUMENT_HANDLER_HPP
#define TINA_TOOL_BOX_DOCUMENT_HANDLER_HPP

#include <QString>
#include <QWidget>
#include <memory>

// 文档处理器接口
class IDocumentHandler {
public:
    virtual ~IDocumentHandler() = default;

    virtual bool canHandle(const QString &fileType) const = 0;

    virtual QWidget *createView(QWidget *parent) = 0;

    virtual bool loadDocument(QWidget *view, const QString &filePath) = 0;
    virtual QString getFileTypeName() const = 0;
    virtual void cleanup(QWidget* view) = 0;
};

class PdfDocumentHandler : public IDocumentHandler {
public:
    bool canHandle(const QString &fileType) const override {
        return fileType.toLower() == "pdf";
    }

    QWidget *createView(QWidget *parent) override;

    bool loadDocument(QWidget *view, const QString &filePath) override;

    QString getFileTypeName() const override;
    void cleanup(QWidget *view) override;
};


// 文本文档处理器
class TextDocumentHandler : public IDocumentHandler {
public:
    bool canHandle(const QString& fileType) const override {
        return QStringList{"txt", "md", "py", "json", "xml", "yaml", "yml"}.contains(fileType.toLower());
    }
    
    QWidget* createView(QWidget* parent) override;
    bool loadDocument(QWidget* view, const QString& filePath) override;

    QString getFileTypeName() const override;
    void cleanup(QWidget *view) override;
};

class ScriptDocumentHandler : public IDocumentHandler {
public:
    bool canHandle(const QString &fileType) const override;

    QString getFileTypeName() const override;

    QWidget * createView(QWidget *parent) override;

    bool loadDocument(QWidget *view, const QString &filePath) override;

    void cleanup(QWidget *view) override;
};

// 文档处理器工厂
class DocumentHandlerFactory {
public:
    static std::shared_ptr<IDocumentHandler> createHandler(const QString& fileType);
    static void registerHandler(std::shared_ptr<IDocumentHandler> handler);
    static QStringList getSupportedFileTypes();
private:
    static QList<std::shared_ptr<IDocumentHandler>> handlers_;
};

#endif //TINA_TOOL_BOX_DOCUMENT_HANDLER_HPP
