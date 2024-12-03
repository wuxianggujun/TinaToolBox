#ifndef TINA_TOOL_BOX_PDF_VIEWER_HPP
#define TINA_TOOL_BOX_PDF_VIEWER_HPP

#include <QWidget>
#include <QScrollArea>
#include <memory>
#include <fpdfview.h>
#include <fpdf_formfill.h>
#include <fpdf_doc.h>
#include <fpdf_text.h>
#include <cmath>
#include <mutex>
#include <spdlog/spdlog.h>

class QLabel;
class QPushButton;
class QSpinBox;




class PdfViewer : public QWidget {
    Q_OBJECT

public:
    struct PdfInfo {
        int pageCount;
        int searchablePages;
        bool hasText;
        QString creator;
        QString producer;
    };

    class PDFiumLibrary
    {
    public:
        static void Initialize()
        {
            if (!initialized_)
            {
                FPDF_InitLibrary();
                initialized_ = true;
            }

        }

        static void Destroy() {
            if (initialized_) {
                FPDF_DestroyLibrary();
                initialized_ = false;
            }
        }
    private:
        static bool initialized_;
    };

    explicit PdfViewer(QWidget *parent = nullptr);

    ~PdfViewer() override;

    bool loadDocument(const QString &filePath);
    bool loadPage(int pageIndex);

    void closeDocument();
    bool isPageSearchable(int pageIndex);

    bool searchText(const QString &text, bool matchCase = false, bool wholeWord = false);

    void clearSearch();

    bool findNext();
    PdfInfo getPdfInfo();

    bool findPrevious();

private slots:
    void previousPage();

    void nextPage();

    void pageNumberChanged(int pageNum);

    void zoomIn();

    void zoomOut();

    void renderPage();
    void loadAndRenderPage();

private:

    void setupUI();

    void updatePageInfo() const;

    QLabel *pageLabel_{};
    QScrollArea *scrollArea_{};
    QPushButton *prevButton_{};
    QPushButton *nextButton_{};
    QSpinBox *pageSpinBox_{};
    QPushButton *zoomInButton_{};
    QPushButton *zoomOutButton_{};

    FPDF_DOCUMENT document_{nullptr};
    FPDF_FORMHANDLE formHandle_{nullptr};
    FPDF_FORMFILLINFO formFillInfo_{};
    FPDF_PAGE currentPage_{ nullptr };

    int currentPageIndex_{ 0 }; 
    int pageCount_{ 0 };        
    double zoomFactor_{ 1.0 };  

    struct SearchResult {
        int pageIndex;
        QRectF rect;
        double fontSize;
    };

    QString searchText_;
    std::vector<SearchResult> searchResults_;
    size_t currentSearchIndex_{0};


    void updateUIState(); 

    void highlightSearchResults(QPainter &painter, const QRectF &rect);

    void renderSearchResults(QImage &image);

    bool isCleaningUp_ = false;
    std::mutex cleanupMutex_;
};



#endif //TINA_TOOL_BOX_PDF_VIEWER_HPP
