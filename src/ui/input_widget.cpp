#include "input_widget.h"
#include "candidate_view.h"
#include "status_bar.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <context.h>

InputWidget::InputWidget(QWidget *parent)
    : QTextEdit(parent) {
    setPlaceholderText(QStringLiteral("在此输入文字..."));
    setFont(QFont(QStringLiteral("Noto Sans CJK SC,Microsoft YaHei,SimSun"), 14));
    setMinimumSize(500, 300);
}

InputWidget::~InputWidget() = default;

void InputWidget::setInputContext(std::shared_ptr<core::InputContext> ctx) {
    ctx_ = std::move(ctx);
}

void InputWidget::setCandidateView(CandidateView *view) {
    candidateView_ = view;
}

void InputWidget::setStatusBar(StatusBar *bar) {
    statusBar_ = bar;
}

void InputWidget::setChineseMode(bool on) {
    if (chineseMode_ == on) return;
    chineseMode_ = on;
    if (!on && ctx_ && ctx_->isComposing()) {
        ctx_->handleEscape();
        updateCandidateView();
    }
    emit chineseModeChanged(on);
}

void InputWidget::keyPressEvent(QKeyEvent *event) {
    if (!ctx_ || !chineseMode_) {
        QTextEdit::keyPressEvent(event);
        return;
    }

    int key = event->key();
    QString text = event->text();

    if (key == Qt::Key_Backspace) {
        auto r = ctx_->handleBackspace();
        if (r == core::InputContext::KeyResult::Ignored) {
            QTextEdit::keyPressEvent(event);
            return;
        }
        updateCandidateView();
        return;
    }

    if (key == Qt::Key_Escape) {
        auto r = ctx_->handleEscape();
        if (r == core::InputContext::KeyResult::Ignored) {
            QTextEdit::keyPressEvent(event);
            return;
        }
        updateCandidateView();
        return;
    }

    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        auto r = ctx_->handleEnter();
        if (r == core::InputContext::KeyResult::Committed) {
            commitText(QString::fromStdString(ctx_->committedText()));
            ctx_->clearCommitted();
            updateCandidateView();
            return;
        }
        QTextEdit::keyPressEvent(event);
        return;
    }

    if (key == Qt::Key_Space && ctx_->isComposing()) {
        auto r = ctx_->handleKey(' ');
        if (r == core::InputContext::KeyResult::Committed) {
            commitText(QString::fromStdString(ctx_->committedText()));
            ctx_->clearCommitted();
        }
        updateCandidateView();
        return;
    }

    if (ctx_->isComposing() && key >= Qt::Key_1 && key <= Qt::Key_9) {
        char ch = '1' + (key - Qt::Key_1);
        auto r = ctx_->handleKey(ch);
        if (r == core::InputContext::KeyResult::Committed) {
            commitText(QString::fromStdString(ctx_->committedText()));
            ctx_->clearCommitted();
        }
        updateCandidateView();
        return;
    }

    if (key == Qt::Key_Minus || key == Qt::Key_PageUp) {
        if (ctx_->isComposing()) {
            ctx_->handlePageUp();
            updateCandidateView();
            return;
        }
    }

    if (key == Qt::Key_Equal || key == Qt::Key_PageDown) {
        if (ctx_->isComposing()) {
            ctx_->handlePageDown();
            updateCandidateView();
            return;
        }
    }

    if (!text.isEmpty()) {
        char ch = text.at(0).toLatin1();
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            auto r = ctx_->handleKey(ch);
            if (r == core::InputContext::KeyResult::Consumed) {
                updateCandidateView();
                return;
            }
        }
    }

    if (!ctx_->isComposing()) {
        QTextEdit::keyPressEvent(event);
    }
}

void InputWidget::commitText(const QString &text) {
    if (text.isEmpty()) return;
    textCursor().insertText(text);
}

void InputWidget::updateCandidateView() {
    if (!candidateView_) return;

    if (!ctx_->isComposing()) {
        candidateView_->hide();
        return;
    }

    QString preedit = QString::fromStdString(ctx_->segmentedPreedit());
    auto pageCandidates = ctx_->currentPageCandidates();

    QStringList items;
    for (const auto &c : pageCandidates) {
        items << QString::fromStdString(c.text);
    }

    candidateView_->setPreeditText(preedit);
    candidateView_->setCandidates(items);
    candidateView_->setPageInfo(ctx_->currentPage() + 1, ctx_->totalPages());

    positionCandidateView();
    candidateView_->show();
    candidateView_->raise();
}

void InputWidget::positionCandidateView() {
    if (!candidateView_) return;

    QRect cursorRect = this->cursorRect();
    QPoint globalPos = mapToGlobal(cursorRect.bottomLeft());
    globalPos.setY(globalPos.y() + 4);
    candidateView_->move(globalPos);
    candidateView_->adjustSize();
}
