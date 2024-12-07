// MainWindowMenuBar.cpp
#include "MainWindowMenuBar.hpp"
#include <QTimer>
#include <QEvent>
#include <QStyle>

namespace TinaToolBox {
    MainWindowMenuBar::MainWindowMenuBar(QWidget *parent) : QWidget(parent) {
        setFixedHeight(30);
        setMouseTracking(true);
    
        m_layout = new QHBoxLayout(this);
        m_layout->setSpacing(0);
        m_layout->setContentsMargins(0, 0, 0, 0);


        // 设置全局样式
        setStyleSheet(R"(
    MainWindowMenuBar {
        background-color: #F0F0F0;
    }
    /* 左侧菜单按钮样式 */
    QPushButton[class="menu-button"] {
        border: none;
        height: 30px;
        padding: 5px 10px;
        margin: 0px;
        background: transparent;
    }
    /* 活动状态的菜单按钮 */
    QPushButton[class="menu-button"][active="true"] {
        background-color: #E0E0E0;
    }
    /* 右侧控制按钮样式 */
    QPushButton[class="control-button"] {
        border: none;
        height: 30px;
        min-width: 45px;
        padding: 0;
        background: transparent;
    }
    QPushButton[class="control-button"]:hover {
        background-color: #E0E0E0;
    }
    QPushButton::menu-indicator {
        width: 0px;
    }
    #close_button:hover {
        background-color: #E81123;
        color: white;
    }
)");

        // 初始化菜单数据
        m_menuData = {
            {
                "文件(&F)", {
                    {"新建", "Ctrl+N"},
                    {"打开", "Ctrl+O"},
                    {"保存", "Ctrl+S"},
                    {"另存为", "Ctrl+Shift+S"},
                    {"", ""}, // separator
                    {"导入", "Ctrl+I"},
                    {"导出", "Ctrl+E"},
                    {"", ""}, // separator
                    {"退出", "Alt+F4"}
                }
            },
            {
                "编辑(&E)", {
                    {"撤销", "Ctrl+Z"},
                    {"重做", "Ctrl+Y"},
                    {"", ""}, // separator
                    {"剪切", "Ctrl+X"},
                    {"复制", "Ctrl+C"},
                    {"粘贴", "Ctrl+V"}
                }
            }
        };

