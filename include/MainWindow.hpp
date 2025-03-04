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

#include "ConfigManager.hpp"
#include "LogSystem.hpp"
#include "MainWindowMenuBar.hpp"
#include "Singleton.hpp"
#include "ThemeManager.hpp"
#include "Tokenizer.hpp"


namespace TinaToolBox {
    class StatusBar;
    class LogPanel;
    class DocumentArea;
    class RecentFilesWidget;

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);

        ~MainWindow();

    protected:
        void closeEvent(QCloseEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
        

    private:
        QWidget *createLeftPanel();

        QWidget *createFileListToolBar();

        void onFileSelected(const QString &filePath);

        void onRemoveFileRequested(const QString &filePath);

        void onFunctionEntryClicked(const QString &functionName);
        
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

        void setupConnections();

        void updateFileHistory(const QString &filePath);

        bool eventFilter(QObject *obj, QEvent *event) override;

        QPoint dragPosition;
        bool isDragging = false;
        MainWindowMenuBar *m_menuBar;
        QTabWidget *tabWidget;
        QWidget *centerWidget;
        QVBoxLayout *mainLayout;
        QWidget *titleBar;
        QSplitter *mainSplitter;
        QSplitter *rightSplitter;

        StatusBar *statusBar{nullptr};

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
