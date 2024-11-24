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
#include "MergedTableView.hpp"
#include "TableModel.hpp"
#include "ExcelProcessor.hpp"
#include "RunButton.hpp"

class DocumentTab : public QWidget {
    Q_OBJECT
public:
    explicit DocumentTab(const QString& filePath,QWidget* parent = nullptr);

    QTextEdit* setupTextView();
    MergedTableView* setupExcelView();
    
    void moveSheetTabs(bool showAtTop);

    QString getFilePath() const {return file_path_;}

    QWidget* getToolBar() const {return toolbar_;}
    void setupToolBar(const QString& fileType);
    
private slots:
    void changeSheet(int index);
private:

    void runScript() const;
    
    QString file_path_;
    QVBoxLayout* layout_;
    QStackedWidget* stacked_widget_;
    QTabWidget* sheet_tab_;

    // 添加工具栏
    QWidget* toolbar_;
    RunButton* run_button_;

    MergedTableView* table_view_{nullptr};
    QTextEdit*  text_edit_{nullptr};
    TableModel* table_model_{nullptr};
    std::unique_ptr<ExcelProcessor> excel_processor_{nullptr};
};

inline void DocumentTab::runScript() const {
    // TODO: 实现脚本运行逻辑
    qDebug() << "Running script:" << file_path_;
}

class DocumentArea : public QWidget {
    Q_OBJECT
public:
    explicit DocumentArea(QWidget* parent = nullptr);

    QWidget* openDocument(const QString& filePath,const QString& fileType);

private slots:
    void closeTab(int index);

private:
    QVBoxLayout* layout_;
    QTabWidget* tab_widget_;
    QWidget* toolbar_; // 添加工具栏
    QMap<QString,DocumentTab*> documents_;
    
};



#endif //TINA_TOOL_BOX_DOCUMENT_AREA_HPP
