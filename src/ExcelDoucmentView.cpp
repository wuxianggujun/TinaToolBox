#include "ExcelDoucmentView.hpp"
#include <spdlog/spdlog.h>

namespace TinaToolBox {
    ExcelDocumentView::ExcelDocumentView(const std::shared_ptr<Document> &document):document_(document),tableView_(new MergedTableView()),model_(new TableModel(tableView_)) {
        tableView_->setModel(model_);

        loadExcelFile();
        
    }

    void ExcelDocumentView::updateContent() {
        loadExcelFile();
    }

    bool ExcelDocumentView::saveContent() {
        return true;
    }

    QWidget * ExcelDocumentView::widget() {
        return tableView_;
    }

    void ExcelDocumentView::loadExcelFile() {
        try {
            xlnt::workbook wb;
            wb.load(document_->filePath().toStdString());

            auto ws = wb.active_sheet();
            processWorksheet(ws);
        }catch (const std::exception &e) {
            spdlog::error("Failed to load Excel file: {}", e.what());
        }
    }

    void ExcelDocumentView::processWorksheet(const xlnt::worksheet &ws) {
        QVector<QVector<QVariant>> data;
        QVector<QPair<QPair<int, int>, QPair<int, int>>> mergedCells;

        // 获取工作表的有效范围
        auto range = ws.calculate_dimension();
    
        // 读取单元格数据
        for (int row = range.top_left().row(); row <= range.bottom_right().row(); ++row) {
            QVector<QVariant> rowData;
            for (auto col = range.top_left().column(); col <= range.bottom_right().column(); ++col) {
                auto cell = ws.cell(xlnt::cell_reference(col, row));
                rowData.append(getCellValue(cell));
            }
            data.append(rowData);
        }

        // 处理合并单元格
        for (const auto& range : ws.merged_ranges()) {
            // 使用column_number()获取列号
            QPair<int, int> startPos(
                range.top_left().row() - 1, 
                static_cast<int>(range.top_left().column().index) - 1
            );
            QPair<int, int> endPos(
                range.bottom_right().row() - 1, 
                static_cast<int>(range.bottom_right().column().index) - 1
            );
            mergedCells.append(qMakePair(startPos, endPos));
        }

        // 更新模型数据
        model_->setData(data, mergedCells);
        tableView_->setMergedCells(mergedCells);
    }

    QVariant ExcelDocumentView::getCellValue(const xlnt::cell& cell) {
        if (cell.has_value()) {
            switch (cell.data_type()) {
                case xlnt::cell_type::number:
                    return QVariant(cell.value<double>());
                case xlnt::cell_type::boolean:
                    return QVariant(cell.value<bool>());
                case xlnt::cell_type::date:
                    return QVariant(QString::fromStdString(cell.to_string()));
                default:
                    return QVariant(QString::fromStdString(cell.to_string()));
            }
        }
        return QVariant();
    }

    
}
