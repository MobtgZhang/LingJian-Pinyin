#pragma once

#include <QWidget>

class MoreInputSubmenu : public QWidget {
    Q_OBJECT
public:
    explicit MoreInputSubmenu(QWidget *parent = nullptr);

    void popup(const QPoint &pos);

signals:
    void voiceInputClicked();
    void handwritingInputClicked();
    void skinStoreClicked();
    void customizeStatusBarClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QRectF itemRect(int index) const;
    int itemHitTest(const QPoint &pos) const;

    int hoveredItem_ = -1;
};
