//
// Created by wuxianggujun on 2024/11/24.
//

#ifndef TINA_TOOL_BOX_DOCUMENT_AREA_HPP
#define TINA_TOOL_BOX_DOCUMENT_AREA_HPP

#include <QWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QTableView>
#include <QMap>
#include <QStackedWidget>
#include <memory>
#include "Document.hpp"
#include "TableModel.hpp"
#include "ExcelProcessor.hpp"
#include "RunButton.hpp"
#include "DocumentTabWidget.hpp"
#include "DocumentView.hpp"
#include "LineNumberTextEdit.hpp"
#include "PdfViewer.hpp"
#include "SettingsPanel.hpp"


namespace TinaToolBox {
    
    class DocumentArea : public QWidget {
        Q_OBJECT

    public:
        explicit DocumentArea(QWidget *parent = nullptr);
        ~DocumentArea() override;
        
    public slots:
        void onDocumentOpened(std::shared_ptr<Document> document);
        void onDocumentClosed(std::shared_ptr<Document> document);
        void onCurrentDocumentChanged(std::shared_ptr<Document> document);

    private:
        
        void setupConnections();

        DocumentView* createDocumentView(const std::shared_ptr<Document>& document);
        void cleanupDocumentView(const QString& filePath);
        
        DocumentTabWidget *tabWidget_;
        QMap<QString,DocumentView*> documentViews_;
    };
}

#endif //TINA_TOOL_BOX_DOCUMENT_AREA_HPP
