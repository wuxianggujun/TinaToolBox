#include "SettingsPanel.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QStandardPaths>
#include <QDir>
#include <spdlog/spdlog.h>

SettingsPanel::SettingsPanel(QWidget *parent):QWidget(parent) {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.mkpath(".")) {  // 创建所有必需的父目录
        spdlog::error("Failed to create data directory: {}", appDataPath.toStdString());
        return;
    }
    configPath_ = dir.filePath("settings.ini");
    setupUI();
    loadSettings();
}

SettingsPanel::~SettingsPanel() {
    saveSettings();
}

void SettingsPanel::loadSettings() {
    ini_config_.SetUnicode();
    ini_config_.LoadFile(configPath_.toStdString().c_str());
}

void SettingsPanel::saveSettings() {
    // 保存UI值到配置文件
    // TODO: 实现具体的设置保存逻辑
    ini_config_.SaveFile(configPath_.toStdString().c_str());
}

void SettingsPanel::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    splitter_ = new QSplitter(Qt::Horizontal);

    settingsTree_ = new QTreeWidget();
    settingsTree_->setHeaderHidden(true);
    settingsTree_->setMinimumHeight(200);
    settingsTree_->setStyleSheet(R"(
        QTreeWidget {
            border: none;
            background-color: #f3f3f3;
        }
        QTreeWidget::item {
            height: 30px;
            padding-left: 10px;
        }
        QTreeWidget::item:hover {
            background-color: #e5e5e5;
        }
        QTreeWidget::item:selected {
            background-color: #ffffff;
        }
    )");

    // 创建右侧设置页面
    settingsPages_ = new QStackedWidget;
    settingsPages_->setStyleSheet(R"(
        QStackedWidget {
            background-color: white;
        }
    )");
    
    splitter_->addWidget(settingsTree_);
    splitter_->addWidget(settingsPages_);
    
    mainLayout->addWidget(splitter_);

    createSettingsTree();
    createSettingsPages();

    connect(settingsTree_,&QTreeWidget::currentItemChanged,this,&SettingsPanel::onSettingsTreeItemChanged);
}

void SettingsPanel::createSettingsTree() {
    auto* generalItem = new QTreeWidgetItem(settingsTree_,{"常规"});
    auto* editorItem = new QTreeWidgetItem(settingsTree_,{"编辑器"});
    auto* themeItem = new QTreeWidgetItem(settingsTree_,{"主题"});

    settingsTree_->addTopLevelItem(generalItem);
    settingsTree_->addTopLevelItem(editorItem);
    settingsTree_->addTopLevelItem(themeItem);

    settingsTree_->setCurrentItem(generalItem);
}

void SettingsPanel::createSettingsPages() {
    settingsPages_->addWidget(createGeneralSettingsPage());
    settingsPages_->addWidget(createEditorSettingsPage());
    settingsPages_->addWidget(createThemeSettingsPage());
}

QWidget * SettingsPanel::createGeneralSettingsPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    // 创建常规设置组
    auto* generalGroup = new QGroupBox("常规设置");
    auto* generalLayout = new QVBoxLayout(generalGroup);

    auto* autoSaveCheck = new QCheckBox("自动保存");
    auto* showLineNumbersCheck = new QCheckBox("显示行号");
    
    generalLayout->addWidget(autoSaveCheck);
    generalLayout->addWidget(showLineNumbersCheck);
    generalLayout->addStretch();

    layout->addWidget(generalGroup);
    layout->addStretch();

    return page;
}

QWidget* SettingsPanel::createEditorSettingsPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    // 创建编辑器设置组
    auto* editorGroup = new QGroupBox("编辑器设置");
    auto* editorLayout = new QVBoxLayout(editorGroup);

    // 字体大小设置
    auto* fontSizeLayout = new QHBoxLayout;
    auto* fontSizeLabel = new QLabel("字体大小:");
    auto* fontSizeSpinner = new QSpinBox;
    fontSizeSpinner->setRange(8, 72);
    fontSizeSpinner->setValue(12);
    fontSizeLayout->addWidget(fontSizeLabel);
    fontSizeLayout->addWidget(fontSizeSpinner);
    fontSizeLayout->addStretch();

    editorLayout->addLayout(fontSizeLayout);
    editorLayout->addStretch();

    layout->addWidget(editorGroup);
    layout->addStretch();

    return page;
}

QWidget * SettingsPanel::createThemeSettingsPage() {
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(20, 20, 20, 20);

    // 创建主题设置组
    auto* themeGroup = new QGroupBox("主题设置");

    layout->addWidget(themeGroup);
    layout->addStretch();

    return page;
}

void SettingsPanel::onSettingsTreeItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    if (!current) return;
    
    int index = settingsTree_->indexOfTopLevelItem(current);
    if (index >= 0) {
        settingsPages_->setCurrentIndex(index);
    }
}
