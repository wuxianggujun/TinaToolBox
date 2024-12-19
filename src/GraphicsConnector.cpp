//
// Created by wuxianggujun on 2024/12/19.
//
#include <QPainter>
#include <QDebug>
#include "BlockItem.hpp"
#include "GraphicsConnector.hpp"

namespace TinaToolBox {
    
    GraphicsConnector::GraphicsConnector(Connector* connector, BlockItem* parent)
        : QGraphicsItem(parent), connector_(connector), parentBlockItem_(parent) {
        setFlags(QGraphicsItem::ItemIsSelectable);
        setAcceptHoverEvents(true); // 启用悬停事件
    }

    GraphicsConnector::~GraphicsConnector() {}

    QRectF GraphicsConnector::boundingRect() const {
        // 连接点的边界矩形
        qreal adjust = 2;
        switch (connector_->getConnectorType()) {
            case Connector::Type::INPUT:
                return QRectF(-2 - adjust, -5 - adjust, 4 + adjust * 2, 10 + adjust * 2);
            case Connector::Type::OUTPUT:
                return QRectF(-2 - adjust, -5 - adjust, 4 + adjust * 2, 10 + adjust * 2);
            default:
                return QRectF(-5 - adjust, -5 - adjust, 10 + adjust * 2, 10 + adjust * 2);
        }
    }

    void GraphicsConnector::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
        // 绘制连接点
        const int notchDepth = 10;
        const int notchWidth = 5;
        switch (connector_->getConnectorType()) {
            case Connector::Type::INPUT: {
                painter->setBrush(Qt::NoBrush);
                QPen pen(Qt::black, 2);
                painter->setPen(pen);
                QPainterPath path;
                path.moveTo(0, -notchDepth / 2);
                path.lineTo(notchWidth, 0);
                path.lineTo(0, notchDepth / 2);
                path.closeSubpath();
                painter->drawPath(path);
                break;
            }
            case Connector::Type::OUTPUT: {
                QPainterPath path;
                painter->setBrush(Qt::black);
                painter->setPen(Qt::NoPen);
                path.moveTo(0, -notchDepth / 2);
                path.lineTo(notchWidth, 0);
                path.lineTo(0, notchDepth / 2);
                path.closeSubpath();
                painter->drawPath(path);
                break;
            }
            default: {
                if (connector_->getConnectorType() == Connector::Type::INPUT) {
                    painter->setBrush(Qt::red);
                } else {
                    painter->setBrush(Qt::green);
                }
                painter->setPen(QPen(Qt::black, 1));
                painter->drawEllipse(QPointF(0, 0), 5, 5);
            }
        }
    }

    Connector* GraphicsConnector::getConnector() const {
        return connector_;
    }

    BlockItem* GraphicsConnector::getParentBlockItem() const {
        return parentBlockItem_;
    }
    
} // TinaToolBox