#ifndef TINA_TOOL_BOX_PDF_VIEWER_HPP
#define TINA_TOOL_BOX_PDF_VIEWER_HPP

#include <QWidget>
#include <QScrollArea>
#include <memory>
#include <fpdfview.h>
#include <fpdf_formfill.h>

class QLabel;
class QPushButton;
class QSpinBox;

class PdfViewer : public QWidget {
    Q_OBJECT

public:
    explicit PdfViewer(QWidget *parent = nullptr);

    ~PdfViewer() override;

    bool loadDocument(const QString &filePath);

private slots:
    void previousPage();

    void nextPage();

    void pageNumberChanged(int pageNum);

    void zoomIn();

    void zoomOut();

    void renderPage();

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
    int pageCount_{0};

    int currentPage_{0};
    double zoomFactor_{1.0};
};

#endif //TINA_TOOL_BOX_PDF_VIEWER_HPP
