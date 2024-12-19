//
// Created by wuxianggujun on 2024/12/19.
//

#pragma once
#include <QColor>
#include <QList>
#include <QString>

#include "Connector.hpp"

// 数据模型
namespace TinaToolBox {
    class Block {
    public:
        enum Type {
            IF,
            LOOP,
            PRINT,
            START,
            END,
            COMMAND,
            REPORTER,
            BOOLEAN
        };

        enum Shape {
            RECTANGLE,
            ROUNDED,
            HEXAGON,
            C_SHAPE,
            NOTCHED_RECTANGLE, // 新增：带有凹槽和凸起的矩形
        };

        explicit Block(Type type);

        ~Block();

        void addParameter(const QString &param);

        void setParent(Block *parent);

        void addChild(Block *child);

        [[nodiscard]] Type getType() const;

        [[nodiscard]] QColor getColor() const;

        void setColor(const QColor &color);

        [[nodiscard]] Shape getShape() const;
        void setShape(Shape shape);

        [[nodiscard]] const QList<QString> &getParameters() const;

        [[nodiscard]] Block *getParent() const;

        [[nodiscard]] const QList<Block *> &getChildren() const;

        void removeChild(Block *child);

        void clearChildren();

        // 添加连接点
        void addConnector(Connector::Type type, Connector::DataType dataType);

        void removeConnector(Connector *connector);

        [[nodiscard]] const QList<Connector *> &getConnectors() const;

        [[nodiscard]] Connector *findConnector(Connector::Type type, Connector::DataType dataType) const;

    private:
        Type type_;
        QColor color_;
        Shape shape_;
        QList<QString> parameters_;
        Block *parent_;
        QList<Block *> children_;
        QList<Connector *> connectors_; // 连接点列表
    };
} // TinaToolBox
