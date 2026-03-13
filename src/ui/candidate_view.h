#pragma once

#include <QFrame>
#include <QStringList>
#include <QPoint>

class QMouseEvent;
class QContextMenuEvent;

class CandidateView : public QFrame {
    Q_OBJECT
public:
    explicit CandidateView(QWidget *parent = nullptr);

    void setCandidates(const QStringList &candidates);

    enum class Skin {
        Dark,
        Light,
        Blue,
    };

    void setSkin(Skin skin);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QStringList candidates_;
    bool dragging_ = false;
    QPoint dragPosition_;
    Skin skin_ = Skin::Dark;
};

