// MainWindowMenuBar.hpp
#ifndef TINA_TOOL_BOX_MAIN_WINDOW_MENU_BAR_HPP
#define TINA_TOOL_BOX_MAIN_WINDOW_MENU_BAR_HPP

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMenu>
#include <QMap>

class MainWindowMenuBar : public QWidget {
    Q_OBJECT

public:
    explicit MainWindowMenuBar(QWidget *parent = nullptr);

    // 添加菜单方法
    QMenu *addMenu(const QString &title);

    // 设置菜单数据
    void setupMenus();

    void setupWindowControls();

    void updateMaximizeButton(bool isMaximized);
    
signals:
    void menuActionTriggered(const QString &actionName);

    void minimizeClicked();

    void maximizeClicked();

    void closeClicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    
private:
    QHBoxLayout *m_layout;
    QPushButton *m_settingsBtn;
    QPushButton *m_minBtn;
    QPushButton *m_maxBtn;
    QPushButton *m_closeBtn;
    // 存储菜单数据的结构
    QMap<QString, QList<QPair<QString, QString> > > m_menuData;
    bool m_switchingMenu = false;  // 用于跟踪菜单切换状态
    QMenu* m_activeMenu =  nullptr;
    QPushButton* m_activeMenuButton = nullptr;
};

#endif // TINA_TOOL_BOX_MAIN_WINDOW_MENU_BAR_HPP
