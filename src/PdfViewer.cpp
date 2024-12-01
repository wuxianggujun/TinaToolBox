#include "PdfViewer.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QFileInfo>
#include <QLineEdit>
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
    auto *searchEdit = new QLineEdit(this);
    auto *searchButton = new QPushButton("搜索", this);
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

    controlLayout->addWidget(searchEdit);
    controlLayout->addWidget(searchButton);

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

    connect(searchButton, &QPushButton::clicked, [this, searchEdit]() {
        this->searchText(searchEdit->text());
    });

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

bool PdfViewer::searchText(const QString& text, bool matchCase, bool wholeWord) {
    if (!document_ || text.isEmpty()) {
        return false;
    }

    searchText_ = text;
    searchResults_.clear();
    currentSearchIndex_ = 0;

    for (int pageIndex = 0; pageIndex < pageCount_; ++pageIndex) {
        FPDF_PAGE page = FPDF_LoadPage(document_, pageIndex);
        if (!page) continue;

        FPDF_TEXTPAGE textPage = FPDFText_LoadPage(page);
        if (!textPage) {
            FPDF_ClosePage(page);
            continue;
        }

        std::wstring wtext = text.toStdWString();
        unsigned long flags = 0;
        if (matchCase) flags |= FPDF_MATCHCASE;
        if (wholeWord) flags |= FPDF_MATCHWHOLEWORD;

        FPDF_SCHHANDLE search = FPDFText_FindStart(textPage, 
            reinterpret_cast<FPDF_WIDESTRING>(wtext.c_str()), flags, 0);

        while (FPDFText_FindNext(search)) {
            SearchResult result;
            result.pageIndex = pageIndex;
            
            int startIndex = FPDFText_GetSchResultIndex(search);
            int count = FPDFText_GetSchCount(search);

            // 获取每个字符的边界框并合并
            double left = DBL_MAX, top = DBL_MAX;
            double right = -DBL_MAX, bottom = -DBL_MAX;
            double avgFontSize = 0;
            int validCharCount = 0;

            for (int charIndex = startIndex; charIndex < startIndex + count; ++charIndex) {
                double charLeft, charTop, charRight, charBottom;
                if (FPDFText_GetCharBox(textPage, charIndex, &charLeft, &charRight, 
                                      &charBottom, &charTop)) {
                    left = min(left, charLeft);
                    right = max(right, charRight);
                    
                    // 使用字体大小来确定合理的高度
                    double fontSize = FPDFText_GetFontSize(textPage, charIndex);
                    if (fontSize > 0) {
                        avgFontSize += fontSize;
                        validCharCount++;
                        
                        // 根据字体大小调整上下边界
                        double centerY = (charTop + charBottom) / 2;
                        double halfHeight = fontSize * 0.7; // 可以调整这个系数
                        top = min(top, centerY - halfHeight);
                        bottom = max(bottom, centerY + halfHeight);
                    }
                }
            }

            if (left != DBL_MAX && validCharCount > 0) {
                avgFontSize /= validCharCount;
                result.rect = QRectF(left, top, right - left, bottom - top);
                result.fontSize = avgFontSize;
                searchResults_.push_back(result);
            }
        }

        FPDFText_FindClose(search);
        FPDFText_ClosePage(textPage);
        FPDF_ClosePage(page);
    }

    if (!searchResults_.empty()) {
        currentPage_ = searchResults_[0].pageIndex;
        pageSpinBox_->setValue(currentPage_ + 1);
        renderPage();
        return true;
    }

    return false;
}

bool PdfViewer::findNext() {
    if (searchResults_.empty()) return false;

    currentSearchIndex_ = (currentSearchIndex_ + 1) % searchResults_.size();
    const auto &result = searchResults_[currentSearchIndex_];

    if (result.pageIndex != currentPage_) {
        currentPage_ = result.pageIndex;
        pageSpinBox_->setValue(currentPage_ + 1);
    }
    renderPage();
    return true;
}


bool PdfViewer::findPrevious() {
    if (searchResults_.empty()) return false;

    if (currentSearchIndex_ == 0)
        currentSearchIndex_ = searchResults_.size() - 1;
    else
        currentSearchIndex_--;

    const auto &result = searchResults_[currentSearchIndex_];

    if (result.pageIndex != currentPage_) {
        currentPage_ = result.pageIndex;
        pageSpinBox_->setValue(currentPage_ + 1);
    }
    renderPage();
    return true;
}

void PdfViewer::clearSearch() {
    searchResults_.clear();
    currentSearchIndex_ = 0;
    searchText_.clear();
    renderPage();
}


void PdfViewer::renderSearchResults(QImage& image) {
    if (searchResults_.empty()) return;

    FPDF_PAGE page = FPDF_LoadPage(document_, currentPage_);
    if (!page) return;

    double pageWidth = FPDF_GetPageWidth(page);
    double pageHeight = FPDF_GetPageHeight(page);
    FPDF_ClosePage(page);

    double scaleX = (image.width() / pageWidth);
    double scaleY = (image.height() / pageHeight);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    for (size_t i = 0; i < searchResults_.size(); ++i) {
        const auto& result = searchResults_[i];
        if (result.pageIndex == currentPage_) {
            // 计算正确的Y坐标（PDF坐标系从底部开始，Qt从顶部开始）
            double y = pageHeight - result.rect.bottom(); // 翻转Y坐标
            
            QRectF scaledRect(
                result.rect.x() * scaleX,
                y * scaleY,  // 使用翻转后的Y坐标
                result.rect.width() * scaleX,
                result.rect.height() * scaleY  // 高度保持正值
            );

            QColor highlightColor = (i == currentSearchIndex_) 
                ? QColor(255, 255, 0, 127)
                : QColor(255, 200, 0, 80);
            
            painter.fillRect(scaledRect, highlightColor);
            painter.setPen(QPen(Qt::darkYellow, 0.5));
            painter.drawRect(scaledRect);
        }
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

    renderSearchResults(image);

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
