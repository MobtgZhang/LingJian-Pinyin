#pragma once

#include <QTextEdit>
#include <memory>

namespace core {
class InputContext;
}
class CandidateView;
class StatusBar;

class InputWidget : public QTextEdit {
    Q_OBJECT
public:
    explicit InputWidget(QWidget *parent = nullptr);
    ~InputWidget() override;

    void setInputContext(std::shared_ptr<core::InputContext> ctx);
    void setCandidateView(CandidateView *view);
    void setStatusBar(StatusBar *bar);

    bool isChineseMode() const { return chineseMode_; }
    void setChineseMode(bool on);

signals:
    void chineseModeChanged(bool on);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void commitText(const QString &text);
    void updateCandidateView();
    void positionCandidateView();

    std::shared_ptr<core::InputContext> ctx_;
    CandidateView *candidateView_ = nullptr;
    StatusBar *statusBar_ = nullptr;
    bool chineseMode_ = true;
};
