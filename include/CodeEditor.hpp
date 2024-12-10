#pragma once

#include <QPlainTextEdit>
#include "QSet"

namespace TinaToolBox {
    class CodeEditorLineArea;

    class CodeEditor : public QPlainTextEdit {
        Q_OBJECT

    public:
        explicit CodeEditor(QWidget *parent = nullptr);

        ~CodeEditor() override;

        // 断点相关方法
        void toggleBreakpoint(int line);

        bool hasBreakpoint(int line) const;

        QSet<int> getBreakpoints() const;

        void clearBreakpoints();

        // 行号区域宽度计算

        int lineNumberAreaWidth() const;

        void lineNumberAreaPaintEvent(QPaintEvent *event);

    signals:
        void breakpointToggled(int line, bool added);

    protected:
        void resizeEvent(QResizeEvent *event) override;

        void wheelEvent(QWheelEvent *event) override;

    private slots:
        void updateLineNumberAreaWidth(int newBlockCount);

        void highlightCurrentLine();

        void updateLineNumberArea(const QRect &rect, int dy);

    private:
        CodeEditorLineArea *lineNumberArea;

        QSet<int> breakpoints_;

        void setupEditor();

        void setupConnections();

        void zoomIn(int range);

        void zoomOut(int range);

        static constexpr int MIN_FONT_SIZE = 8;
        static constexpr int MAX_FONT_SIZE = 32;

        // 断点区域宽度
        static constexpr int BREAKPOINT_MARGIN = 15;
        // 行号左边距
        static constexpr int LINE_NUMBER_PADDING = 10;

        friend class CodeEditorLineArea;
    };

    // 专门的行号区域类，支持断点功能
    class CodeEditorLineArea : public QWidget {
    public:
        explicit CodeEditorLineArea(CodeEditor *editor);

        QSize sizeHint() const override;

    protected:
        void paintEvent(QPaintEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;

        void mouseDoubleClickEvent(QMouseEvent *event) override;

    private:
        CodeEditor *codeEditor;

        bool isInBreakpointArea(const QPoint &pos) const;

        int lineNumberAtPos(const QPoint& pos) const;
    };
}
