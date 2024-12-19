//
// Created by wuxianggujun on 2024/12/19.
//
#pragma once

#include <QGraphicsItem>

#include "BlockItem.hpp"
#include "Connector.hpp"


namespace TinaToolBox {
    class BlockItem;

    class GraphicsConnector:public QGraphicsItem{
    public:
        GraphicsConnector(Connector* connector,BlockItem* parent);
        ~GraphicsConnector() override;

        QRectF boundingRect() const override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        Connector* getConnector() const;
        BlockItem* getParentBlockItem() const;

    private:
        Connector* connector_;
        BlockItem* parentBlockItem_;
    };
} // TinaToolBox
