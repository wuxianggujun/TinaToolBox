//
// Created by wuxianggujun on 2024/11/24.
//
#pragma once

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

        DocumentView* getCurrentDocumentView() const;
    public slots:
        void onDocumentOpened(std::shared_ptr<Document> document);

        void onDocumentClosed(std::shared_ptr<Document> document);

        void onDocumentStateChanged(std::shared_ptr<Document> document);

        void onCurrentDocumentChanged(std::shared_ptr<Document> document);

        void onDocumentError(std::shared_ptr<Document> document, const QString &error);

    private:
        void setupConnections();

        DocumentView *createDocumentView(const std::shared_ptr<Document> &document);

        void cleanupDocumentView(const QString &filePath);

        void updateTabState(DocumentView *view, const std::shared_ptr<Document> &document);

    private:
        DocumentTabWidget *tabWidget_;
        QMap<QString, DocumentView *> documentViews_;
    };
}
