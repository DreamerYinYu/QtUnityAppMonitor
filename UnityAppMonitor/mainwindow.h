#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QFile>
#include <QDateTime>
#include <QStandardPaths>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 开始/停止监控共用按钮
    void on_pushButton_clicked();
    // 删除日志文件按钮
    void on_pushButton_2_clicked();
    // 清空面板显示按钮（新增）
    void on_pushButton_3_clicked();
    // 轮询读取日志增量
    void readLog();
    // 更新监控用时
    void updateTimeLabel();

private:
    Ui::MainWindow *ui;
    QTimer *m_logTimer;    // 日志读取定时器
    QTimer *m_timeTimer;   // 计时定时器
    QFile m_logFile;
    qint64 m_lastPos = 0;
    bool m_isMonitoring = false; // 监控状态标记
    int m_totalSec = 0;
    QString m_company;
    QString m_product;

    // 统一设置所有锁定控件启用/禁用
    void setWidgetEnable(bool enable);
    QString formatTime(int sec);
    QString getLogPath();

    QString getConfigPath();    // 获取config.txt完整路径
    void loadConfig();         // 启动加载配置
    void saveConfig();         // 保存配置到txt
};

#endif // MAINWINDOW_H