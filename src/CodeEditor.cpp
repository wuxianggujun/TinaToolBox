#include "CodeEditor.hpp"
#include <QPainter>
#include <QTextBlock>
#include <QMouseEvent>

namespace TinaToolBox {
    CodeEditor::CodeEditor(QWidget *parent): QPlainTextEdit(parent) {
        lineNumberArea = new CodeEditorLineArea(this);
        setupEditor();
        setupConnections();
    }

    CodeEditor::~CodeEditor() {
        delete lineNumberArea;
    }

    void CodeEditor::toggleBreakpoint(int line) {
        if (line <= 0 || line > document()->blockCount())
            return;

        if (breakpoints_.contains(line)) {
            breakpoints_.remove(line);
            emit breakpointToggled(line, false);
        } else {
            breakpoints_.insert(line);
            emit breakpointToggled(line, true);
        }
        lineNumberArea->update();
    }

    bool CodeEditor::hasBreakpoint(int line) const{
        return breakpoints_.contains(line);
    }

    QSet<int> CodeEditor::getBreakpoints() const {
        return breakpoints_;
    }

    void CodeEditor::clearBreakpoints() {
        breakpoints_.clear();
        lineNumberArea->update();
    }

    int CodeEditor::lineNumberAreaWidth() const {
        int digits = 1;
        int max = qMax(1, blockCount());
        while (max >= 10) {
            max /= 10;
            ++digits;
        }

        // 使用更大的字体计算宽度
        QFont lineNumberFont = font();
        lineNumberFont.setPointSize(12);
        QFontMetrics fm(lineNumberFont);
    
        int space = BREAKPOINT_MARGIN + LINE_NUMBER_PADDING + 
                    fm.horizontalAdvance(QLatin1Char('9')) * digits + 
                    LINE_NUMBER_PADDING;
        return space;
    }

    void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
        QPainter painter(lineNumberArea);
        painter.fillRect(event->rect(), QColor("#ffffff"));

        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        // 设置行号字体
        QFont lineNumberFont = font();
        lineNumberFont.setPointSize(12);  // 设置行号字体大小
        painter.setFont(lineNumberFont);
        
        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                // 绘制断点
                if (hasBreakpoint(blockNumber + 1)) {
                    QRect breakpointRect(2, top + 4, 12, 12);
                    painter.setPen(Qt::NoPen);
                    painter.setBrush(QColor("#ff4444"));
                    painter.drawEllipse(breakpointRect);
                }

                // 绘制行号
                QString number = QString::number(blockNumber + 1);
                painter.setPen(QColor("#999999"));
              
                // 计算行号文本的垂直中心位置
                int textHeight = fontMetrics().height();
                int verticalCenter = top + (blockBoundingRect(block).height() - textHeight) / 2;
            
                // 使用 QRect 确保文本垂直居中
                QRect numberRect(
                    BREAKPOINT_MARGIN + LINE_NUMBER_PADDING,
                    verticalCenter,
                    lineNumberArea->width() - BREAKPOINT_MARGIN - LINE_NUMBER_PADDING * 2,
                    textHeight
                );

                // 使用 AlignVCenter 确保文本垂直居中
                painter.drawText(numberRect, 
                               Qt::AlignRight | Qt::AlignVCenter, 
                               number);
            }

            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
            ++blockNumber;
        }

        // 绘制分隔线
        painter.setPen(QColor("#e0e0e0"));
        painter.drawLine(event->rect().topRight(), event->rect().bottomRight());
    }

    void CodeEditor::resizeEvent(QResizeEvent *event) {
        QPlainTextEdit::resizeEvent(event);
        QRect cr = contentsRect();
        lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                        lineNumberAreaWidth(), cr.height()));
        
    }

    void CodeEditor::wheelEvent(QWheelEvent *event) {
        if (event->modifiers() & Qt::ControlModifier) {
            const int delta = event->angleDelta().y();
            if (delta > 0)
                zoomIn(1);
            else
                zoomOut(1);
            event->accept();
        } else {
            QPlainTextEdit::wheelEvent(event);
        }
    }

    void CodeEditor::updateLineNumberAreaWidth(int newBlockCount) {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    }

    void CodeEditor::highlightCurrentLine() {
        QList<QTextEdit::ExtraSelection> extraSelections;

        if (!isReadOnly()) {
            QTextEdit::ExtraSelection selection;
            QColor lineColor = QColor(0, 0, 139, 40); // 深蓝色，alpha=40
            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }

        setExtraSelections(extraSelections);
    }

    void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
        if (dy)
            lineNumberArea->scroll(0, dy);
        else
            lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

        if (rect.contains(viewport()->rect()))
            updateLineNumberAreaWidth(0);
    }

    void CodeEditor::setupEditor() {
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);

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

        // 设置缓冲区文本大小
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

        // 设置更大的字体
        QFont editorFont = font();
        editorFont.setPointSize(12);  // 设置字体大小
        setFont(editorFont);
    }

    void CodeEditor::setupConnections() {
        connect(this,&CodeEditor::blockCountChanged,this,&CodeEditor::updateLineNumberAreaWidth);
        connect(this,&CodeEditor::updateRequest,this,&CodeEditor::updateLineNumberArea);
        connect(this,&CodeEditor::cursorPositionChanged,this,&CodeEditor::highlightCurrentLine);

        updateLineNumberAreaWidth(0);
        highlightCurrentLine();
    }

    void CodeEditor::zoomIn(int range) {
        QFont f = font();
        const int newSize = f.pointSize() + range;
        if (newSize <= MAX_FONT_SIZE) {
            f.setPointSize(newSize);
            setFont(f);
        }
    }

    void CodeEditor::zoomOut(int range) {
        QFont f = font();
        const int newSize = f.pointSize() - range;
        if (newSize >= MIN_FONT_SIZE) {
            f.setPointSize(newSize);
            setFont(f);
        }
    }

    // CodeEditorLineArea implementation
    CodeEditorLineArea::CodeEditorLineArea(CodeEditor *editor)
        : QWidget(editor), codeEditor(editor) {
        setCursor(Qt::PointingHandCursor);
        setMouseTracking(true);
    }

    QSize CodeEditorLineArea::sizeHint() const {
        return {codeEditor->lineNumberAreaWidth(), 0};
    }

    void CodeEditorLineArea::paintEvent(QPaintEvent *event) {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

    void CodeEditorLineArea::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton && isInBreakpointArea(event->pos())) {
            int line = lineNumberAtPos(event->pos());
            if (line > 0) {
                codeEditor->toggleBreakpoint(line);
            }
        }
    }

    void CodeEditorLineArea::mouseDoubleClickEvent(QMouseEvent *event) {
        // 双击事件的处理与单击相同
        mousePressEvent(event);
    }

    bool CodeEditorLineArea::isInBreakpointArea(const QPoint& pos) const {
        return pos.x() <= codeEditor->BREAKPOINT_MARGIN;
    }

    int CodeEditorLineArea::lineNumberAtPos(const QPoint& pos) const {
        QTextCursor cursor = codeEditor->cursorForPosition(QPoint(0, pos.y()));
        return cursor.blockNumber() + 1;
    }
}
