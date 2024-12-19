//
// Created by wuxianggujun on 2024/12/19.
//
#include <QVBoxLayout>
#include "BlockProgrammingDocumentView.hpp"

namespace TinaToolBox {
    BlockProgrammingDocumentView::BlockProgrammingDocumentView(std::shared_ptr<Document> document, QWidget *parent)
       : QObject(parent), document_(std::move(document)) {
        blockProgrammingView_ = new BlockProgrammingView(parent);
        
        // 在这里添加一些示例积木块
        blockProgrammingView_->addBlock(Block::Type::IF, QPointF(50, 50));
        blockProgrammingView_->addBlock(Block::Type::LOOP, QPointF(200, 50));
        blockProgrammingView_->addBlock(Block::Type::PRINT, QPointF(350, 50));
    }

    BlockProgrammingDocumentView::~BlockProgrammingDocumentView() {
        // 析构函数中不需要手动 delete blockProgrammingView_，因为它是 QWidget 的子对象，会在父对象析构时自动 delete
    }

    void BlockProgrammingDocumentView::updateContent() {
        // 在这里实现更新积木块内容的逻辑，例如从 document_ 中加载数据
    }

    bool BlockProgrammingDocumentView::saveContent() {
        // 在这里实现保存积木块内容的逻辑，例如保存到 document_ 中
        return true;
    }

    QWidget *BlockProgrammingDocumentView::widget() {
        return blockProgrammingView_;
    }
} // TinaToolBox