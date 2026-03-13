#pragma once

#include <QWidget>

class HelpSubmenu : public QWidget {
    Q_OBJECT
public:
    explicit HelpSubmenu(QWidget *parent = nullptr);

    void popup(const QPoint &pos);

signals:
    void aboutClicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QRectF itemRect() const;
    bool hovered_ = false;
};
