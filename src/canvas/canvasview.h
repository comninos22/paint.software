#pragma once

#include <QWidget>

class CanvasWidget;
class QScrollBar;

// Keep the canvas viewport independent of its always-visible navigation bars.
class CanvasView : public QWidget {
public:
    explicit CanvasView(QWidget *parent = nullptr);
    CanvasWidget *canvas() const { return m_canvas; }
    QScrollBar *horizontalScrollBar() const { return m_horizontal; }
    QScrollBar *verticalScrollBar() const { return m_vertical; }

private:
    void updateScrollBars();
    void scrollBy(const QPointF &delta);

    CanvasWidget *m_canvas;
    QScrollBar *m_horizontal;
    QScrollBar *m_vertical;
};
