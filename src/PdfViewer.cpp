#include "PdfViewer.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QFileInfo>
#include <QMessageBox>
#include <QPainter>

#include "spdlog/spdlog.h"

PdfViewer::PdfViewer(QWidget *parent)
    : QWidget(parent) {
    setupUI();
}

PdfViewer::~PdfViewer() {
    if (formHandle_) {
        FPDFDOC_ExitFormFillEnvironment(formHandle_);
    }
    if (document_) {
        FPDF_CloseDocument(document_);
    }
    FPDF_DestroyLibrary();
};

void PdfViewer::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);

    // 控制栏
    auto *controlLayout = new QHBoxLayout();

    prevButton_ = new QPushButton("上一页", this);
    nextButton_ = new QPushButton("下一页", this);
    pageSpinBox_ = new QSpinBox(this);
    // 设置 SpinBox 的样式，隐藏上下箭头
    pageSpinBox_->setButtonSymbols(QAbstractSpinBox::NoButtons);
    pageSpinBox_->setAlignment(Qt::AlignCenter);
    // 设置最小宽度以显示完整页码
    pageSpinBox_->setMinimumWidth(60);

    zoomInButton_ = new QPushButton("+", this);
    zoomOutButton_ = new QPushButton("-", this);

    controlLayout->addWidget(prevButton_);
    controlLayout->addWidget(pageSpinBox_);
    controlLayout->addWidget(nextButton_);
    controlLayout->addStretch();
    controlLayout->addWidget(zoomOutButton_);
    controlLayout->addWidget(zoomInButton_);

    mainLayout->addLayout(controlLayout);

    // 滚动区域
    scrollArea_ = new QScrollArea(this);
    pageLabel_ = new QLabel(this);
    pageLabel_->setAlignment(Qt::AlignCenter);
    scrollArea_->setWidget(pageLabel_);
    scrollArea_->setWidgetResizable(true);

    mainLayout->addWidget(scrollArea_);

    // 连接信号和槽
    connect(prevButton_, &QPushButton::clicked, this, &PdfViewer::previousPage);
    connect(nextButton_, &QPushButton::clicked, this, &PdfViewer::nextPage);
    connect(pageSpinBox_, &QSpinBox::valueChanged, this, &PdfViewer::pageNumberChanged);
    connect(zoomInButton_, &QPushButton::clicked, this, &PdfViewer::zoomIn);
    connect(zoomOutButton_, &QPushButton::clicked, this, &PdfViewer::zoomOut);

    // 初始状态
    prevButton_->setEnabled(false);
    nextButton_->setEnabled(false);
    pageSpinBox_->setEnabled(false);
    zoomInButton_->setEnabled(false);
    zoomOutButton_->setEnabled(false);
}

bool PdfViewer::loadDocument(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "错误", "文件不存在");
        spdlog::error("PDF文件不存在: {}", filePath.toStdString());
        return false;
    }

    try {
        FPDF_InitLibrary();

        document_ = FPDF_LoadDocument(filePath.toStdString().c_str(), nullptr);
        if (!document_) {
            QMessageBox::warning(this, "错误", "无法打开PDF文件");
            spdlog::error("无法加载PDF文件: {}", filePath.toStdString());
            return false;
        }

        FPDF_FORMFILLINFO form_callbacks = {0};
        form_callbacks.version = 2;
        formHandle_ = FPDFDOC_InitFormFillEnvironment(document_, &form_callbacks);

        pageCount_ = FPDF_GetPageCount(document_);

        // 更新UI状态
        currentPage_ = 0;
        pageSpinBox_->setRange(1, pageCount_);
        pageSpinBox_->setValue(1);
        prevButton_->setEnabled(true);
        nextButton_->setEnabled(pageCount_ > 1);
        pageSpinBox_->setEnabled(true);
        zoomInButton_->setEnabled(true);
        zoomOutButton_->setEnabled(true);

        renderPage();
        return true;
    } catch (const std::exception &e) {
        QMessageBox::warning(this, "错误", QString("加载PDF文件时发生错误: %1").arg(e.what()));
        spdlog::error("加载PDF文件时发生异常: {}", e.what());
        return false;
    } catch (...) {
        QMessageBox::warning(this, "错误", "加载PDF文件时发生未知错误");
        spdlog::error("加载PDF文件时发生未知异常");
        return false;
    }
}

void PdfViewer::previousPage() {
    if (currentPage_ > 0) {
        currentPage_--;
        pageSpinBox_->setValue(currentPage_ + 1);
        renderPage();
    }
}

void PdfViewer::nextPage() {
    if (currentPage_ < pageCount_ - 1) {
        currentPage_++;
        pageSpinBox_->setValue(currentPage_ + 1);
        renderPage();
    }
}

void PdfViewer::pageNumberChanged(int pageNum) {
    if (pageNum >= 1 && pageNum <= pageCount_) {
        currentPage_ = pageNum - 1;
        renderPage();
    }
}

void PdfViewer::zoomIn() {
    if (zoomFactor_ < 5.0) {
        // 限制最大缩放为500%
        zoomFactor_ += 0.25; // 每次增加25%
        renderPage();
    }
}

void PdfViewer::zoomOut() {
    if (zoomFactor_ > 0.25) {
        // 限制最小缩放为25%
        zoomFactor_ -= 0.25; // 每次减少25%
        renderPage();
    }
}

void PdfViewer::renderPage() {
    if (!document_ || currentPage_ < 0 || currentPage_ >= pageCount_) {
        return;
    }

    // 加载页面
    FPDF_PAGE page = FPDF_LoadPage(document_, currentPage_);
    if (!page) {
        return;
    }

    // 获取页面尺寸
    double pageWidth = FPDF_GetPageWidth(page);
    double pageHeight = FPDF_GetPageHeight(page);

    // 使用更高的DPI来提高清晰度（默认72DPI提高到144或更高）
    const double kDPI = 144.0;
    const double kScale = kDPI / 72.0;

    // 计算缩放后的尺寸
    int scaledWidth = static_cast<int>(pageWidth * kScale * zoomFactor_);
    int scaledHeight = static_cast<int>(pageHeight * kScale * zoomFactor_);

    // 创建QImage
    QImage image(scaledWidth, scaledHeight, QImage::Format_RGBA8888);
    image.fill(Qt::white);

    // 设置图像DPI
    image.setDotsPerMeterX(static_cast<int>(kDPI * 39.37));
    image.setDotsPerMeterY(static_cast<int>(kDPI * 39.37));
    
    // 创建FPDF位图
    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(scaledWidth, scaledHeight,
                                             FPDFBitmap_BGRA, image.bits(), image.bytesPerLine());

    // 使用高质量渲染标志
    const int flags = FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH;
    // 渲染页面
    FPDF_RenderPageBitmap(bitmap, page, 0, 0, scaledWidth, scaledHeight, 0, flags);

    if (formHandle_) {
        FPDF_FFLDraw(formHandle_, bitmap, page, 0, 0, scaledWidth, scaledHeight, 0, FPDF_ANNOT);
    }

    // 显示图像
    pageLabel_->setPixmap(QPixmap::fromImage(std::move(image)));

    // 清理
    FPDFBitmap_Destroy(bitmap);
    FPDF_ClosePage(page);

    // 更新页码信息
    updatePageInfo();
}


void PdfViewer::updatePageInfo() const {
    prevButton_->setEnabled(currentPage_ > 0);
    nextButton_->setEnabled(currentPage_ < pageCount_ - 1);
}
