#ifndef TINA_TOOL_BOX_SETTINGS_PANEL_HPP
#define TINA_TOOL_BOX_SETTINGS_PANEL_HPP

#include <QWidget>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QSplitter>
#include <SimpleIni.h>

class SettingsPanel : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPanel(QWidget *parent = nullptr);
    ~SettingsPanel() override;

    void loadSettings();
    void saveSettings();

private:
    void setupUI();
    void createSettingsTree();
    void createSettingsPages();
    void loadConfigFile();
    void saveConfigFile();


    QTreeWidget* settingsTree_;
    QStackedWidget* settingsPages_;
    QSplitter* splitter_;
    CSimpleIniA ini_config_;
    QString configPath_;

    QWidget* createGeneralSettingsPage();
    QWidget* createEditorSettingsPage();
    QWidget* createThemeSettingsPage();
    
    void onSettingsTreeItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
};



#endif // TINA_TOOL_BOX_SETTINGS_PANEL_HPP