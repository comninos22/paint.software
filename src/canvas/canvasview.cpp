#include "canvasview.h"
#include "i18n.h"
#include "canvaswidget.h"
#include "core/document.h"
#include <QGridLayout>
#include <QScrollBar>
#include <QSignalBlocker>
#include <algorithm>

CanvasView::CanvasView(QWidget *parent) : QWidget(parent),
    m_canvas(new CanvasWidget(this)),
    m_horizontal(new QScrollBar(Qt::Horizontal, this)),
    m_vertical(new QScrollBar(Qt::Vertical, this))
{
    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_canvas, 0, 0);
    layout->addWidget(m_vertical, 0, 1);
    layout->addWidget(m_horizontal, 1, 0);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(0, 1);
    setFocusProxy(m_canvas);
    retranslate();
    m_horizontal->setSingleStep(20);
    m_vertical->setSingleStep(20);

    connect(m_canvas, &CanvasWidget::viewChanged, this, &CanvasView::updateScrollBars);
    connect(m_canvas, &CanvasWidget::scrollRequested, this, &CanvasView::scrollBy);
    connect(m_horizontal, &QScrollBar::valueChanged, this, [this](int value) {
        m_canvas->setPan(QPointF(-value, m_canvas->pan().y()));
    });
    connect(m_vertical, &QScrollBar::valueChanged, this, [this](int value) {
        m_canvas->setPan(QPointF(m_canvas->pan().x(), -value));
    });
    updateScrollBars();
}

void CanvasView::updateScrollBars() {
    auto updateAxis = [](QScrollBar *bar, double imageSize, int viewportSize, double pan) {
        const QSignalBlocker blocker(bar);
        viewportSize = std::max(1, viewportSize);
        // Small images stay centred. Large images have a little workspace around
        // their edges. Include a hand-panned/zoom-anchored position without moving
        // it, so updating the bars never changes the user's view.
        const double centre = (viewportSize - imageSize) / 2.0;
        const double minPan = imageSize > viewportSize ? viewportSize - imageSize - 40 : centre;
        const double maxPan = imageSize > viewportSize ? 40 : centre;
        const int value = qRound(-pan);
        bar->setRange(std::min(qRound(-maxPan), value), std::max(qRound(-minPan), value));
        bar->setPageStep(viewportSize);
        bar->setValue(value);
    };
    const Document *doc = m_canvas->document();
    if (!doc) {
        const QSignalBlocker horizontal(m_horizontal), vertical(m_vertical);
        m_horizontal->setRange(0, 0);
        m_vertical->setRange(0, 0);
        return;
    }
    const int rs = m_canvas->rulerSize();
    updateAxis(m_horizontal, doc->width() * m_canvas->zoom(), m_canvas->width() - rs, m_canvas->pan().x());
    updateAxis(m_vertical, doc->height() * m_canvas->zoom(), m_canvas->height() - rs, m_canvas->pan().y());
}

void CanvasView::scrollBy(const QPointF &delta) {
    auto moveAxis = [](QScrollBar *bar, double pan, double amount) {
        if (amount == 0 || bar->minimum() == bar->maximum()) return pan;
        return std::clamp(pan + amount, -double(bar->maximum()), -double(bar->minimum()));
    };
    const QPointF pan = m_canvas->pan();
    const QPointF next(moveAxis(m_horizontal, pan.x(), delta.x()),
                       moveAxis(m_vertical, pan.y(), delta.y()));
    if (next != pan) m_canvas->setPan(next);
}

void CanvasView::retranslate() {
    m_horizontal->setAccessibleName(TR("Barre de défilement horizontale du canevas"));
    m_vertical->setAccessibleName(TR("Barre de défilement verticale du canevas"));
}
