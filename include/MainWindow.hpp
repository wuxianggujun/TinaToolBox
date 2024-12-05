#ifndef TINA_TOOL_BOX_MAINWINDOW_HPP
#define TINA_TOOL_BOX_MAINWINDOW_HPP

#include "PythonScriptManager.hpp"
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
#include "DocumentHandler.hpp"
#include "MainWindowMenuBar.hpp"
#include "Tokenizer.hpp"


class LogPanel;
class DocumentArea;

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

private slots:
    void handleMenuAction(const QString &actionName);

    void toggleMaximize();

    void showBottomPanel();

    void hideBottomPanel();

    void showFilePathToolTip(QTreeWidgetItem *item, int column);

    void showFileTreeContextMenu(const QPoint &pos);

    void onFileDoubleClicked(const QTreeWidgetItem* item);

    void onRunButtonStateChanged(bool isRunning);
    
private:
    bool isTitleBarArea(const QPoint& pos) const;

    void setUpUI();

    void createTileBar();

    void loadFileHistory();

    void openFile();

   void updateUIState();

    void setupConnections();

    void saveFileHistory(const QString &filePath);

    void updateFileHistory(const QString &filePath);

    void updateFileTree();

    bool eventFilter(QObject *obj, QEvent *event) override;
    
private:
    QPoint dragPosition;
    bool isDragging = false;
    MainWindowMenuBar *m_menuBar;
    QTabWidget *tabWidget;
    QWidget *centerWidget;
    QVBoxLayout *mainLayout;
    QWidget *titleBar;
    QSplitter *mainSplitter;
    QSplitter *rightSplitter;

    QTabWidget *leftPanelTab;
    QTreeWidget *fileTree;
    QTreeWidget *scriptTree;
    
    DocumentArea *documentArea;
    QStackedWidget *propertyStack;
    QWidget *bottomPanel;
    LogPanel *logPanel;
    QPushButton *maxButton;
    QSplitter *bottomSplitter;

    // PythonScriptManager manager_;

};

#endif // TINA_TOOL_BOX_MAINWINDOW_HPP
