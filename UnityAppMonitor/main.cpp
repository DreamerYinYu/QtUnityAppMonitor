#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");
    // 2. 构建深色调色板
    QPalette darkPalette;
    // 窗口背景
    darkPalette.setColor(QPalette::Window, QColor(45,45,45));
    // 窗口文字
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    // 控件背景
    darkPalette.setColor(QPalette::Base, QColor(30,30,30));
    // 控件文字
    darkPalette.setColor(QPalette::Text, Qt::white);
    // 按钮
    darkPalette.setColor(QPalette::Button, QColor(60,60,60));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    // 边框、分割线
    darkPalette.setColor(QPalette::Mid, QColor(90,90,90));
    // 高亮选中
    darkPalette.setColor(QPalette::Highlight, QColor(42,130,218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    // 3. 全局应用深色调色板
    a.setPalette(darkPalette);

    MainWindow w;
    w.setWindowIcon(QIcon("favicon32.ico"));
    w.show();
    return QApplication::exec();
}
