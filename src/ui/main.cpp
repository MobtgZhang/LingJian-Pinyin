#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QScreen>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPainter>
#include <QImage>
#include <QCursor>

#include "candidate_view.h"
#include "input_widget.h"
#include "status_bar.h"
#include "theme_manager.h"
#include "skin_loader.h"
#include "skin_selector.h"
#include "soft_keyboard.h"
#include "about_dialog.h"

#include <context.h>

static QIcon createLingTrayIcon() {
    const int size = 22;
    QImage img(size, size, QImage::Format_ARGB32);
    img.fill(0);
    QPainter p;
    if (!p.begin(&img)) return QIcon();
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    // 浅色圆形背景 + 深色文字，在深色/浅色托盘主题下都清晰可见
    p.setBrush(QColor(248, 248, 248));
    p.setPen(Qt::NoPen);
    p.drawEllipse(2, 2, size - 4, size - 4);
    QFont font;
    font.setPointSize(12);
    font.setWeight(QFont::Bold);
    p.setFont(font);
    p.setPen(QColor(50, 50, 50));
    p.drawText(QRect(0, 0, size, size), Qt::AlignCenter, QStringLiteral("\u7075"));
    p.end();
    return QIcon(QPixmap::fromImage(img));
}

static QString findDictPath() {
    QStringList searchPaths = {
        QApplication::applicationDirPath() + QStringLiteral("/../../data/pinyin_dict.txt"),
        QApplication::applicationDirPath() + QStringLiteral("/../data/pinyin_dict.txt"),
        QApplication::applicationDirPath() + QStringLiteral("/data/pinyin_dict.txt"),
        QStringLiteral("data/pinyin_dict.txt"),
        QStringLiteral("../data/pinyin_dict.txt"),
        QStringLiteral("../../data/pinyin_dict.txt"),
    };
    for (const auto &p : searchPaths) {
        if (QFile::exists(p)) {
            return QDir(p).absolutePath();
        }
    }
    return {};
}

