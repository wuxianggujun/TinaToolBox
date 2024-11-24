//
// Created by wuxianggujun on 2024/11/24.
//

#include "MergedTableView.hpp"

#include "spdlog/spdlog.h"

MergedTableView::MergedTableView(QWidget *parent):QTableView(parent) {
    setupUI();
}

void MergedTableView::setMergedCells(const QVector<QPair<QPair<int, int>, QPair<int, int>>> &mergedCells) {
        if (mergedCells.isEmpty()) {
            return;
        }

        // 处理每个合并单元格
        for (const auto& cell : mergedCells) {
            try {
                const auto& startPos = cell.first;
                const auto& endPos = cell.second;
            
                int startRow = startPos.first;
                int startCol = startPos.second;
                int endRow = endPos.first;
                int endCol = endPos.second;
            
                // 计算跨度
                int rowSpan = endRow - startRow + 1;
                int colSpan = endCol - startCol + 1;
            
                // 只在跨度大于1时设置合并单元格
                if (rowSpan > 1 || colSpan > 1) {
                    setSpan(startRow, startCol, rowSpan, colSpan);
                }
            
            } catch (const std::exception& e) {
                spdlog::error("Error setting merged cell: {}", e.what());
            }
        }
    
        // 更新视图
        viewport()->update();
}

void MergedTableView::setupUI() {
    // 显示网格线
    setShowGrid(true);

    setSelectionMode(QTableView::SingleSelection);

    setSelectionBehavior(QTableView::SelectItems);

    // 设置交替行颜色
    setAlternatingRowColors(true);
}


