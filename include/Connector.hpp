//
// Created by wuxianggujun on 2024/12/19.
//

#pragma once

#include <QString>
// 连接点
namespace TinaToolBox {
    class Block;

    class Connector {
    public:
        enum Type {
            INPUT,
            OUTPUT
        };

        enum DataType {
            INTEGER,
            STRING,
            BOOLEAN,
            FLOAT,
            DOUBLE
        };

        Connector(Type type, DataType dataType, Block *block);

        //添加一些方法
        [[nodiscard]] Type getConnectorType() const;
        [[nodiscard]] DataType getDataType() const;
        [[nodiscard]] Block* getOwnerBlock() const;
        
    private:
        Type connectorType_;
        DataType dataType_;
        Block* ownerBlock_;
        
    };
} // TinaToolBox
