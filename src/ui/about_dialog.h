#pragma once

#include <QWidget>

class AboutDialog : public QWidget {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget *parent = nullptr);

    void popup(const QPoint &pos);
    void showCentered(QWidget *parent);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QRectF closeButtonRect() const;
    bool closeHovered_ = false;
};
