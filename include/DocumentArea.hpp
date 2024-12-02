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

#include "DocumentHandler.hpp"
#include "MergedTableView.hpp"
#include "TableModel.hpp"
#include "ExcelProcessor.hpp"
#include "RunButton.hpp"
#include "DocumentTabWidget.hpp"
#include "LineNumberTextEdit.hpp"
#include "PdfViewer.hpp"

class DocumentTab : public QWidget {
    Q_OBJECT

public:
    explicit DocumentTab(QString filePath, QWidget *parent = nullptr);

    QPlainTextEdit *setupTextView();

    MergedTableView *setupExcelView();

    PdfViewer *setupPdfView();

    PdfViewer *getPdfViewer() const { return pdf_view_; }

    void moveSheetTabs(bool showAtTop);

    QString getFilePath() const { return file_path_; }

    QWidget *getToolBar() const { return toolbar_; }

private slots:
    void changeSheet(int index);

private:
    void runScript() const;

    QString file_path_;
    QVBoxLayout *layout_;
    QStackedWidget *stacked_widget_;
    QTabWidget *sheet_tab_;

    // 添加工具栏
    QWidget *toolbar_{};
    RunButton *run_button_{};

    // 视图组件
    MergedTableView *table_view_{nullptr};
    LineNumberTextEdit *text_edit_{nullptr};
    PdfViewer *pdf_view_{nullptr};
    TableModel *table_model_{nullptr};

    // Excel处理器
    std::unique_ptr<ExcelProcessor> excel_processor_{nullptr};
};

inline void DocumentTab::runScript() const {
    // TODO: 实现脚本运行逻辑
    qDebug() << "Running script:" << file_path_;
}

class DocumentArea : public QWidget {
    Q_OBJECT

public:
    explicit DocumentArea(QWidget *parent = nullptr);

    // 简化的公共接口
    bool openFile(const QString &filePath);

    void closeFile(int index);

    // 获取当前文档
    QString currentFilePath() const;

    QWidget *currentView() const;

signals:
    void fileOpened(const QString &filePath);

    void fileClosed(const QString &filePath);

    void currentFileChanged(const QString &filePath);

    void error(const QString &message);

private:

    struct DocumentInfo {
        QWidget* view;
        std::shared_ptr<IDocumentHandler> handler;
    };
    
    QWidget *createDocumentView(const QString &filePath);
    
    QVBoxLayout *layout_;
    DocumentTabWidget *tab_widget_;
    QWidget *toolbar_{}; // 添加工具栏
    QMap<QString, DocumentInfo > openDocuments_;
};


#endif //TINA_TOOL_BOX_DOCUMENT_AREA_HPP
