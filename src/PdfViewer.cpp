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

#include "ExceptionHandler.hpp"
#include "spdlog/spdlog.h"

bool PdfViewer::PDFiumLibrary::initialized_ = false;

PdfViewer::PdfViewer(QWidget *parent)
    : QWidget(parent)
    , document_(nullptr)
    , formHandle_(nullptr)
    , currentPage_(nullptr)
    , currentPageIndex_(0)
    , pageCount_(0)
    , zoomFactor_(1.0) {
    PDFiumLibrary::Initialize();
    setupUI();
}

PdfViewer::~PdfViewer() {
    std::lock_guard<std::mutex> lock(cleanupMutex_);
    if (!isCleaningUp_) {
        closeDocument();
    }
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

    updateUIState();
}

void PdfViewer::updatePageInfo() const
{
}

bool PdfViewer::loadDocument(const QString &filePath) {
    closeDocument();  // 确保先清理旧文档

    ExceptionHandler handler("加载PDF文档失败");
    return handler([this, &filePath]() {
        // 1. 加载文档
        document_ = FPDF_LoadDocument(filePath.toStdString().c_str(), nullptr);
        if (!document_) {
            spdlog::error("无法加载PDF文件: {}", filePath.toStdString());
            return false;
        }

        // 2. 初始化form环境
        formFillInfo_.version = 2;
        formHandle_ = FPDFDOC_InitFormFillEnvironment(document_, &formFillInfo_);
        if (!formHandle_) {
            spdlog::error("初始化form环境失败");
            FPDF_CloseDocument(document_);
            document_ = nullptr;
            return false;
        }

        // 执行文档打开动作
        FORM_DoDocumentJSAction(formHandle_);
        FORM_DoDocumentAAction(formHandle_, FPDFDOC_AACTION_WC);

        // 4. 获取页数和其他初始化
        pageCount_ = FPDF_GetPageCount(document_);
        currentPageIndex_ = 0;  // 设置初始页为第一页

        // 启用UI控件
        if (pageSpinBox_) {
            pageSpinBox_->setRange(1, pageCount_);
            pageSpinBox_->setValue(1);
            pageSpinBox_->setEnabled(true);
        }

        // 更新导航按钮状态
        updateUIState();

        // 渲染第一页
        loadAndRenderPage();

        return true;
        });
}

bool PdfViewer::loadPage(int pageIndex) {
    if (!document_ || pageIndex < 0 || pageIndex >= pageCount_) {
        return false;
    }

    // 关闭当前页面
    if (currentPage_) {
        FORM_DoPageAAction(currentPage_, formHandle_, FPDFPAGE_AACTION_CLOSE);
        FORM_OnBeforeClosePage(currentPage_, formHandle_);
        FPDF_ClosePage(currentPage_);
        currentPage_ = nullptr;
    }

    // 加载新页面
    currentPage_ = FPDF_LoadPage(document_, pageIndex);
    if (!currentPage_) {
        return false;
    }

    // 执行页面相关的表单操作
    FORM_OnAfterLoadPage(currentPage_, formHandle_);
    FORM_DoPageAAction(currentPage_, formHandle_, FPDFPAGE_AACTION_OPEN);

    return true;
}

void PdfViewer::closeDocument() {
    std::lock_guard<std::mutex> lock(cleanupMutex_);
    if (isCleaningUp_) {
        return;
    }
    isCleaningUp_ = true;

    ExceptionHandler handler("清理PDF文档资源失败");
    handler([this]() {
        // 重置UI状态
        if (pageLabel_) {
            pageLabel_->clear();
        }

        if (pageSpinBox_) {
            pageSpinBox_->setValue(0);
            pageSpinBox_->setEnabled(false);
        }
        if (prevButton_) prevButton_->setEnabled(false);
        if (nextButton_) nextButton_->setEnabled(false);
        if (zoomInButton_) zoomInButton_->setEnabled(false);
        if (zoomOutButton_) zoomOutButton_->setEnabled(false);

        // 按照正确的顺序清理PDFium资源
        if (currentPage_) {
            if (formHandle_) {
                FORM_DoPageAAction(currentPage_, formHandle_, FPDFPAGE_AACTION_CLOSE);
                FORM_OnBeforeClosePage(currentPage_, formHandle_);
            }
            FPDF_ClosePage(currentPage_);
            currentPage_ = nullptr;
        }

        if (formHandle_) {
            FORM_DoDocumentAAction(formHandle_, FPDFDOC_AACTION_WC);
            FPDFDOC_ExitFormFillEnvironment(formHandle_);
            formHandle_ = nullptr;
        }

        if (document_) {
            FPDF_CloseDocument(document_);
            document_ = nullptr;
        }

        // 重置其他状态
        pageCount_ = 0;
        currentPageIndex_ = 0;
        zoomFactor_ = 1.0;
        searchResults_.clear();
        currentSearchIndex_ = 0;
        searchText_.clear();

        return true;
        });
}

