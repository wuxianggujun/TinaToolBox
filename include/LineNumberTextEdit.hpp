#ifndef TINA_TOOL_BOX_LINE_NUMBER_TEXT_EDIT_HPP
#define TINA_TOOL_BOX_LINE_NUMBER_TEXT_EDIT_HPP

#include <QPlainTextEdit>
#include <QTextEdit>
#include <QPointer>
#include <QTextEdit>

#include <QPlainTextEdit>
class LineNumberArea;

class LineNumberTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit LineNumberTextEdit(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);

    int lineNumberAreaWidth();
protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);

    void updateLineNumberArea(const QRect &rect, int dy);

    void highlightCurrentLine();

private:
    QWidget *lineNumberArea;
    void zoomIn(int range);
    void zoomOut(int range);
    
    // 字体大小限制
    const int MIN_FONT_SIZE = 8;
    const int MAX_FONT_SIZE = 32;
};

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(LineNumberTextEdit *editor) : QWidget(editor), codeEditor(editor) {
    }

    [[nodiscard]] QSize sizeHint() const override {
        return {codeEditor->lineNumberAreaWidth(), 0};
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    LineNumberTextEdit *codeEditor;
};

#endif
