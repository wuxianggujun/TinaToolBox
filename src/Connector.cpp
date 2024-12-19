//
// Created by wuxianggujun on 2024/12/19.
//

#include "Block.hpp"
#include "Connector.hpp"

namespace TinaToolBox {
    Connector::Connector(Type type, DataType dataType, Block *block) : connectorType_(type), dataType_(dataType),
                                                                       ownerBlock_(block) {
    }

    Connector::Type Connector::getConnectorType() const {
        return connectorType_;
    }

    Connector::DataType Connector::getDataType() const {
        return dataType_;
    }

    Block *Connector::getOwnerBlock() const {
        return ownerBlock_;
    }
} // TinaToolBox