bool PdfViewer::isPageSearchable(int pageIndex) {
    if (!document_ || pageIndex < 0 || pageIndex >= pageCount_) {
        return false;
    }

    FPDF_PAGE page = FPDF_LoadPage(document_, pageIndex);
    if (!page) return false;

    FPDF_TEXTPAGE textPage = FPDFText_LoadPage(page);
    if (!textPage) {
        FPDF_ClosePage(page);
        return false;
    }

    // 获取页面文本字符数
    int charCount = FPDFText_CountChars(textPage);

    FPDFText_ClosePage(textPage);
    FPDF_ClosePage(page);

    return charCount > 0;
}

bool PdfViewer::searchText(const QString &text, bool matchCase, bool wholeWord) {
    if (!document_ || text.isEmpty()) {
        return false;
    }

    bool hasSearchablePages = false;

    for (int i= 0 ;i<pageCount_;++i)
    {
	    if (isPageSearchable(i))
	    {
            hasSearchablePages = true;
            break;
	    }
    }

    if (!hasSearchablePages) {
        QMessageBox::warning(this, "搜索提示",
            "当前PDF文档不支持搜索，可能是扫描件或图片型PDF。\n"
            "建议使用OCR软件处理后再搜索。");
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
                    left = std::min(left, charLeft);
                    right = std::max(right, charRight);

                    // 使用字体大小来确定合理的高度
                    double fontSize = FPDFText_GetFontSize(textPage, charIndex);
                    if (fontSize > 0) {
                        avgFontSize += fontSize;
                        validCharCount++;

                        // 根据字体大小调整上下边界
                        double centerY = (charTop + charBottom) / 2;
                        double halfHeight = fontSize * 0.7; // 可以调整这个系数
                        top = std::min(top, centerY - halfHeight);
                        bottom = std::max(bottom, centerY + halfHeight);
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


    if (searchResults_.empty()) {
        QMessageBox::information(this, "搜索结果",
            "未找到匹配的内容。");
        return false;
    }
        // 跳转到第一个搜索结果所在的页面
        currentPageIndex_ = searchResults_[0].pageIndex;
        loadAndRenderPage();
        return true;
 
}

bool PdfViewer::findNext() {
    if (searchResults_.empty()) return false;

    currentSearchIndex_ = (currentSearchIndex_ + 1) % searchResults_.size();
    const auto &result = searchResults_[currentSearchIndex_];
    if (result.pageIndex != currentPageIndex_)
    {
        currentPageIndex_ = result.pageIndex;
        loadAndRenderPage();
    }
    else
    {
        renderPage();
    }
    return true;
}

PdfViewer::PdfInfo PdfViewer::getPdfInfo() {
    PdfInfo info = { 0, 0, false, "", "" };

    if (!document_) return info;

    info.pageCount = pageCount_;

    // 检查可搜索页面
    for (int i = 0; i < pageCount_; ++i) {
        if (isPageSearchable(i)) {
            info.searchablePages++;
        }
    }

    info.hasText = (info.searchablePages > 0);

    // 获取文档元数据
    unsigned long bufferLen = 256;
    std::vector<char> buffer(bufferLen);

    if (FPDF_GetMetaText(document_, "Creator", buffer.data(), bufferLen)) {
        info.creator = QString::fromUtf8(buffer.data());
    }

    if (FPDF_GetMetaText(document_, "Producer", buffer.data(), bufferLen)) {
        info.producer = QString::fromUtf8(buffer.data());
    }

    return info;
}


bool PdfViewer::findPrevious() {
    if (searchResults_.empty()) return false;

    if (currentSearchIndex_ == 0)
        currentSearchIndex_ = searchResults_.size() - 1;
    else
        currentSearchIndex_--;

    const auto &result = searchResults_[currentSearchIndex_];
    // 如果结果在不同页面，需要切换页面
    if (result.pageIndex != currentPageIndex_) {
        currentPageIndex_ = result.pageIndex;
        loadAndRenderPage();
    }
    else {
        // 如果在同一页面，只需要重新渲染
        renderPage();
    }

    return true;
}

void PdfViewer::clearSearch() {
    searchResults_.clear();
    currentSearchIndex_ = 0;
    searchText_.clear();
    renderPage();
}


void PdfViewer::renderSearchResults(QImage &image) {
    if (searchResults_.empty()) return;

    // 使用当前已加载的页面，而不是重新加载
    if (!currentPage_) return;

    double pageWidth = FPDF_GetPageWidth(currentPage_);
    double pageHeight = FPDF_GetPageHeight(currentPage_);

    double scaleX = (image.width() / pageWidth);
    double scaleY = (image.height() / pageHeight);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    for (size_t i = 0; i < searchResults_.size(); ++i) {
        const auto &result = searchResults_[i];
        if (result.pageIndex == currentPageIndex_) {
            // 计算正确的Y坐标（PDF坐标系从底部开始，Qt从顶部开始）
            double y = pageHeight - result.rect.bottom(); // 翻转Y坐标

            QRectF scaledRect(
                result.rect.x() * scaleX,
                y * scaleY, // 使用翻转后的Y坐标
                result.rect.width() * scaleX,
                result.rect.height() * scaleY // 高度保持正值
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





void PdfViewer::zoomOut()
{

}

void PdfViewer::renderPage() {
    if (!document_ || !formHandle_) {
        return;
    }

    // 加载当前页面（如果还没加载）
    if (!loadPage(currentPageIndex_)) {
        return;
    }

    // 获取页面尺寸
    double pageWidth = FPDF_GetPageWidth(currentPage_);
    double pageHeight = FPDF_GetPageHeight(currentPage_);

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
    FPDF_RenderPageBitmap(bitmap, currentPage_, 0, 0, scaledWidth, scaledHeight, 0, flags);

    if (formHandle_) {
        FPDF_FFLDraw(formHandle_, bitmap, currentPage_, 0, 0, scaledWidth, scaledHeight, 0, FPDF_ANNOT);
    }


    // 渲染搜索结果
    if (!searchResults_.empty()) {
        renderSearchResults(image);
    }

    // 显示图像
    pageLabel_->setPixmap(QPixmap::fromImage(std::move(image)));

    // 清理
    FPDFBitmap_Destroy(bitmap);

    // 更新页码信息
    updatePageInfo();
}


// 新增：统一处理页面加载和渲染的函数
void PdfViewer::loadAndRenderPage() {
    if (!document_ || currentPageIndex_ < 0 || currentPageIndex_ >= pageCount_) {
        return;
    }

    ExceptionHandler handler("页面加载失败");
    handler([this]() {
        // 加载新页面
        if (!loadPage(currentPageIndex_)) {
            spdlog::error("加载页面失败：页码 {}", currentPageIndex_ + 1);
            return false;
        }

        // 获取页面尺寸
        double pageWidth = FPDF_GetPageWidth(currentPage_);
        double pageHeight = FPDF_GetPageHeight(currentPage_);

        // 使用更高的DPI来提高清晰度
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
            FPDFBitmap_BGRA,
            image.bits(),
            image.bytesPerLine());

        // 使用高质量渲染标志
        const int flags = FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH;

        // 渲染页面
        FPDF_RenderPageBitmap(bitmap, currentPage_, 0, 0, scaledWidth, scaledHeight, 0, flags);

        // 渲染表单
        if (formHandle_) {
            FPDF_FFLDraw(formHandle_, bitmap, currentPage_, 0, 0, scaledWidth, scaledHeight, 0, flags);
        }

        // 渲染搜索结果（如果有）
        if (!searchResults_.empty()) {
            renderSearchResults(image);
        }

        // 显示图像
        pageLabel_->setPixmap(QPixmap::fromImage(std::move(image)));

        // 清理位图
        FPDFBitmap_Destroy(bitmap);

        return true;
        });
}

void PdfViewer::previousPage() {
    if (currentPageIndex_ > 0) {
        currentPageIndex_--;
        loadAndRenderPage();
        updateUIState();
    }
}

void PdfViewer::nextPage() {
    if (currentPageIndex_ < pageCount_ - 1) {
        currentPageIndex_++;
        loadAndRenderPage();
        updateUIState();
    }
}

void PdfViewer::pageNumberChanged(int pageNum) {
    if (pageNum >= 1 && pageNum <= pageCount_) {
        currentPageIndex_ = pageNum - 1;
        loadAndRenderPage();
        updateUIState();
    }
}


void PdfViewer::zoomIn()
{
}

void PdfViewer::updateUIState() {
    if (!document_) {
        // 如果没有文档，禁用所有控件
        if (prevButton_) prevButton_->setEnabled(false);
        if (nextButton_) nextButton_->setEnabled(false);
        if (pageSpinBox_) pageSpinBox_->setEnabled(false);
        if (zoomInButton_) zoomInButton_->setEnabled(false);
        if (zoomOutButton_) zoomOutButton_->setEnabled(false);
        return;
    }

    // 更新页码显示
    if (pageSpinBox_) {
        pageSpinBox_->setRange(1, pageCount_);
        pageSpinBox_->setValue(currentPageIndex_ + 1);
        pageSpinBox_->setEnabled(true);
    }

    // 更新导航按钮状态
    if (prevButton_) {
        prevButton_->setEnabled(currentPageIndex_ > 0);
        spdlog::debug("上一页按钮状态: {}", currentPageIndex_ > 0);
    }

    if (nextButton_) {
        nextButton_->setEnabled(currentPageIndex_ < pageCount_ - 1);
        spdlog::debug("下一页按钮状态: {}", currentPageIndex_ < pageCount_ - 1);
    }

    // 更新缩放按钮状态
    if (zoomInButton_) zoomInButton_->setEnabled(true);
    if (zoomOutButton_) zoomOutButton_->setEnabled(true);

    spdlog::debug("当前页码: {}, 总页数: {}", currentPageIndex_ + 1, pageCount_);
}