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
#include "DocumentHandler.hpp"
#include "MergedTableView.hpp"
#include "TableModel.hpp"
#include "ExcelProcessor.hpp"
#include "RunButton.hpp"
#include "DocumentTabWidget.hpp"
#include "LineNumberTextEdit.hpp"
#include "PdfViewer.hpp"
#include "SettingsPanel.hpp"


namespace TinaToolBox {
    class DocumentTab : public QWidget {
        Q_OBJECT

    public:
        explicit DocumentTab(QString filePath, QWidget *parent = nullptr);

        void moveSheetTabs(bool showAtTop);

        [[nodiscard]] QString getFilePath() const { return file_path_; }

        [[nodiscard]] QWidget *getToolBar() const { return toolbar_; }

    private:
        QString file_path_;
        QVBoxLayout *layout_;
        QStackedWidget *stacked_widget_;
        QTabWidget *sheet_tab_;

        // 添加工具栏
        QWidget *toolbar_{};
        RunButton *run_button_{};
        // Excel处理器
        std::unique_ptr<ExcelProcessor> excel_processor_{nullptr};
    };


    class DocumentArea : public QWidget {
        Q_OBJECT

    public:
        explicit DocumentArea(QWidget *parent = nullptr);

        std::shared_ptr<Document> currentDocument() const;

        bool openDocument(const QString &filePath);
        void setupConnections();
        void currentDocumentChanged(std::shared_ptr<Document> document);

        void documentOpened(std::shared_ptr<Document> document);

        void documentClosed(std::shared_ptr<Document> document);

        // 简化的公共接口
        bool openFile(const QString &filePath);

        void closeFile(int index);

        // 获取当前文档
        [[nodiscard]] QString getCurrentFilePath() const;

        [[nodiscard]] QWidget *getCurrentDocument() const;

        void closeAllDocuments();

        void showSettingsPanel();

        [[nodiscard]] DocumentTabWidget *getTabWidget() const { return tab_widget_; }

    signals:
        void fileOpened(const QString &filePath);

        void fileClosed(const QString &filePath);

        void currentFileChanged(const QString &filePath);

        void error(const QString &message);

    private:
        struct DocumentInfo {
            QWidget *view;
            std::shared_ptr<IDocumentHandler> handler;
        };

        bool canCloseDocument(int index) const;

        QWidget *createDocumentView(const QString &filePath);

        SettingsPanel *settingsPanel_{nullptr};

        QVBoxLayout *layout_;
        DocumentTabWidget *tab_widget_;
        QWidget *toolbar_{}; // 添加工具栏
        QMap<QString, DocumentInfo> openDocuments_;

        DocumentTabWidget *tabWidget_;
        QMap<QString, std::shared_ptr<Document> > documents_;
    };
}

#endif //TINA_TOOL_BOX_DOCUMENT_AREA_HPP
