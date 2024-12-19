//
// Created by wuxianggujun on 2024/12/19.
//

#pragma once

#include <QGraphicsView>
#include <QGraphicsScene>
#include "BlockItem.hpp"
#include "Block.hpp"

namespace TinaToolBox {
    class BlockProgrammingView : public QGraphicsView {
        Q_OBJECT

    public:
        explicit BlockProgrammingView(QWidget *parent = nullptr);
        ~BlockProgrammingView() override;

        void addBlock(Block::Type type, const QPointF &pos);
    protected:
        bool eventFilter(QObject *watched, QEvent *event) override;
        
    private:
        QGraphicsScene *scene_;
        QList<Block *> blocks_;
        QList<BlockItem *> blockItems_;
    };
} // TinaToolBox
