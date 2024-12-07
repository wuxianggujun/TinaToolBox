//
// Created by wuxianggujun on 2024/11/24.
//

#ifndef TINA_TOOL_BOX_DOCUMENT_TAB_WIDGET_HPP
#define TINA_TOOL_BOX_DOCUMENT_TAB_WIDGET_HPP

#include <QTabWidget>
#include <QHBoxLayout>
#include "RunButton.hpp"
#include <QTabBar>

#include "Document.hpp"

namespace TinaToolBox {
    class DocumentTabWidget : public QTabWidget {
        Q_OBJECT

    public:
        explicit DocumentTabWidget(QWidget *parent = nullptr);

        // 添加公共方法
        int addDocumentTab(QWidget *widget, const QString &label);

        void removeDocumentTab(int index);

        QWidget *currentDocument() const;

        void addDocument(const std::shared_ptr<Document> &document,QWidget* view);
        void removeDocument(const std::shared_ptr<Document> &document);

    signals:
        void runButtonStateChanged(bool isRunning);

    protected:
        void showEvent(QShowEvent *event) override;

        void resizeEvent(QResizeEvent *event) override;

        QSize sizeHint() const override;

        QSize minimumSizeHint() const override;

    private:
        RunButton *runButton_;

        void updateRunButtonVisibility(const std::shared_ptr<Document> &document);
    };
}
#endif //TINA_TOOL_BOX_DOCUMENT_TAB_WIDGET_HPP
