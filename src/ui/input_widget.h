#pragma once

#include <QTextEdit>
#include <QTimer>
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

    void setSimplifiedTraditional(bool traditional);
    void setHalfFullWidth(bool fullWidth);

    /** 插入已确认的文本（会应用简繁、全半角转换） */
    void insertCommittedText(const QString &text);

    /** 插入普通文本（会应用全半角标点转换，供软键盘等调用） */
    void insertTextWithConversion(const QString &text);

signals:
    void chineseModeChanged(bool on);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void commitText(const QString &text);
    QString applyPunctuationWidth(const QString &text) const;
    void updateCandidateView();
    void positionCandidateView();
    void scheduleDecode();
    void flushDecode();

    std::shared_ptr<core::InputContext> ctx_;
    CandidateView *candidateView_ = nullptr;
    StatusBar *statusBar_ = nullptr;
    bool chineseMode_ = true;
    bool traditionalMode_ = false;
    bool fullWidthMode_ = true;
    QTimer decodeDebounceTimer_;
    static constexpr int kDebounceMs = 35;
};
