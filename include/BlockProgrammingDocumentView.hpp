//
// Created by wuxianggujun on 2024/12/19.
//

#pragma once

#include <QObject>
#include "DocumentView.hpp"
#include "BlockProgrammingView.hpp"

namespace TinaToolBox {
    class BlockProgrammingDocumentView : public QObject, public IDocumentView {
        Q_OBJECT
    public:
        explicit BlockProgrammingDocumentView(std::shared_ptr<Document> document, QWidget *parent = nullptr);
        ~BlockProgrammingDocumentView() override;
        
        void updateContent() override;

        bool saveContent() override;

        QWidget *widget() override;
    private:
        std::shared_ptr<Document> document_;
        BlockProgrammingView* blockProgrammingView_;
    };
} // TinaToolBox
