//
// Created by wuxianggujun on 2024/12/19.
//

#include "BlockItem.hpp"
#include "GraphicsConnector.hpp"
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QBrush>
#include <QFontMetrics>
#include <QDebug>

namespace TinaToolBox {
    BlockItem::BlockItem(Block *block, QGraphicsItem *parent) : QGraphicsItem(parent), block_(block) {
        setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
        // 启用悬停事件
        setAcceptHoverEvents(true);

        for (Connector *connector: block_->getConnectors()) {
            auto *graphicsConnector = new GraphicsConnector(connector, this);
            connectorMap_.insert(connector, graphicsConnector);
        }
        updateShape();
        updateConnectorPositions();
    }

    BlockItem::~BlockItem() {
        // 在析构函数中释放资源
        for (auto it = connectorMap_.begin(); it != connectorMap_.end(); ++it) {
            delete it.value();
        }
        connectorMap_.clear();
    }

    QRectF BlockItem::boundingRect() const {
        // 边界矩形应足够大以包含整个积木块及其连接点
        return blockShape_.boundingRect().adjusted(-CONNECTOR_RADIUS, -CONNECTOR_RADIUS, CONNECTOR_RADIUS,
                                                   CONNECTOR_RADIUS);
    }

    QPainterPath BlockItem::shape() const {
        return blockShape_;
    }

