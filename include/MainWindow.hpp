#pragma once

#include <QMainWindow>
#include <QFileDialog>
#include <QSplitter>
#include <QListWidget>
#include <QMenuBar>
#include <QWidgetAction>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QMessageBox>
#include <QTreeWidgetItem>
#include "MainWindowMenuBar.hpp"
#include "Tokenizer.hpp"


namespace TinaToolBox {
    class LogPanel;
    class DocumentArea;
    class RecentFilesWidget;

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);

        ~MainWindow();

    protected:
        void mousePressEvent(QMouseEvent *event) override;

        void mouseMoveEvent(QMouseEvent *event) override;

        void mouseReleaseEvent(QMouseEvent *event) override;

        void mouseDoubleClickEvent(QMouseEvent *event) override;

        void closeEvent(QCloseEvent *event) override;

    private:
        QWidget *createLeftPanel();

        QWidget *createFileListToolBar();

        void onFileSelected(const QString& filePath);
        void onRemoveFileRequested(const QString& filePath);
        
    private slots:
        void handleMenuAction(const QString &actionName);

        void toggleMaximize();

        void showBottomPanel();

        void hideBottomPanel();
        
        void onSettingsClicked();

        void onFileDoubleClicked(const QTreeWidgetItem *item);

        void onRunButtonStateChanged(bool isRunning);

    private:
        bool isTitleBarArea(const QPoint &pos) const;

        void setUpUI();

        void createTileBar();
        
        void openFile();

        bool isScriptFile(const QString &filePath) const;
        

        void onScriptTreeItemDoubleClicked(const QTreeWidgetItem *item, int column);
        
        void setupConnections();

        void updateFileHistory(const QString &filePath);
        
        bool eventFilter(QObject *obj, QEvent *event) override;

        void filterTreeItems(bool showScriptsOnly);

        QPoint dragPosition;
        bool isDragging = false;
        MainWindowMenuBar *m_menuBar;
        QTabWidget *tabWidget;
        QWidget *centerWidget;
        QVBoxLayout *mainLayout;
        QWidget *titleBar;
        QSplitter *mainSplitter;
        QSplitter *rightSplitter;

        RecentFilesWidget *recentFilesWidget;
        QTabWidget *leftPanelTab;
        QComboBox *viewModeComboBox;

        DocumentArea *documentArea;
        QStackedWidget *propertyStack;
        QWidget *bottomPanel;
        LogPanel *logPanel;
        QPushButton *maxButton;
        QSplitter *bottomSplitter;
    };
}
