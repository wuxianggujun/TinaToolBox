//
// Created by wuxianggujun on 2024/12/19.
//

#pragma once

#include <QGraphicsItem>
#include <QPainterPath>
#include <QMap>
#include <QPainter>
#include "Block.hpp"
#include "Connector.hpp"

namespace TinaToolBox {
    class GraphicsConnector;

    class BlockItem : public QGraphicsItem {
    public:
        explicit BlockItem(Block *block, QGraphicsItem *parent = nullptr);

        ~BlockItem() override;

        QRectF boundingRect() const override;

        QPainterPath shape() const override;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

        Block *getBlock() const;

        // 添加图形连接点
        void addGraphicsConnector(GraphicsConnector *connector);

        // 获取与指定 Connector 关联的 GraphicsConnector
        GraphicsConnector *getGraphicsConnector(Connector *connector) const;

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    private:
        Block *block_;
        // 积木块的形状
        QPainterPath blockShape_;
        // 连接点映射
        QMap<Connector*, GraphicsConnector *> connectorMap_;

        void updateShape();

        void updateConnectorPositions();

        void drawBlock(QPainter *painter);

        void drawConnectors(QPainter *painter);

        // 获取连接点的坐标
        QPointF getConnectorPosition(const Connector *connector) const;

        QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

        static constexpr int BLOCK_WIDTH = 100;
        static constexpr int BLOCK_HEIGHT = 40;
        static constexpr int CONNECTOR_RADIUS = 5;
    };
} // TinaToolBox
