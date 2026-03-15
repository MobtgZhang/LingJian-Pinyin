#pragma once

#include <QString>
#include <QWidget>

class QDialog;

/** 与主界面风格一致的主题提示框（用于语音识别错误等） */
class ThemedMessageBox {
public:
    /** 显示警告提示。若 voiceDialog 非空，显示期间设置其 suppressCloseOnFocusLoss，关闭后清除，避免点击确定时关闭语音窗口。 */
    static void warning(QWidget* parent, const QString& title, const QString& text,
                        QWidget* voiceDialog = nullptr);
};
