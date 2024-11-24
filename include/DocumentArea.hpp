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

#include "TableModel.hpp"
#include "ExcelProcessor.hpp"
#include "MergedTableView.hpp"
class DocumentTab : public QWidget {
    Q_OBJECT
public:
    explicit DocumentTab(const QString& filePath,QWidget* parent = nullptr);

    QTextEdit* setupTextView();
    MergedTableView* setupExcelView();
    void moveSheetTabs(bool showAtTop);
private slots:
    void changeSheet(int index);
private:
    QString file_path_;
    QVBoxLayout* layout_;
    QStackedWidget* stacked_widget_;
    QTabWidget* sheet_tab_;

    MergedTableView* table_view_{nullptr};
    QTextEdit*  text_edit_{nullptr};
    TableModel* table_model_{nullptr};
    std::unique_ptr<ExcelProcessor> excel_processor_{nullptr};
};

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
    QMap<QString,DocumentTab*> documents_;
    
};



#endif //TINA_TOOL_BOX_DOCUMENT_AREA_HPP
