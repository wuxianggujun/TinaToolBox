//
// Created by wuxianggujun on 2024/11/24.
//

#include "DocumentTabWidget.hpp"
#include <QTabBar>

DocumentTabWidget::DocumentTabWidget(QWidget *parent): QTabWidget(parent) {
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);

    // 创建一个容器来包含标签栏和工具栏
    tabBarContainer_ = new QWidget(this);
    tabBarLayout_ = new QHBoxLayout(tabBarContainer_);
    tabBarLayout_->setContentsMargins(0, 0, 0, 0);
    tabBarLayout_->setSpacing(0);

    // 将原始的标签栏移动到新的容器中
    QTabBar* bar = tabBar();
    bar->setParent(tabBarContainer_);
    tabBarLayout_->addWidget(bar, 1);  // 添加伸展因子1，使标签栏占据大部分空间

    // 创建运行按钮
    runButton_ = new RunButton(tabBarContainer_);
    runButton_->setFixedSize(20, 20);
    tabBarLayout_->addWidget(runButton_, 0, Qt::AlignVCenter);  // 添加到布局的右侧，垂直居中

    // 设置样式
    setStyleSheet(R"(
        QTabBar::tab {
            height: 35px;
        }
    )");

    // 添加一个空的占位标签页并立即移除，以确保tabBar始终显示
    addTab(new QWidget(), "");
    removeTab(0);
}

int DocumentTabWidget::addDocumentTab(QWidget* widget, const QString& label) {
    return addTab(widget, label);
}

void DocumentTabWidget::removeDocumentTab(int index) {
    removeTab(index);
}

QWidget* DocumentTabWidget::currentDocument() const {
    return currentWidget();
}

void DocumentTabWidget::resizeEvent(QResizeEvent *event) {
    QTabWidget::resizeEvent(event);
    // 确保tabBarContainer_与标签栏区域大小一致
    QRect r = tabBar()->geometry();
    tabBarContainer_->setGeometry(r);
}

void DocumentTabWidget::setupTabBar() {
}