    void BlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
        painter->setRenderHint(QPainter::Antialiasing);
        // 绘制积木块
        drawBlock(painter);
        // 绘制连接点
        // 注意：由于 GraphicsConnector 是 BlockItem 的子项，它们会自动绘制，这里可以根据需要添加一些额外的装饰
        drawConnectors(painter);
    }

    Block *BlockItem::getBlock() const {
        return block_;
    }

    void BlockItem::addGraphicsConnector(GraphicsConnector *connector) {
        connectorMap_.insert(connector->getConnector(), connector);
    }

    GraphicsConnector *BlockItem::getGraphicsConnector(Connector *connector) const {
        return connectorMap_.value(connector, nullptr);
    }

    void BlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            // 记录鼠标按下时的位置
        }

        QGraphicsItem::mousePressEvent(event);
    }

    void BlockItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
        // 处理鼠标移动事件，实现积木块的拖拽
        if (event->buttons() & Qt::LeftButton) {
            // ... 拖拽逻辑 ...
        }
        QGraphicsItem::mouseMoveEvent(event);
    }

    void BlockItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
        // 处理鼠标释放事件，例如检测连接
        if (event->button() == Qt::LeftButton) {
            // ... 连接逻辑 ...
        }
        QGraphicsItem::mouseReleaseEvent(event);
    }

    void BlockItem::updateShape() {
        // 根据 Block 的类型和其他属性更新积木块的形状
        blockShape_ = QPainterPath();
        const int borderRadius = 10; // 圆角半径
        const int notchDepth = 10; // 凹槽深度
        const int notchWidth = 20; // 凹槽宽度

        switch (block_->getType()) {
            case Block::Type::IF: {
                // 六边形
                const int hexagonWidth = BLOCK_WIDTH;
                const int hexagonHeight = BLOCK_HEIGHT;
                const int sideWidth = hexagonWidth / 4;
                blockShape_.moveTo(sideWidth, 0);
                blockShape_.lineTo(hexagonWidth - sideWidth, 0);
                blockShape_.lineTo(hexagonWidth, hexagonHeight / 2);
                blockShape_.lineTo(hexagonWidth - sideWidth, hexagonHeight);
                blockShape_.lineTo(sideWidth, hexagonHeight);
                blockShape_.lineTo(0, hexagonHeight / 2);
                blockShape_.closeSubpath();
                break;
            }
            case Block::Type::LOOP: {
                // C 形
                const int notchDepth = 10;
                const int notchWidth = 20;

                blockShape_.moveTo(borderRadius, 0);
                blockShape_.lineTo(BLOCK_WIDTH - borderRadius, 0);
                blockShape_.arcTo(BLOCK_WIDTH - borderRadius * 2, 0, borderRadius * 2, borderRadius * 2, 90, -90);

                blockShape_.lineTo(BLOCK_WIDTH, BLOCK_HEIGHT - borderRadius);
                blockShape_.arcTo(BLOCK_WIDTH - borderRadius * 2, BLOCK_HEIGHT - borderRadius * 2, borderRadius * 2,
                                  borderRadius * 2, 0, -90);

                blockShape_.lineTo(notchWidth + borderRadius, BLOCK_HEIGHT);
                blockShape_.arcTo(notchWidth, BLOCK_HEIGHT - borderRadius, borderRadius * 2, borderRadius, 270, 90);

                blockShape_.lineTo(notchWidth, notchDepth + borderRadius);
                blockShape_.arcTo(notchWidth - borderRadius, notchDepth, borderRadius, borderRadius, 0, -90);

                blockShape_.lineTo(borderRadius, notchDepth);
                blockShape_.arcTo(0, notchDepth, borderRadius * 2, borderRadius * 2, 180, 90);
                blockShape_.lineTo(0, borderRadius);
                blockShape_.arcTo(0, 0, borderRadius * 2, borderRadius * 2, 180, -90);
                blockShape_.closeSubpath();
                break;
            }
            case Block::Type::PRINT: {
                blockShape_.addRoundedRect(0, 0, BLOCK_WIDTH, BLOCK_HEIGHT, borderRadius, borderRadius);
                break;
            }
            case Block::Type::START: {
                blockShape_.moveTo(0, BLOCK_HEIGHT);
                blockShape_.lineTo(0, borderRadius);
                blockShape_.arcTo(0, 0, borderRadius * 2, borderRadius * 2, 180, -90);
                blockShape_.lineTo(BLOCK_WIDTH - borderRadius, 0);
                blockShape_.arcTo(BLOCK_WIDTH - borderRadius * 2, 0, borderRadius * 2, borderRadius * 2, 90, -90);
                blockShape_.lineTo(BLOCK_WIDTH, BLOCK_HEIGHT);
                blockShape_.closeSubpath();
                break;
            }
            case Block::Type::COMMAND: {
                // 带凹槽和凸起的矩形
                const int notchDepth = 10;
                const int notchWidth = 20;
                blockShape_.moveTo(0, notchDepth + borderRadius);
                blockShape_.arcTo(0, notchDepth, borderRadius * 2, borderRadius * 2, 180, -90);
                blockShape_.lineTo(notchWidth - borderRadius, notchDepth);
                blockShape_.arcTo(notchWidth - borderRadius, 0, borderRadius, borderRadius, 270, -90);

                blockShape_.lineTo(notchWidth + borderRadius, 0);
                blockShape_.arcTo(notchWidth, -borderRadius, borderRadius * 2, borderRadius, 90, 90);

                blockShape_.lineTo(BLOCK_WIDTH - borderRadius, 0);
                blockShape_.arcTo(BLOCK_WIDTH - borderRadius * 2, 0, borderRadius * 2, borderRadius * 2, 90, -90);
                blockShape_.lineTo(BLOCK_WIDTH, BLOCK_HEIGHT - borderRadius);
                blockShape_.arcTo(BLOCK_WIDTH - borderRadius * 2, BLOCK_HEIGHT - borderRadius * 2, borderRadius * 2,
                                  borderRadius * 2, 0, -90);
                blockShape_.lineTo(notchWidth + borderRadius, BLOCK_HEIGHT);
                blockShape_.arcTo(notchWidth, BLOCK_HEIGHT - borderRadius, borderRadius * 2, borderRadius, 270, 90);
                blockShape_.lineTo(notchWidth - borderRadius, BLOCK_HEIGHT - notchDepth);
                blockShape_.arcTo(0, BLOCK_HEIGHT - notchDepth - borderRadius * 2, borderRadius * 2, borderRadius * 2,
                                  180, 90);
                blockShape_.closeSubpath();
                break;
            }
            case Block::Type::BOOLEAN: {
                // 菱形
                blockShape_.moveTo(BLOCK_WIDTH / 2, 0);
                blockShape_.lineTo(BLOCK_WIDTH, BLOCK_HEIGHT / 2);
                blockShape_.lineTo(BLOCK_WIDTH / 2, BLOCK_HEIGHT);
                blockShape_.lineTo(0, BLOCK_HEIGHT / 2);
                blockShape_.closeSubpath();
                break;
            }
            case Block::Shape::NOTCHED_RECTANGLE:
                blockShape_.moveTo(0, notchDepth + borderRadius);
            blockShape_.arcTo(0, notchDepth, borderRadius * 2, borderRadius * 2, 180, -90); // 左上角圆角
            blockShape_.lineTo(notchWidth - borderRadius, notchDepth); // 凹槽左侧
            blockShape_.lineTo(notchWidth - borderRadius, notchDepth * 0.6); // 凹槽内部上点
            blockShape_.lineTo(notchWidth + borderRadius, notchDepth * 0.6); // 凹槽内部下点
            blockShape_.lineTo(notchWidth + borderRadius, notchDepth); // 凹槽右侧

            blockShape_.lineTo(BLOCK_WIDTH - borderRadius, notchDepth); // 顶部直线
            blockShape_.arcTo(BLOCK_WIDTH - borderRadius * 2, notchDepth, borderRadius * 2, borderRadius * 2, 90, -90); // 右上角圆角
            blockShape_.lineTo(BLOCK_WIDTH, BLOCK_HEIGHT - borderRadius); // 右侧直线
            blockShape_.arcTo(BLOCK_WIDTH - borderRadius * 2, BLOCK_HEIGHT - borderRadius * 2, borderRadius * 2, borderRadius * 2, 0, -90); // 右下角圆角

            blockShape_.lineTo(notchWidth + borderRadius, BLOCK_HEIGHT); // 底部直线

            blockShape_.lineTo(notchWidth + borderRadius, BLOCK_HEIGHT - notchDepth * 0.6);
            blockShape_.lineTo(notchWidth - borderRadius, BLOCK_HEIGHT - notchDepth * 0.6);
            blockShape_.lineTo(notchWidth - borderRadius, BLOCK_HEIGHT);

            blockShape_.arcTo(0, BLOCK_HEIGHT - borderRadius * 2, borderRadius * 2, borderRadius * 2, 270, -90); // 左下角圆角

            blockShape_.closeSubpath();
                break;

            default: {
                blockShape_.addRect(0, 0, BLOCK_WIDTH, BLOCK_HEIGHT);
                break;
            }
        }
        prepareGeometryChange();
    }

    void BlockItem::updateConnectorPositions() {
        // 更新连接点的位置
        for (auto it = connectorMap_.begin(); it != connectorMap_.end(); ++it) {
            Connector *connector = it.key();
            GraphicsConnector *graphicsConnector = it.value();
            QPointF pos = getConnectorPosition(connector);
            graphicsConnector->setPos(pos);
        }
    }

    void BlockItem::drawBlock(QPainter *painter) {
        // 绘制积木块主体
        QPen pen(Qt::black, 2);
        QBrush brush(block_->getColor());
        painter->setPen(pen);
        painter->setBrush(brush);
        painter->drawPath(blockShape_);
        // 绘制文本
        QString text;
        switch (block_->getType()) {
            case Block::IF:
                text = "IF";
                break;
            case Block::LOOP:
                text = "LOOP";
                break;
            case Block::PRINT:
                text = "PRINT";
                break;
        }

        // 添加参数到文本中
        for (const QString &param: block_->getParameters()) {
            text += "\n" + param;
        }

        QFontMetrics metrics(painter->font());
        QRect textRect = metrics.boundingRect(QRect(0, 0, BLOCK_WIDTH, BLOCK_HEIGHT), Qt::AlignCenter, text);
        painter->drawText(textRect, Qt::AlignCenter, text);
    }

    void BlockItem::drawConnectors(QPainter *painter) {
        // 绘制连接点（可选的额外装饰）
        for (auto it = connectorMap_.constBegin(); it != connectorMap_.constEnd(); ++it) {
            const Connector *connector = it.key();
            const GraphicsConnector *graphicsConnector = it.value();

            // 根据连接点类型设置不同的颜色
            QBrush connectorBrush;
            if (connector->getConnectorType() == Connector::Type::INPUT) {
                connectorBrush = QBrush(Qt::red); // 输入连接点为红色
            } else {
                connectorBrush = QBrush(Qt::green); // 输出连接点为绿色
            }

            painter->setBrush(connectorBrush);
            painter->setPen(Qt::NoPen); // 连接点通常没有边框

            // 绘制一个圆形作为连接点的装饰
            painter->drawEllipse(graphicsConnector->pos(), CONNECTOR_RADIUS, CONNECTOR_RADIUS);
        }
    }

    QPointF BlockItem::getConnectorPosition(const Connector *connector) const {
        // 根据连接点类型和积木块的形状计算连接点的坐标
        QPointF connectorPos;
        const int notchDepth = 10;
        const int notchWidth = 20;
        int index = 0;

        // 查找当前连接点在其类型列表中的索引
        const QList<Connector *> &connectors = block_->getConnectors();
        for (int i = 0; i < connectors.size(); ++i) {
            if (connectors[i] == connector) {
                index = i;
                break;
            }
        }
        switch (block_->getShape()) {
            case Block::Shape::RECTANGLE:
                switch (connector->getConnectorType()) {
                    case Connector::Type::INPUT:
                        connectorPos = QPointF(notchWidth / 2, 0);
                        break;
                    case Connector::Type::OUTPUT:
                        connectorPos = QPointF(notchWidth / 2, BLOCK_HEIGHT);
                        break;
                    default:
                        connectorPos = QPointF(0, 0);
                        break;
                }
                break;
            case Block::Shape::ROUNDED:
                switch (connector->getConnectorType()) {
                    case Connector::Type::INPUT:
                        connectorPos = QPointF(BLOCK_WIDTH / 2, 0);
                        break;
                    case Connector::Type::OUTPUT:
                        connectorPos = QPointF(BLOCK_WIDTH / 2, BLOCK_HEIGHT);
                        break;
                    default:
                        connectorPos = QPointF(0, 0);
                        break;
                }
                break;
            case Block::Shape::HEXAGON:
                switch (connector->getConnectorType()) {
                    case Connector::Type::INPUT:
                        connectorPos = QPointF(BLOCK_WIDTH / 2, 0);
                        break;
                    case Connector::Type::OUTPUT:
                        connectorPos = QPointF(BLOCK_WIDTH / 2, BLOCK_HEIGHT);
                        break;
                    default:
                        connectorPos = QPointF(0, 0);
                        break;
                }
                break;
            case Block::Shape::C_SHAPE:
                switch (connector->getConnectorType()) {
                    case Connector::Type::INPUT:
                        connectorPos = QPointF(notchWidth / 2, 0);
                        break;
                    case Connector::Type::OUTPUT:
                        connectorPos = QPointF(notchWidth / 2, BLOCK_HEIGHT);
                        break;
                    default:
                        connectorPos = QPointF(0, 0);
                        break;
                }
                break;
            case Block::Shape::NOTCHED_RECTANGLE:
                switch (connector->getConnectorType()) {
                    case Connector::Type::INPUT:
                        connectorPos = QPointF(notchWidth / 2, 0); // 凹槽中心
                        break;
                    case Connector::Type::OUTPUT:
                        connectorPos = QPointF(notchWidth / 2, BLOCK_HEIGHT); // 凸起中心
                        break;
                    default:
                        connectorPos = QPointF(0, 0);
                        break;
                }
            default:
                switch (connector->getConnectorType()) {
                    case Connector::Type::INPUT:
                        // 输入连接点通常位于积木块的左侧
                        connectorPos = QPointF(0, (index + 1) * (BLOCK_HEIGHT / (connectors.size() + 1)));
                        break;
                    case Connector::Type::OUTPUT:
                        // 输出连接点通常位于积木块的右侧
                        connectorPos = QPointF(BLOCK_WIDTH, (index + 1) * (BLOCK_HEIGHT / (connectors.size() + 1)));
                        break;
                    default:
                        connectorPos = QPointF(0, 0);
                        break;
                }
                break;
        }
        return connectorPos;
    }

    QVariant BlockItem::itemChange(GraphicsItemChange change, const QVariant &value) {
        if (change == ItemPositionHasChanged) {
            updateConnectorPositions();
        }
        return QGraphicsItem::itemChange(change, value);
    }
} // TinaToolBox
