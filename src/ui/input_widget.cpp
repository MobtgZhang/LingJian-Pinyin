#include "input_widget.h"
#include "candidate_view.h"
#include "status_bar.h"
#include "sc_tc_converter.h"

#include <QKeyEvent>
#include <QScrollBar>
#include <context.h>

InputWidget::InputWidget(QWidget *parent)
    : QTextEdit(parent) {
    setPlaceholderText(QStringLiteral("在此输入文字..."));
    setFont(QFont(QStringLiteral("Noto Sans CJK SC,Microsoft YaHei,SimSun"), 14));
    setMinimumSize(500, 300);
    decodeDebounceTimer_.setSingleShot(true);
    connect(&decodeDebounceTimer_, &QTimer::timeout, this, [this]() {
        if (ctx_ && ctx_->isComposing()) {
            ctx_->updateCandidates();
            updateCandidateView();
        }
    });
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

void InputWidget::setSimplifiedTraditional(bool traditional) {
    traditionalMode_ = traditional;
}

void InputWidget::setHalfFullWidth(bool fullWidth) {
    fullWidthMode_ = fullWidth;
}

void InputWidget::insertCommittedText(const QString &text) {
    commitText(text);
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
        if (!event->text().isEmpty()) {
            insertTextWithConversion(event->text());
            event->accept();
            return;
        }
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
        if (r == core::InputContext::KeyResult::Committed) {
            commitText(QString::fromStdString(ctx_->committedText()));
            ctx_->clearCommitted();
        }
        scheduleDecode();
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
        int globalIdx = ctx_->currentPage() * ctx_->pageSize() + ctx_->currentCursorIndex();
        flushDecode();
        auto r = ctx_->selectCandidate(static_cast<std::size_t>(globalIdx));
        if (r == core::InputContext::KeyResult::Committed) {
            commitText(QString::fromStdString(ctx_->committedText()));
            ctx_->clearCommitted();
        }
        updateCandidateView();
        return;
    }

    if (ctx_->isComposing() && key >= Qt::Key_1 && key <= Qt::Key_9) {
        flushDecode();
        char ch = '1' + (key - Qt::Key_1);
        auto r = ctx_->handleKey(ch);
        if (r == core::InputContext::KeyResult::Committed) {
            commitText(QString::fromStdString(ctx_->committedText()));
            ctx_->clearCommitted();
        }
        updateCandidateView();
        return;
    }

    if (key == Qt::Key_Minus || key == Qt::Key_Up || key == Qt::Key_PageUp) {
        if (ctx_->isComposing()) {
            ctx_->handlePageUp();
            updateCandidateView();
            return;
        }
    }

    if (key == Qt::Key_Equal || key == Qt::Key_Down || key == Qt::Key_PageDown) {
        if (ctx_->isComposing()) {
            ctx_->handlePageDown();
            updateCandidateView();
            return;
        }
    }

    if (key == Qt::Key_Left) {
        if (ctx_->isComposing()) {
            ctx_->handleCursorLeft();
            updateCandidateView();
            return;
        }
    }

    if (key == Qt::Key_Right) {
        if (ctx_->isComposing()) {
            ctx_->handleCursorRight();
            updateCandidateView();
            return;
        }
    }

    if (!text.isEmpty()) {
        char ch = text.at(0).toLatin1();
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            auto r = ctx_->handleKey(ch);
            if (r == core::InputContext::KeyResult::Consumed) {
                scheduleDecode();
                updateCandidateView();
                return;
            }
        }
    }

    if (!ctx_->isComposing()) {
        if (!event->text().isEmpty()) {
            insertTextWithConversion(event->text());
            event->accept();
            return;
        }
        QTextEdit::keyPressEvent(event);
    }
}

QString InputWidget::applyPunctuationWidth(const QString &text) const {
    if (text.isEmpty()) return text;
    QString result = text;
    if (fullWidthMode_) {
        QString out;
        out.reserve(result.size());
        for (QChar ch : result) {
            ushort c = ch.unicode();
            if (c >= 0x0021 && c <= 0x007E) {
                out += QChar(0xFF01 + (c - 0x0021));
            } else if (c == 0x0020) {
                out += QChar(0x3000);
            } else {
                out += ch;
            }
        }
        result = out;
    } else {
        QString out;
        out.reserve(result.size());
        for (QChar ch : result) {
            ushort c = ch.unicode();
            if (c >= 0xFF01 && c <= 0xFF5E) {
                out += QChar(0x0021 + (c - 0xFF01));
            } else if (c == 0x3000) {
                out += QChar(0x0020);
            } else if (c == 0x3001) {
                out += QChar(',');
            } else if (c == 0x3002) {
                out += QChar('.');
            } else {
                out += ch;
            }
        }
        result = out;
    }
    return result;
}

void InputWidget::insertTextWithConversion(const QString &text) {
    if (text.isEmpty()) return;
    QString result = applyPunctuationWidth(text);
    QTextEdit::insertPlainText(result);
}

void InputWidget::commitText(const QString &text) {
    if (text.isEmpty()) return;
    QString result = text;
    if (traditionalMode_ && chineseMode_) {
        lingjian::ScTcConverter conv;
        if (conv.isAvailable()) {
            result = QString::fromStdString(conv.toTraditional(text.toStdString()));
        }
    }
    result = applyPunctuationWidth(result);
    textCursor().insertText(result);
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
    lingjian::ScTcConverter conv;
    const bool needTraditional = traditionalMode_ && chineseMode_ && conv.isAvailable();
    for (const auto &c : pageCandidates) {
        QString text = QString::fromStdString(c.text);
        if (needTraditional) {
            text = QString::fromStdString(conv.toTraditional(c.text));
        }
        items << text;
    }

    candidateView_->setPreeditText(preedit);
    candidateView_->setCandidates(items);
    candidateView_->setPageInfo(ctx_->currentPage() + 1, ctx_->totalPages());
    candidateView_->setHighlightedIndex(ctx_->currentCursorIndex());

    positionCandidateView();
    if (!candidateView_->isVisible()) {
        candidateView_->show();
    }
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

void InputWidget::scheduleDecode() {
    decodeDebounceTimer_.stop();
    if (ctx_ && ctx_->isComposing()) {
        decodeDebounceTimer_.start(kDebounceMs);
    }
}

void InputWidget::flushDecode() {
    decodeDebounceTimer_.stop();
    if (ctx_ && ctx_->isComposing()) {
        ctx_->updateCandidates();
    }
}
