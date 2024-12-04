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

    // 简化的公共接口
    bool openFile(const QString &filePath);

    void closeFile(int index);

    // 获取当前文档
    [[nodiscard]] QString currentFilePath() const;

    [[nodiscard]] QWidget *currentView() const;

    void closeAllDocuments();

    [[nodiscard]] DocumentTabWidget* getTabWidget() const { return tab_widget_; }
    
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

    bool canCloseDocument(int index) const;
    
    QWidget *createDocumentView(const QString &filePath);
    
    QVBoxLayout *layout_;
    DocumentTabWidget *tab_widget_;
    QWidget *toolbar_{}; // 添加工具栏
    QMap<QString, DocumentInfo > openDocuments_;
};


#endif //TINA_TOOL_BOX_DOCUMENT_AREA_HPP
