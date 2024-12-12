#include "ExcelDoucmentView.hpp"

#include <QApplication>
#include <spdlog/spdlog.h>

#include "LoadingProgressDialog.hpp"


namespace TinaToolBox {
    ExcelDocumentView::ExcelDocumentView(const std::shared_ptr<Document> &document): document_(document),
        tableView_(new MergedTableView()), model_(new TableModel(tableView_)) {
        tableView_->setModel(model_);

        loadExcelFile();
    }

    void ExcelDocumentView::updateContent() {
        loadExcelFile();
    }

    bool ExcelDocumentView::saveContent() {
        return true;
    }

    QWidget *ExcelDocumentView::widget() {
        return tableView_;
    }

    void ExcelDocumentView::loadExcelFile() {
        auto* progressDialog = LoadingProgressDialog::getInstance();
        try {

            QString fileName = QFileInfo(document_->filePath()).fileName();
            progressDialog->startProgress(QString("Loading %1").arg(fileName));

            QXlsx::Document xlsx(document_->filePath());
            progressDialog->updateProgress(20, "Opening file...");
            if (!xlsx.load()) {
                progressDialog->finishProgress();
                throw std::runtime_error("Failed to load Excel file");
            }
            progressDialog->updateProgress(40, "Reading data...");
            processWorksheet(xlsx);
            progressDialog->finishProgress();
        } catch (const std::exception &e) {
            progressDialog->finishProgress();
            document_->setState(Document::State::Error);
            spdlog::error("Failed to load Excel file: {}", e.what());
        }
    }

    void ExcelDocumentView::processWorksheet(const QXlsx::Document &xlsx) {
        auto* progressDialog = LoadingProgressDialog::getInstance();
    
        if (!xlsx.currentWorksheet()) return;

        QXlsx::CellRange range = xlsx.dimension();
        if (!range.isValid()) return;

        int totalRows = range.lastRow() - range.firstRow() + 1;
        int currentRow = 0;

        QVector<QVector<QVariant>> data;
        data.reserve(totalRows);

        // 读取数据
        for (int row = range.firstRow(); row <= range.lastRow(); ++row) {
            QVector<QVariant> rowData;
            rowData.reserve(range.lastColumn() - range.firstColumn() + 1);
        
            for (int col = range.firstColumn(); col <= range.lastColumn(); ++col) {
                QVariant value = xlsx.cellAt(row, col) ? xlsx.cellAt(row, col)->value() : QVariant();
                rowData.append(value);
            }
            data.append(rowData);
        
            // 更新进度
            currentRow++;
            int progress = 40 + (currentRow * 50) / totalRows;
            QString statusText = QString("Processing row %1 of %2")
                                   .arg(currentRow)
                                   .arg(totalRows);
            progressDialog->updateProgress(progress, statusText);
        
            // 处理事件，保持UI响应
            QApplication::processEvents();
        }

        // 处理合并单元格
        progressDialog->updateProgress(90, "Processing merged cells...");
        QList<QXlsx::CellRange> mergedRanges = xlsx.currentWorksheet()->mergedCells();
        QVector<QPair<QPair<int, int>, QPair<int, int>>> mergedCells;
    
        for (const QXlsx::CellRange& cell_range : mergedRanges) {
            mergedCells.append(qMakePair(
                QPair<int, int>(cell_range.firstRow() - 1, cell_range.firstColumn() - 1),
                QPair<int, int>(cell_range.lastRow() - 1, cell_range.lastColumn() - 1)
            ));
        }

        progressDialog->updateProgress(95, "Updating view...");
        model_->setData(data, mergedCells);
        tableView_->setMergedCells(mergedCells);
    }

    QVariant ExcelDocumentView::getCellValue(const QXlsx::Cell *cell) {
        if (!cell) return QVariant();
        return cell->value();
    }
}
