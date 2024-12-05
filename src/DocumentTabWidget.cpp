//
// Created by wuxianggujun on 2024/11/24.
//

#include "DocumentTabWidget.hpp"

#include <QTimer>
#include <QHBoxLayout>
#include <QWidget>
#include <spdlog/spdlog.h>

#include "SettingsPanel.hpp"


void DocumentTabWidget::showEvent(QShowEvent* event) {
    QTabWidget::showEvent(event);
}

void DocumentTabWidget::resizeEvent(QResizeEvent* event) {
    QTabWidget::resizeEvent(event);
}

QSize DocumentTabWidget::sizeHint() const {
    QSize sz = QTabWidget::sizeHint();
    sz.setHeight(sz.height() + 35);  // 确保总高度包含标签栏高度
    return sz;
}

QSize DocumentTabWidget::minimumSizeHint() const {
    QSize sz = QTabWidget::minimumSizeHint();
    sz.setHeight(sz.height() + 35);  // 确保最小高度包含标签栏高度
    return sz;
}

DocumentTabWidget::DocumentTabWidget(QWidget *parent): QTabWidget(parent) {
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);
    
    // 设置QTabWidget的样式
    setStyleSheet(R"(
        QTabBar::tab {
            height: 35px;
            padding: 0px 10px;
            background: transparent;
        }
        
        QTabBar::tab:selected {
            background: #ffffff;
            border-bottom: 2px solid #007acc;
        }
        
        QTabBar::tab:hover:!selected {
            background: #e0e0e0;
        }
    )");
    
    // 创建运行按钮和容器
    auto* cornerWidget = new QWidget(this);
    auto* cornerLayout = new QHBoxLayout(cornerWidget);
    cornerLayout->setContentsMargins(0, 0, 0, 0);
    cornerLayout->setSpacing(0);
    
    runButton_ = new RunButton(this);
    runButton_->setFixedSize(35, 35);
    
    // 将按钮添加到布局中，并设置垂直居中对齐
    cornerLayout->addWidget(runButton_, 0, Qt::AlignVCenter);
    cornerWidget->setFixedHeight(35);

    connect(runButton_,&RunButton::stateChanged,this,[this](bool running)
    {
            if (count() > 0 && currentIndex() >= 0)
            {
                QWidget* currentDoc = currentWidget();
                if (currentDoc && !qobject_cast<SettingsPanel*>(currentDoc))
                {
                    emit runButtonStateChanged(true);
                }
            }
    });

    connect(this, &QTabWidget::currentChanged, this, [this](int index)
    {
    	// 只在当前标签页是可编辑文档时显示运行按钮
	    if (runButton_)
	    {
		    bool isSettingTab = (widget(index) && qobject_cast<SettingsPanel*>(widget(index)));
            runButton_->setVisible(!isSettingTab);
	    }
    });

    // 设置corner widget
    setCornerWidget(cornerWidget, Qt::TopRightCorner);
}


int DocumentTabWidget::addDocumentTab(QWidget* widget, const QString& label) {
    int index = addTab(widget, label);
    return index;
}

void DocumentTabWidget::removeDocumentTab(int index) {
    removeTab(index);
}

QWidget* DocumentTabWidget::currentDocument() const {
    return currentWidget();
}
