#include "candidate_view.h"

#include <algorithm>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

CandidateView::CandidateView(QWidget *parent)
    : QFrame(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);
}

void CandidateView::setCandidates(const QStringList &candidates) {
    if (candidates_ == candidates) return;
    candidates_ = candidates;
    rebuildLayout();
    updateGeometry();
    adjustSize();
    update();
}

void CandidateView::setPreeditText(const QString &text) {
    if (preeditText_ == text) return;
    preeditText_ = text;
    rebuildLayout();
    updateGeometry();
    adjustSize();
    update();
}

void CandidateView::setPageInfo(int current, int total) {
    if (currentPage_ == current && totalPages_ == total) return;
    currentPage_ = current;
    totalPages_ = total;
    update();
}

void CandidateView::setHighlightedIndex(int index) {
    int maxIdx = std::max(0, static_cast<int>(candidates_.size()) - 1);
    int clamped = std::clamp(index, 0, maxIdx);
    if (highlightedIndex_ != clamped) {
        highlightedIndex_ = clamped;
        update();
    }
}

void CandidateView::applySkinColors(const QColor &bg, const QColor &border,
                                     const QColor &text, const QColor &highlight,
                                     const QColor &preedit, const QColor &index,
                                     int borderRadius, int fontSize) {
    bgColor_ = bg;
    borderColor_ = border;
    textColor_ = text;
    highlightColor_ = highlight;
    preeditColor_ = preedit;
    indexColor_ = index;
    borderRadius_ = borderRadius;
    fontSize_ = fontSize;
    rebuildLayout();
    updateGeometry();
    adjustSize();
    update();
}

void CandidateView::rebuildLayout() {
    candRects_.clear();

    QFont font;
    font.setPointSize(fontSize_);
    QFontMetrics fm(font);

    int totalHeight = kPreeditHeight + kHeight;
    int x = kPadding;

    for (int i = 0; i < candidates_.size(); ++i) {
        QString label = QString::number(i + 1) + QStringLiteral(". ") + candidates_[i];
        int w = fm.horizontalAdvance(label) + 16;
        CandRect cr;
        cr.index = i;
        cr.rect = QRectF(x, kPreeditHeight, w, kHeight);
        candRects_.append(cr);
        x += w + 4;
    }

    int arrowW = 24;
    int arrowY = kPreeditHeight;
    pageUpRect_ = QRectF(x + 4, arrowY, arrowW, kHeight);
    pageDownRect_ = QRectF(x + 4 + arrowW + 2, arrowY, arrowW, kHeight);
}

QSize CandidateView::sizeHint() const {
    int minW = 200;
    int minH = kPreeditHeight + kHeight;
    if (candidates_.isEmpty() && preeditText_.isEmpty()) {
        return {minW, minH};
    }

    double right = kPadding;
    for (const auto &cr : candRects_) {
        right = std::max(right, cr.rect.right());
    }
    right = std::max(right, pageDownRect_.right());

    QFont preeditFont;
    preeditFont.setPointSize(fontSize_ - 1);
    QFontMetrics pfm(preeditFont);
    int preeditWidth = pfm.horizontalAdvance(preeditText_) + kPadding * 2;

    int w = std::max(static_cast<int>(right) + kPadding, preeditWidth);
    w = std::max(w, minW);

    return {w, minH};
}

int CandidateView::hitTestCandidate(const QPoint &pos) const {
    for (const auto &cr : candRects_) {
        if (cr.rect.contains(pos)) return cr.index;
    }
    return -1;
}