        setupMenus();
        setupWindowControls();
    }

    void MainWindowMenuBar::setupMenus() {
        for (auto it = m_menuData.begin(); it != m_menuData.end(); ++it) {
            QPushButton *menuBtn = new QPushButton(it.key(), this);
            menuBtn->setProperty("class", "menu-button");
            menuBtn->setFlat(true);
            menuBtn->setFixedHeight(30);

            QMenu *menu = new QMenu(this);
            menu->installEventFilter(this);

            for (const auto &[fst, snd]: it.value()) {
                if (fst.isEmpty()) {
                    menu->addSeparator();
                } else {
                    QAction *action = menu->addAction(fst);
                    if (!snd.isEmpty()) {
                        action->setShortcut(QKeySequence(snd));
                    }
                    connect(action, &QAction::triggered, this, [this, name = fst]() {
                        emit menuActionTriggered(name);
                    });
                }
            }

            // 连接菜单隐藏信号
            connect(menu, &QMenu::aboutToHide, this, [this]() {
                // 只有在不是切换到其他菜单的情况下才清空状态
                if (!m_switchingMenu) {
                    m_activeMenu = nullptr;
                    m_activeMenuButton = nullptr;
                }
                m_switchingMenu = false;
            });

            menuBtn->setMenu(menu);

            menuBtn->installEventFilter(this);
            m_layout->addWidget(menuBtn);
        }
        m_layout->addStretch();
    }

    void MainWindowMenuBar::setupWindowControls() {
        m_layout->addStretch();

        // 设置按钮
        m_settingsBtn = new QPushButton("⚙", this);
        m_settingsBtn->setProperty("class", "control-button"); // 设置为控制按钮类
        m_settingsBtn->setFixedSize(45, 30);
        connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindowMenuBar::settingsClicked);
        m_layout->addWidget(m_settingsBtn);

        // 最小化按钮
        m_minBtn = new QPushButton("─", this);
        m_minBtn->setProperty("class", "control-button"); // 设置为控制按钮类
        m_minBtn->setFixedSize(45, 30);
        connect(m_minBtn, &QPushButton::clicked, this, &MainWindowMenuBar::minimizeClicked);
        m_layout->addWidget(m_minBtn);

        // 最大化按钮
        m_maxBtn = new QPushButton("□", this);
        m_maxBtn->setProperty("class", "control-button"); // 设置为控制按钮类
        m_maxBtn->setFixedSize(45, 30);
        connect(m_maxBtn, &QPushButton::clicked, this, &MainWindowMenuBar::maximizeClicked);
        m_layout->addWidget(m_maxBtn);

        // 关闭按钮
        m_closeBtn = new QPushButton("×", this);
        m_closeBtn->setProperty("class", "control-button"); // 设置为控制按钮类
        m_closeBtn->setObjectName("close_button");
        m_closeBtn->setFixedSize(45, 30);
        connect(m_closeBtn, &QPushButton::clicked, this, &MainWindowMenuBar::closeClicked);
        m_layout->addWidget(m_closeBtn);
    }

    void MainWindowMenuBar::updateMaximizeButton(bool isMaximized) {
        if (m_maxBtn) {
            m_maxBtn->setText(isMaximized ? "❐" : "□");
        }
    }

    bool MainWindowMenuBar::eventFilter(QObject *watched, QEvent *event) {
        if (auto *menu = qobject_cast<QMenu*>(watched)) {
            if (event->type() == QEvent::Hide) {
                if (!m_switchingMenu) {
                    if (m_activeMenuButton) {
                        m_activeMenuButton->setProperty("active", false);
                        m_activeMenuButton->style()->unpolish(m_activeMenuButton);
                        m_activeMenuButton->style()->polish(m_activeMenuButton);
                    }
                    m_activeMenu = nullptr;
                    m_activeMenuButton = nullptr;
                }
                m_switchingMenu = false;
                return true;
            }
        }

        // 当有活动菜单时，检查鼠标位置
        if (m_activeMenu && (event->type() == QEvent::MouseMove || event->type() == QEvent::Enter)) {
            QPoint globalPos = QCursor::pos();
        
            // 遍历所有菜单按钮
            for (auto *child : children()) {
                if (auto *btn = qobject_cast<QPushButton*>(child)) {
                    if (btn->property("class").toString() == "menu-button" && btn != m_activeMenuButton) {
                        QRect btnGeometry = QRect(btn->mapToGlobal(QPoint(0, 0)), btn->size());
                    
                        if (btnGeometry.contains(globalPos)) {
                            // 更新按钮状态
                            if (m_activeMenuButton) {
                                m_activeMenuButton->setProperty("active", false);
                                m_activeMenuButton->style()->unpolish(m_activeMenuButton);
                                m_activeMenuButton->style()->polish(m_activeMenuButton);
                            }
                        
                            btn->setProperty("active", true);
                            btn->style()->unpolish(btn);
                            btn->style()->polish(btn);
                        
                            m_switchingMenu = true;
                            m_activeMenu->hide();
                        
                            QPoint pos = btnGeometry.bottomLeft();
                            btn->menu()->popup(pos);
                            m_activeMenu = btn->menu();
                            m_activeMenuButton = btn;
                            return true;
                        }
                    }
                }
            }
        }

        // 处理菜单按钮的点击事件
        auto *menuBtn = qobject_cast<QPushButton*>(watched);
        if (menuBtn && menuBtn->property("class").toString() == "menu-button") {
            if (event->type() == QEvent::MouseButtonPress) {
                if (m_activeMenu == menuBtn->menu()) {
                    menuBtn->setProperty("active", false);
                    menuBtn->style()->unpolish(menuBtn);
                    menuBtn->style()->polish(menuBtn);
                
                    m_activeMenu->hide();
                    m_activeMenu = nullptr;
                    m_activeMenuButton = nullptr;
                } else {
                    // 更新按钮状态
                    if (m_activeMenuButton) {
                        m_activeMenuButton->setProperty("active", false);
                        m_activeMenuButton->style()->unpolish(m_activeMenuButton);
                        m_activeMenuButton->style()->polish(m_activeMenuButton);
                    }
                
                    menuBtn->setProperty("active", true);
                    menuBtn->style()->unpolish(menuBtn);
                    menuBtn->style()->polish(menuBtn);
                
                    m_switchingMenu = true;
                    if (m_activeMenu) {
                        m_activeMenu->hide();
                    }
                    QPoint pos = menuBtn->mapToGlobal(QPoint(0, menuBtn->height()));
                    menuBtn->menu()->popup(pos);
                    m_activeMenu = menuBtn->menu();
                    m_activeMenuButton = menuBtn;
                }
                return true;
            }
        }

        return QWidget::eventFilter(watched, event);
    }

    QMenu *MainWindowMenuBar::addMenu(const QString &title) {
        QPushButton *button = new QPushButton(title, this);
        button->setProperty("class", "menu-button"); // 设置属性以应用特殊样式
        button->setFlat(true);
        button->setFixedHeight(30);

        QMenu *menu = new QMenu(this);
        button->setMenu(menu);

        // 在stretch之前插入按钮
        m_layout->insertWidget(m_layout->count() - (m_layout->count() > 0 ? 5 : 1), button);

        return menu;
    }
}