static void applySkinToUI(StatusBar *statusBar, CandidateView *candidateView,
                          SoftKeyboard *softKeyboard) {
    const auto &skin = ThemeManager::instance().skin();
    statusBar->applySkinColors(
        skin.sbBackground, skin.sbBorder, skin.sbText,
        skin.sbHover, skin.sbLogo, skin.sbAi, skin.sbBorderRadius);
    candidateView->applySkinColors(
        skin.cvBackground, skin.cvBorder, skin.cvText,
        skin.cvHighlight, skin.cvPreedit, skin.cvIndex,
        skin.cvBorderRadius, skin.candidateFontSize);
    softKeyboard->applySkinColors(
        skin.skBackground, skin.skBorder, skin.skKeyBg, skin.skKeyBorder,
        skin.skKeyText, skin.skKeyHover, skin.skKeyPressed,
        skin.skFuncKeyBg, skin.skTitleBg, skin.skTitleText, skin.skBorderRadius);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("灵键拼音"));

    auto ctx = std::make_shared<core::InputContext>();
    QString dictPath = findDictPath();
    if (!dictPath.isEmpty()) {
        ctx->loadDictionary(dictPath.toStdString());
    }

    QMainWindow mainWin;
    mainWin.setWindowTitle(QStringLiteral("灵键拼音 - LingJian Pinyin"));
    mainWin.resize(640, 420);

    auto *central = new QWidget;
    auto *layout = new QVBoxLayout(central);

    auto *hintLabel = new QLabel(QStringLiteral(
        "提示：在下方输入框中输入拼音，候选栏会自动弹出。\n"
        "空格键选当前候选，数字键1-5选候选词，-/=或↑↓翻页，←→左右选字，Esc取消输入。\n"
        "点击状态栏「中/英」按钮或右键菜单切换输入模式。\n"
        "点击「🎨」按钮加载皮肤ZIP文件（参考搜狗皮肤格式）。"));
    hintLabel->setWordWrap(true);
    hintLabel->setStyleSheet(QStringLiteral(
        "QLabel { color: #666; padding: 8px; background: #f8f8f8; "
        "border-radius: 6px; font-size: 12px; }"));
    layout->addWidget(hintLabel);

    CandidateView candidateView;
    candidateView.hide();

    auto *inputWidget = new InputWidget;
    inputWidget->setInputContext(ctx);
    inputWidget->setCandidateView(&candidateView);
    layout->addWidget(inputWidget);

    mainWin.setCentralWidget(central);

    StatusBar statusBar;
    QScreen *screen = app.primaryScreen();
    QRect screenGeo = screen->availableGeometry();
    QTimer::singleShot(0, &statusBar, [&]() {
        statusBar.adjustSize();
        int x = screenGeo.center().x() - statusBar.width() / 2;
        int y = screenGeo.bottom() - statusBar.height() - 60;
        statusBar.move(x, y);
        statusBar.show();
    });

    inputWidget->setStatusBar(&statusBar);

    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(&mainWin);
    trayIcon->setIcon(createLingTrayIcon());
    trayIcon->setToolTip(QStringLiteral("灵键拼音"));
    trayIcon->show();

    QObject::connect(trayIcon, &QSystemTrayIcon::activated, trayIcon,
        [&](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
                statusBar.show();
                statusBar.raise();
                statusBar.activateWindow();
            }
        });

    QObject::connect(&statusBar, &StatusBar::hideRequested, &mainWin, [&]() {
        statusBar.hide();
    });

    QObject::connect(&statusBar, &StatusBar::inputModeToggled,
                     inputWidget, [inputWidget](StatusBar::InputMode mode) {
        inputWidget->setChineseMode(mode == StatusBar::InputMode::Chinese);
    });

    QObject::connect(&statusBar, &StatusBar::simplifiedTraditionalToggled,
                     inputWidget, [inputWidget](StatusBar::SimplifiedTraditional mode) {
        inputWidget->setSimplifiedTraditional(mode == StatusBar::SimplifiedTraditional::Traditional);
    });

    QObject::connect(&statusBar, &StatusBar::halfFullWidthToggled,
                     inputWidget, [inputWidget](StatusBar::HalfFullWidth mode) {
        inputWidget->setHalfFullWidth(mode == StatusBar::HalfFullWidth::Full);
    });

    QObject::connect(&candidateView, &CandidateView::candidateClicked,
                     inputWidget, [&ctx, inputWidget, &candidateView](int index) {
        int globalIdx = ctx->currentPage() * ctx->pageSize() + index;
        auto r = ctx->selectCandidate(static_cast<std::size_t>(globalIdx));
        if (r == core::InputContext::KeyResult::Committed) {
            inputWidget->insertCommittedText(
                QString::fromStdString(ctx->committedText()));
            ctx->clearCommitted();
        }
        candidateView.hide();
    });

    QObject::connect(&candidateView, &CandidateView::pageUpClicked,
                     inputWidget, [&ctx, &candidateView]() {
        ctx->handlePageUp();
        auto pageCandidates = ctx->currentPageCandidates();
        QStringList items;
        for (const auto &c : pageCandidates)
            items << QString::fromStdString(c.text);
        candidateView.setCandidates(items);
        candidateView.setPageInfo(ctx->currentPage() + 1, ctx->totalPages());
        candidateView.setHighlightedIndex(ctx->currentCursorIndex());
    });

    QObject::connect(&candidateView, &CandidateView::pageDownClicked,
                     inputWidget, [&ctx, &candidateView]() {
        ctx->handlePageDown();
        auto pageCandidates = ctx->currentPageCandidates();
        QStringList items;
        for (const auto &c : pageCandidates)
            items << QString::fromStdString(c.text);
        candidateView.setCandidates(items);
        candidateView.setPageInfo(ctx->currentPage() + 1, ctx->totalPages());
        candidateView.setHighlightedIndex(ctx->currentCursorIndex());
    });

    auto *softKeyboard = new SoftKeyboard;

    QObject::connect(&statusBar, &StatusBar::keyboardClicked, &mainWin, [&]() {
        if (softKeyboard->isVisible()) {
            softKeyboard->hide();
        } else {
            QPoint popupPos = statusBar.mapToGlobal(
                QPoint(statusBar.width() / 2, 0));
            softKeyboard->popup(popupPos);
        }
    });

    QObject::connect(softKeyboard, &SoftKeyboard::keyPressed,
                     inputWidget, [inputWidget](const QString &text) {
        inputWidget->insertPlainText(text);
    });

    QObject::connect(softKeyboard, &SoftKeyboard::specialKeyPressed,
                     inputWidget, [inputWidget](Qt::Key key) {
        switch (key) {
        case Qt::Key_Backspace: {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
            QApplication::sendEvent(inputWidget, &ev);
            break;
        }
        case Qt::Key_Return: {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
            QApplication::sendEvent(inputWidget, &ev);
            break;
        }
        case Qt::Key_Tab: {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
            QApplication::sendEvent(inputWidget, &ev);
            break;
        }
        case Qt::Key_Delete: {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
            QApplication::sendEvent(inputWidget, &ev);
            break;
        }
        case Qt::Key_Insert: {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Insert, Qt::NoModifier);
            QApplication::sendEvent(inputWidget, &ev);
            break;
        }
        default:
            break;
        }
    });

    auto *skinSelector = new SkinSelector;

    QObject::connect(&statusBar, &StatusBar::skinClicked, &mainWin, [&]() {
        QPoint popupPos = statusBar.mapToGlobal(
            QPoint(statusBar.width() / 2, 0));
        skinSelector->popup(popupPos);
    });

    QObject::connect(skinSelector, &SkinSelector::skinSelected,
                     &mainWin, [&](const QString &skinId) {
        ThemeManager::instance().setBuiltinSkin(skinId);
        skinSelector->setCurrentSkin(skinId);
    });

    QObject::connect(skinSelector, &SkinSelector::loadCustomSkinClicked,
                     &mainWin, [&]() {
        QStringList filters;
        filters << QStringLiteral("皮肤文件 (*.zip *.ssf *.ljs)")
                << QStringLiteral("所有文件 (*)");
        QString path = QFileDialog::getOpenFileName(
            &mainWin, QStringLiteral("加载皮肤文件"),
            QDir::homePath(), filters.join(QStringLiteral(";;")));

        if (path.isEmpty()) return;

        if (path.endsWith(QStringLiteral(".zip")) ||
            path.endsWith(QStringLiteral(".ssf")) ||
            path.endsWith(QStringLiteral(".ljs"))) {
            if (ThemeManager::instance().loadSkinFromZip(path)) {
                applySkinToUI(&statusBar, &candidateView, softKeyboard);
                QMessageBox::information(&mainWin,
                    QStringLiteral("皮肤加载成功"),
                    QStringLiteral("已成功加载皮肤：") +
                        ThemeManager::instance().skin().name);
            } else {
                QMessageBox::warning(&mainWin,
                    QStringLiteral("皮肤加载失败"),
                    QStringLiteral("无法加载皮肤文件，请确认ZIP包中包含skin.ini。"));
            }
        }
    });

    QObject::connect(&ThemeManager::instance(), &ThemeManager::skinChanged,
                     &mainWin, [&]() {
        applySkinToUI(&statusBar, &candidateView, softKeyboard);
    });

    // 托盘右键菜单：使用原生 QMenu，避免自定义弹窗导致的双框、闪退问题
    QMenu *trayContextMenu = new QMenu(&mainWin);
    QAction *actToggleBar = trayContextMenu->addAction(QStringLiteral("隐藏状态栏"));
    QAction *actInputMode = trayContextMenu->addAction(QStringLiteral("中/英切换"));
    QAction *actScTc = trayContextMenu->addAction(QStringLiteral("简/繁切换"));
    QAction *actHalfFull = trayContextMenu->addAction(QStringLiteral("全/半角切换"));
    trayContextMenu->addSeparator();
    QAction *actKeyboard = trayContextMenu->addAction(QStringLiteral("软键盘"));
    QAction *actSkin = trayContextMenu->addAction(QStringLiteral("皮肤商城"));
    trayContextMenu->addSeparator();
    QAction *actAbout = trayContextMenu->addAction(QStringLiteral("关于灵键"));

    QObject::connect(trayContextMenu, &QMenu::aboutToShow, &mainWin, [&]() {
        actToggleBar->setText(statusBar.isVisible()
            ? QStringLiteral("隐藏状态栏") : QStringLiteral("显示状态栏"));
        actInputMode->setText((statusBar.inputMode() == StatusBar::InputMode::Chinese)
            ? QStringLiteral("中/英切换 (当前: 中)") : QStringLiteral("中/英切换 (当前: 英)"));
        actScTc->setText((statusBar.simplifiedTraditional() == StatusBar::SimplifiedTraditional::Simplified)
            ? QStringLiteral("简/繁切换 (当前: 简)") : QStringLiteral("简/繁切换 (当前: 繁)"));
        actHalfFull->setText((statusBar.halfFullWidth() == StatusBar::HalfFullWidth::Full)
            ? QStringLiteral("全/半角切换 (当前: 全)") : QStringLiteral("全/半角切换 (当前: 半)"));
    });

    QObject::connect(actToggleBar, &QAction::triggered, &mainWin, [&]() {
        if (statusBar.isVisible()) {
            statusBar.requestHide();
        } else {
            statusBar.show();
            statusBar.raise();
            statusBar.activateWindow();
        }
    }, Qt::QueuedConnection);
    QObject::connect(actInputMode, &QAction::triggered, &mainWin, [&]() {
        StatusBar::InputMode newMode = (statusBar.inputMode() == StatusBar::InputMode::Chinese)
            ? StatusBar::InputMode::English : StatusBar::InputMode::Chinese;
        statusBar.setInputMode(newMode);
        inputWidget->setChineseMode(newMode == StatusBar::InputMode::Chinese);
    });
    QObject::connect(actScTc, &QAction::triggered, &mainWin, [&]() {
        StatusBar::SimplifiedTraditional newMode =
            (statusBar.simplifiedTraditional() == StatusBar::SimplifiedTraditional::Simplified)
            ? StatusBar::SimplifiedTraditional::Traditional : StatusBar::SimplifiedTraditional::Simplified;
        statusBar.setSimplifiedTraditional(newMode);
        inputWidget->setSimplifiedTraditional(newMode == StatusBar::SimplifiedTraditional::Traditional);
    });
    QObject::connect(actHalfFull, &QAction::triggered, &mainWin, [&]() {
        StatusBar::HalfFullWidth newMode = (statusBar.halfFullWidth() == StatusBar::HalfFullWidth::Full)
            ? StatusBar::HalfFullWidth::Half : StatusBar::HalfFullWidth::Full;
        statusBar.setHalfFullWidth(newMode);
        inputWidget->setHalfFullWidth(newMode == StatusBar::HalfFullWidth::Full);
    });
    QObject::connect(actKeyboard, &QAction::triggered, &mainWin, [&]() {
        if (softKeyboard->isVisible()) {
            softKeyboard->hide();
        } else {
            softKeyboard->popup(QCursor::pos());
        }
    });
    QObject::connect(actSkin, &QAction::triggered, &mainWin, [&]() {
        skinSelector->popup(QCursor::pos());
    });
    QObject::connect(actAbout, &QAction::triggered, &mainWin, [&]() {
        AboutDialog *about = new AboutDialog(&mainWin);
        about->setAttribute(Qt::WA_DeleteOnClose);
        about->showCentered(&mainWin);
    });

    trayIcon->setContextMenu(trayContextMenu);

    applySkinToUI(&statusBar, &candidateView, softKeyboard);

    mainWin.show();
    inputWidget->setFocus();

    return app.exec();
}