void CandidateView::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    QRectF bgRect = QRectF(rect()).adjusted(1, 1, -1, -1);
    QPainterPath bgPath;
    bgPath.addRoundedRect(bgRect, borderRadius_, borderRadius_);
    p.fillPath(bgPath, bgColor_);
    p.setPen(QPen(borderColor_, 1));
    p.drawPath(bgPath);

    if (!preeditText_.isEmpty()) {
        QFont preeditFont;
        preeditFont.setPointSize(fontSize_ - 1);
        p.setFont(preeditFont);
        p.setPen(preeditColor_);
        QRectF preeditRect(kPadding, 2, width() - kPadding * 2, kPreeditHeight);
        p.drawText(preeditRect, Qt::AlignVCenter | Qt::AlignLeft, preeditText_);

        p.setPen(QPen(borderColor_, 0.5));
        p.drawLine(QPointF(kPadding, kPreeditHeight),
                   QPointF(width() - kPadding, kPreeditHeight));
    }

    QFont font;
    font.setPointSize(fontSize_);

    for (int i = 0; i < candRects_.size(); ++i) {
        const auto &cr = candRects_[i];

        bool isHighlighted = (hoveredCandidate_ == cr.index) ||
                             (highlightedIndex_ == cr.index);
        if (isHighlighted) {
            QPainterPath hp;
            hp.addRoundedRect(cr.rect.adjusted(0, 4, 0, -4), 4, 4);
            p.fillPath(hp, QColor(highlightColor_.red(), highlightColor_.green(),
                                  highlightColor_.blue(), 30));
        }

        QString indexStr = QString::number(cr.index + 1) + QStringLiteral(". ");
        QString candText = candidates_[cr.index];

        p.setFont(font);

        QRectF indexRect(cr.rect.left() + 4, cr.rect.top(), 24, cr.rect.height());
        p.setPen(highlightedIndex_ == cr.index ? highlightColor_ : indexColor_);
        p.drawText(indexRect, Qt::AlignVCenter | Qt::AlignLeft, indexStr);

        int indexW = p.fontMetrics().horizontalAdvance(indexStr);
        QRectF textRect(cr.rect.left() + 4 + indexW, cr.rect.top(),
                        cr.rect.width() - indexW - 8, cr.rect.height());
        p.setPen(highlightedIndex_ == cr.index ? highlightColor_ : textColor_);
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, candText);
    }

    if (totalPages_ > 1) {
        QFont arrowFont;
        arrowFont.setPointSize(10);
        p.setFont(arrowFont);

        p.setPen(currentPage_ > 1 ? textColor_ : QColor(180, 180, 180));
        p.drawText(pageUpRect_, Qt::AlignCenter, QStringLiteral("\u25C0"));

        p.setPen(currentPage_ < totalPages_ ? textColor_ : QColor(180, 180, 180));
        p.drawText(pageDownRect_, Qt::AlignCenter, QStringLiteral("\u25B6"));

        QFont pageFont;
        pageFont.setPointSize(9);
        p.setFont(pageFont);
        p.setPen(indexColor_);
        QString pageText = QString::number(currentPage_) + QStringLiteral("/")
                           + QString::number(totalPages_);
        QRectF pageRect(pageDownRect_.right() + 2, pageDownRect_.top(),
                        40, pageDownRect_.height());
        p.drawText(pageRect, Qt::AlignVCenter | Qt::AlignLeft, pageText);
    }
}

void CandidateView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        int idx = hitTestCandidate(event->pos());
        if (idx >= 0) {
            emit candidateClicked(idx);
            event->accept();
            return;
        }

        if (pageUpRect_.contains(event->pos())) {
            emit pageUpClicked();
            event->accept();
            return;
        }
        if (pageDownRect_.contains(event->pos())) {
            emit pageDownClicked();
            event->accept();
            return;
        }

        dragging_ = true;
        dragPosition_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
        setCursor(Qt::SizeAllCursor);
        event->accept();
        return;
    }
    QFrame::mousePressEvent(event);
}

void CandidateView::mouseMoveEvent(QMouseEvent *event) {
    if (dragging_ && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - dragPosition_);
        event->accept();
        return;
    }

    int idx = hitTestCandidate(event->pos());
    if (idx != hoveredCandidate_) {
        hoveredCandidate_ = idx;
        if (idx >= 0)
            setCursor(Qt::PointingHandCursor);
        else
            unsetCursor();
        update();
    }
    QFrame::mouseMoveEvent(event);
}

void CandidateView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && dragging_) {
        dragging_ = false;
        unsetCursor();
        event->accept();
        return;
    }
    QFrame::mouseReleaseEvent(event);
}
