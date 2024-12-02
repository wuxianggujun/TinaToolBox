#include "LineNumberTextEdit.hpp"
#include <QPainter>
#include <QTextBlock>

#include "MergedTableView.hpp"

const int MIN_FONT_SIZE = 8;
const int MAX_FONT_SIZE = 24;

LineNumberTextEdit::LineNumberTextEdit(QWidget *parent) : QPlainTextEdit(parent) {
    lineNumberArea = new LineNumberArea(this);

    connect(this, &LineNumberTextEdit::blockCountChanged,
            this, &LineNumberTextEdit::updateLineNumberAreaWidth);
    connect(this, &LineNumberTextEdit::updateRequest,
            this, &LineNumberTextEdit::updateLineNumberArea);
    connect(this, &LineNumberTextEdit::cursorPositionChanged,
            this, &LineNumberTextEdit::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    // 启用双缓冲绘制
    setAttribute(Qt::WA_PaintOnScreen, false);
    setAttribute(Qt::WA_NoSystemBackground, false);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent, false);
    
    // 设置更新模式
    setAutoFillBackground(true);
    viewport()->setAutoFillBackground(true);

    // 优化文本渲染
    setCenterOnScroll(true);
    setWordWrapMode(QTextOption::NoWrap);

    // 设置缓冲区大小
    document()->setMaximumBlockCount(INT_MAX);

    // 优化选择和光标
    setMouseTracking(true);
    setTextInteractionFlags(Qt::TextEditorInteraction);

    // 设置基本样式
    setStyleSheet(R"(
       QPlainTextEdit {
           background-color: #1e1e1e;
           color: #d4d4d4;
           border: none;
           font-family: Consolas, 'Courier New', monospace;
           font-size: 12px;
           selection-background-color: #264f78;
       }
   )");
}

int LineNumberTextEdit::lineNumberAreaWidth() {
    int digits = 4;
    int max = qMax(1, blockCount());
    while (max >= 10000) {
        max /= 10;
        ++digits;
    }
    // 设置更合适的边距
    int leftPadding = 10; // 左边距
    int rightPadding = 10; // 右边距
    int digitWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));

    // 总宽度 = 左边距 + 数字宽度 * 位数 + 右边距
    int space = leftPadding + digitWidth * digits + rightPadding;
    return space;
}

void LineNumberTextEdit::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        // Ctrl + 滚轮实现缩放
        const int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn(1);
        } else {
            zoomOut(1);
        }
        event->accept();
        
        // 更新行号区域宽度，因为字体大小改变可能影响行号宽度
        updateLineNumberAreaWidth(0);
    } else {
        // 普通滚动
        QPlainTextEdit::wheelEvent(event);
    }
    
    // 确保更新视口
    viewport()->update();
}

void LineNumberTextEdit::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                    lineNumberAreaWidth(), cr.height()));
}

void LineNumberTextEdit::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void LineNumberTextEdit::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void LineNumberTextEdit::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        // 修改高亮颜色为深蓝色，但透明度较低
        QColor lineColor = QColor(0, 0, 139, 40); // 深蓝色，alpha=40
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void LineNumberTextEdit::lineNumberAreaPaintEvent(QPaintEvent *event) {
    QPainter painter(lineNumberArea);
    // 首先用背景色清除整个区域
    painter.fillRect(event->rect(), QColor("#ffffff"));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    // 计算文本绘制区域的左边距
    int leftPadding = 10;

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#999999"));

            // 使用新的绘制方式，考虑左右边距
            QRect numberRect(leftPadding, top,
                           lineNumberArea->width() - leftPadding - 10, // 10是右边距
                           fontMetrics().height());

            // 先清除这一行的区域
            painter.fillRect(QRect(0, top, lineNumberArea->width(), fontMetrics().height()), QColor("#ffffff"));
            // 绘制行号
            painter.drawText(numberRect, Qt::AlignRight | Qt::AlignVCenter, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

    // 在最右边绘制分隔线
    painter.setPen(QColor("#e0e0e0")); // 浅灰色的分隔线
    painter.drawLine(
        event->rect().topRight() - QPoint(1, 0),
        event->rect().bottomRight() - QPoint(1, 0)
    );
}

void LineNumberTextEdit::zoomIn(int range) {
    QFont f = font();
    const int newSize = f.pointSize() + range;
    if (newSize <= MAX_FONT_SIZE) {
        f.setPointSize(newSize);
        setFont(f);
    }
}

void LineNumberTextEdit::zoomOut(int range) {
    QFont f = font();
    const int newSize = f.pointSize() - range;
    if (newSize >= MIN_FONT_SIZE) {
        f.setPointSize(newSize);
        setFont(f);
    }
}
