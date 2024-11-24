//
// Created by wuxianggujun on 2024/11/24.
//

#ifndef TINA_TOOL_BOX_DOCUMENT_TAB_WIDGET_HPP
#define TINA_TOOL_BOX_DOCUMENT_TAB_WIDGET_HPP

#include <QTabWidget>
#include <QHBoxLayout>
#include "RunButton.hpp"

class DocumentTabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit DocumentTabWidget(QWidget *parent = nullptr);

    // 添加公共方法
    int addDocumentTab(QWidget* widget, const QString& label);
    void removeDocumentTab(int index);
    QWidget* currentDocument() const;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupTabBar();

    QWidget *tabBarContainer_;
    QHBoxLayout *tabBarLayout_;
    RunButton *runButton_;
};

#endif //TINA_TOOL_BOX_DOCUMENT_TAB_WIDGET_HPP
