//
// Created by wuxianggujun on 2024/12/19.
//

#pragma once
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
            PRINT
        };

        explicit Block(Type type);

        ~Block();

        void addParameter(const QString &param);

        void setParent(Block *parent);

        void addChild(Block *child);

        [[nodiscard]] Type getType() const;

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
        QList<QString> parameters_;
        Block *parent_;
        QList<Block *> children_;
        QList<Connector *> connectors_; // 连接点列表
    };
} // TinaToolBox
