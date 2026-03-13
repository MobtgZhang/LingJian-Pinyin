#pragma once

#include <QFrame>
#include <QStringList>
#include <QPoint>
#include <QVector>
#include <QRectF>

class QMouseEvent;
class QContextMenuEvent;

class CandidateView : public QFrame {
    Q_OBJECT
public:
    explicit CandidateView(QWidget *parent = nullptr);

    void setCandidates(const QStringList &candidates);
    void setPreeditText(const QString &text);
    void setPageInfo(int current, int total);
    void setHighlightedIndex(int index);

    void applySkinColors(const QColor &bg, const QColor &border,
                         const QColor &text, const QColor &highlight,
                         const QColor &preedit, const QColor &index,
                         int borderRadius, int fontSize);

signals:
    void candidateClicked(int index);
    void pageUpClicked();
    void pageDownClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void rebuildLayout();
    int hitTestCandidate(const QPoint &pos) const;

    QStringList candidates_;
    QString preeditText_;
    int currentPage_ = 1;
    int totalPages_ = 1;

    QColor bgColor_{255, 255, 255, 245};
    QColor borderColor_{210, 210, 210};
    QColor textColor_{51, 51, 51};
    QColor highlightColor_{74, 144, 217};
    QColor preeditColor_{255, 102, 0};
    QColor indexColor_{153, 153, 153};
    int borderRadius_ = 8;
    int fontSize_ = 14;

    struct CandRect {
        QRectF rect;
        int index;
    };
    QVector<CandRect> candRects_;
    QRectF pageUpRect_;
    QRectF pageDownRect_;
    int highlightedIndex_ = 0;
    int hoveredCandidate_ = -1;

    bool dragging_ = false;
    QPoint dragPosition_;

    static constexpr int kHeight = 44;
    static constexpr int kPreeditHeight = 24;
    static constexpr int kPadding = 12;
};
