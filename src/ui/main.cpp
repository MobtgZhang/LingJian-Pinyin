#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QScreen>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#include "candidate_view.h"
#include "input_widget.h"
#include "status_bar.h"
#include "theme_manager.h"
#include "skin_loader.h"
#include "skin_selector.h"

#include <context.h>

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

static void applySkinToUI(StatusBar *statusBar, CandidateView *candidateView) {
    const auto &skin = ThemeManager::instance().skin();
    statusBar->applySkinColors(
        skin.sbBackground, skin.sbBorder, skin.sbText,
        skin.sbHover, skin.sbLogo, skin.sbAi, skin.sbBorderRadius);
    candidateView->applySkinColors(
        skin.cvBackground, skin.cvBorder, skin.cvText,
        skin.cvHighlight, skin.cvPreedit, skin.cvIndex,
        skin.cvBorderRadius, skin.candidateFontSize);
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
        "空格键选第一个候选，数字键1-5选候选词，-/=翻页，Esc取消输入。\n"
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
    statusBar.show();
    QTimer::singleShot(0, &statusBar, [&]() {
        statusBar.adjustSize();
        int x = screenGeo.center().x() - statusBar.width() / 2;
        int y = screenGeo.bottom() - statusBar.height() - 60;
        statusBar.move(x, y);
    });

    inputWidget->setStatusBar(&statusBar);

    QObject::connect(&statusBar, &StatusBar::inputModeToggled,
                     inputWidget, [inputWidget](StatusBar::InputMode mode) {
        inputWidget->setChineseMode(mode == StatusBar::InputMode::Chinese);
    });

    QObject::connect(&candidateView, &CandidateView::candidateClicked,
                     inputWidget, [&ctx, inputWidget, &candidateView](int index) {
        int globalIdx = ctx->currentPage() * ctx->pageSize() + index;
        auto r = ctx->selectCandidate(static_cast<std::size_t>(globalIdx));
        if (r == core::InputContext::KeyResult::Committed) {
            inputWidget->textCursor().insertText(
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
                applySkinToUI(&statusBar, &candidateView);
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
        applySkinToUI(&statusBar, &candidateView);
    });

    applySkinToUI(&statusBar, &candidateView);

    mainWin.show();
    inputWidget->setFocus();

    return app.exec();
}
