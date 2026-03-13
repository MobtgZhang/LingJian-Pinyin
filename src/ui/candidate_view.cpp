#include "candidate_view.h"

#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QContextMenuEvent>

CandidateView::CandidateView(QWidget *parent)
    : QFrame(parent) {
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void CandidateView::setCandidates(const QStringList &candidates) {
    candidates_ = candidates;
    updateGeometry();
    update();
}

void CandidateView::setSkin(Skin skin) {
    skin_ = skin;
    updateGeometry();
    update();
}

void CandidateView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int radius = 8;
    QRect rect = this->rect().adjusted(1, 1, -1, -1);

    QColor backgroundColor;
    QColor borderColor;
    QColor textColor;
    QFont font;

    switch (skin_) {
    case Skin::Light:
        backgroundColor = QColor(250, 250, 250, 235);
        borderColor = QColor(200, 200, 200);
        textColor = QColor(40, 40, 40);
        font.setPointSize(12);
        break;
    case Skin::Blue:
        backgroundColor = QColor(30, 60, 120, 230);
        borderColor = QColor(40, 120, 220);
        textColor = QColor(235, 245, 255);
        font.setPointSize(13);
        break;
    case Skin::Dark:
    default:
        backgroundColor = QColor(30, 30, 30, 220);
        borderColor = QColor(80, 80, 80);
        textColor = QColor(240, 240, 240);
        font.setPointSize(12);
        break;
    }

    painter.setBrush(backgroundColor);
    painter.setPen(QPen(borderColor, 1));
    painter.drawRoundedRect(rect, radius, radius);

    painter.setPen(textColor);
    painter.setFont(font);

    int x = 16;
    int y = height() / 2 + painter.fontMetrics().ascent() / 2 - 2;
    int index = 1;
    for (const auto &cand : candidates_) {
        const QString text = QString::number(index) + ". " + cand;
        painter.drawText(x, y, text);
        x += painter.fontMetrics().horizontalAdvance(text) + 24;
        ++index;
    }
}

QSize CandidateView::sizeHint() const {
    if (candidates_.isEmpty()) {
        return {200, 40};
    }
    QFont font;
    switch (skin_) {
    case Skin::Light:
    case Skin::Dark:
        font.setPointSize(12);
        break;
    case Skin::Blue:
        font.setPointSize(13);
        break;
    }
    QFontMetrics fm(font);
    int width = 32;
    for (int i = 0; i < candidates_.size(); ++i) {
        const QString text = QString::number(i + 1) + ". " + candidates_.at(i);
        width += fm.horizontalAdvance(text) + 24;
    }
    return {width, 40};
}

void CandidateView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
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

void CandidateView::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    QAction *darkAction = menu.addAction(QStringLiteral("暗色皮肤"));
    QAction *lightAction = menu.addAction(QStringLiteral("亮色皮肤"));
    QAction *blueAction = menu.addAction(QStringLiteral("蓝色皮肤"));

    QAction *selected = menu.exec(event->globalPos());
    if (!selected) {
        return;
    }

    if (selected == darkAction) {
        skin_ = Skin::Dark;
    } else if (selected == lightAction) {
        skin_ = Skin::Light;
    } else if (selected == blueAction) {
        skin_ = Skin::Blue;
    }
    updateGeometry();
    update();
}


