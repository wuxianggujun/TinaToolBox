//
// Created by wuxianggujun on 2024/12/19.
//

#include "Block.hpp"

namespace TinaToolBox {
    Block::Block(const Type type): type_(type), parent_(nullptr) {
    }

    Block::~Block() {
        // 释放连接点资源
        clearChildren();

        for (Connector *connector: connectors_) {
            delete connector;
        }
        connectors_.clear();
    }

    void Block::addParameter(const QString &param) {
        parameters_.append(param);
    }

    void Block::setParent(Block *parent) {
        parent_ = parent;
    }

    void Block::addChild(Block *child) {
        children_.append(child);
        if (child != nullptr) {
            child->setParent(this);
        }
    }

    Block::Type Block::getType() const {
        return type_;
    }

    const QList<QString> &Block::getParameters() const {
        return parameters_;
    }

    Block *Block::getParent() const {
        return parent_;
    }

    const QList<Block *> &Block::getChildren() const {
        return children_;
    }

    void Block::removeChild(Block *child) {
        children_.removeOne(child);
        // 断开双向连接
        if (child != nullptr) {
            child->setParent(nullptr);
        }
    }

    void Block::clearChildren() {
        for (Block *child: children_) {
            if (child != nullptr) {
                child->setParent(nullptr);
            }
        }
        children_.clear();
    }

    void Block::addConnector(Connector::Type type, Connector::DataType dataType) {
        auto *connector = new Connector(type, dataType, this);
        connectors_.append(connector);
    }

    void Block::removeConnector(Connector *connector) {
        connectors_.removeOne(connector);
        delete connector;
    }

    const QList<Connector *> &Block::getConnectors() const {
        return connectors_;
    }

    Connector *Block::findConnector(Connector::Type type, Connector::DataType dataType) const {
        for (Connector *connector: connectors_) {
            if (connector->getConnectorType() == type && connector->getDataType() == dataType) {
                return connector;
            }
        }
        return nullptr;
    }
} // TinaToolBox
