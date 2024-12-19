//
// Created by wuxianggujun on 2024/12/19.
//

#include "BlockProgrammingView.hpp"

namespace TinaToolBox {
    BlockProgrammingView::BlockProgrammingView(QWidget *parent):QGraphicsView(parent) {
        scene_ = new QGraphicsScene(this);
        setScene(scene_);

        scene_->setBackgroundBrush(QBrush(QColor(220, 220, 220)));
    }

    BlockProgrammingView::~BlockProgrammingView() {
        for (const BlockItem* item: blockItems_) {
            delete item;
        }
        blockItems_.clear();

        for (const Block* block: blocks_) {
            delete block;
        }
        blocks_.clear();
        
    }

    void BlockProgrammingView::addBlock(Block::Type type, const QPointF &pos) {
        // 创建 Block 对象
        Block *block = new Block(type);
        // 根据需要设置 Block 的属性，例如添加参数、连接点等
        // 示例：为 IF 类型的 Block 添加一个输入连接点和一个输出连接点
        if (type == Block::Type::IF) {
            block->addConnector(Connector::Type::INPUT, Connector::DataType::BOOLEAN);
            block->addConnector(Connector::Type::OUTPUT, Connector::DataType::BOOLEAN);
        }

        // 创建 BlockItem 对象
        BlockItem *item = new BlockItem(block);

        // 将 BlockItem 添加到场景中
        scene_->addItem(item);
        item->setPos(pos);
        // 将 Block 和 BlockItem 添加到列表中
        blocks_.append(block);
        blockItems_.append(item);
    }
} // TinaToolBox