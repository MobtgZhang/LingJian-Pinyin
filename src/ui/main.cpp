#include <QApplication>
#include <QTimer>

#include "candidate_view.h"
#include "status_bar.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    StatusBar statusBar;
    statusBar.move(400, 360);
    statusBar.show();
    QTimer::singleShot(0, &statusBar, [&statusBar]() {
        statusBar.adjustSize();
    });

    CandidateView view;
    view.setCandidates(QStringList() << QStringLiteral("你好") << QStringLiteral("您好") << QStringLiteral("你 好"));
    view.move(400, 400);
    view.show();
    QTimer::singleShot(0, &view, [&view]() {
        view.adjustSize();
    });

    return app.exec();
}

