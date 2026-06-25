#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QTextStream>
#include <QFileInfo>
#include <QTextCursor>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 日志轮询定时器 400ms
    m_logTimer = new QTimer(this);
    m_logTimer->setInterval(400);
    connect(m_logTimer, &QTimer::timeout, this, &MainWindow::readLog);

    // 计时定时器 1秒刷新一次用时
    m_timeTimer = new QTimer(this);
    m_timeTimer->setInterval(1000);
    connect(m_timeTimer, &QTimer::timeout, this, &MainWindow::updateTimeLabel);

    // 日志面板控制台深色样式
    ui->plainTextEdit->setStyleSheet(
        "background:#1e1e1e;color:#ccc;font-family:Consolas;font-size:14px;"
        );
    // 初始用时 00:00:00
    ui->label2->setText(formatTime(0));

    // 程序启动自动读取保存的公司、产品名
    loadConfig();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 秒数格式化 HH:mm:ss
QString MainWindow::formatTime(int sec)
{
    int h = sec / 3600;
    int m = (sec % 3600) / 60;
    int s = sec % 60;
    return QString("%1:%2:%3")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'));
}

// ========== 统一控制所有需要锁定的控件：启用/禁用（自带灰色置灰效果） ==========
void MainWindow::setWidgetEnable(bool enable)
{
    // 锁定辅助控件交互+文字置灰
    ui->pushButton_3->setEnabled(enable);
    ui->pushButton_2->setEnabled(enable);
    ui->lineEdit->setEnabled(enable);
    ui->lineEdit_2->setEnabled(enable);

    QPalette pal;
    if (enable)
    {
        // enable=true = 停止监控状态，按钮绿色（开始监控）
        ui->pushButton->setStyleSheet("background:#268836;color:white;padding:4px 8px;");
        // 辅助控件正常白色文字
        pal.setColor(QPalette::Text, Qt::white);
        pal.setColor(QPalette::ButtonText, Qt::white);
    }
    else
    {
        // enable=false = 正在监控状态，按钮红色（停止监控）
        ui->pushButton->setStyleSheet("background:#a82626;color:white;padding:4px 8px;");
        // 辅助控件置灰文字
        pal.setColor(QPalette::Text, QColor(120,120,120));
        pal.setColor(QPalette::ButtonText, QColor(120,120,120));
    }
    // 批量设置锁定控件调色板
    ui->pushButton_2->setPalette(pal);
    ui->pushButton_3->setPalette(pal);
    ui->lineEdit->setPalette(pal);
    ui->lineEdit_2->setPalette(pal);
}

// 拼接Player.log完整路径
QString MainWindow::getLogPath()
{
    QString user = QDir::home().dirName();
    return QString(R"(C:/Users/%1/AppData/LocalLow/%2/%3/Player.log)")
        .arg(user, m_company, m_product);
}

// 每秒更新监控用时标签
void MainWindow::updateTimeLabel()
{
    m_totalSec++;
    ui->label2->setText(formatTime(m_totalSec));
}

// 【单个共用按钮：开始监控 / 停止监控】
void MainWindow::on_pushButton_clicked()
{
    if (!m_isMonitoring)
    {
        m_company = ui->lineEdit->text().trimmed();
        m_product = ui->lineEdit_2->text().trimmed();

        if (m_company.isEmpty() || m_product.isEmpty())
        {
            QMessageBox::warning(this, "提示", "请先填写打包公司名和产品名！");
            return;
        }

        // 新增：点击开始监控，自动保存当前输入到txt
        saveConfig();

        // 下面原有代码不变
        QString logPath = getLogPath();
        QFileInfo info(logPath);
        if (!info.exists())
        {
            ui->plainTextEdit->clear();
            QMessageBox::warning(this, "未找到日志", QString("日志文件不存在：\n%1\n请先运行Unity打包程序生成日志！").arg(logPath));
            return;
        }

        ui->plainTextEdit->clear();
        m_lastPos = 0;

        m_logFile.setFileName(logPath);
        if (m_logFile.open(QIODevice::ReadOnly))
        {
            QTextStream st(&m_logFile);
            QString allLog = st.readAll();
            if (!allLog.isEmpty())
                ui->plainTextEdit->appendPlainText(allLog);
            m_lastPos = m_logFile.size();
            m_logFile.close();
        }

        m_isMonitoring = true;
        ui->pushButton->setText("停止监控");
        setWidgetEnable(false);

        m_logTimer->start();
        m_totalSec = 0;
        m_timeTimer->start();

        QTextCursor cur = ui->plainTextEdit->textCursor();
        cur.movePosition(QTextCursor::End);
        ui->plainTextEdit->setTextCursor(cur);
    }
    else
    {
        m_isMonitoring = false;
        ui->pushButton->setText("开始监控");
        setWidgetEnable(true);

        m_logTimer->stop();
        m_timeTimer->stop();
        m_totalSec = 0;
        ui->label2->setText(formatTime(0));
    }
}

// 【删除日志文件按钮】
void MainWindow::on_pushButton_2_clicked()
{
    m_company = ui->lineEdit->text().trimmed();
    m_product = ui->lineEdit_2->text().trimmed();
    QString logPath = getLogPath();
    QFile logFile(logPath);
    if (logFile.exists())
    {
        bool ok = logFile.remove();
        if (!ok)
        {
            QMessageBox::critical(this, "删除失败", "文件被占用或权限不足，删除日志失败");
        }
    }
    else
    {
        QMessageBox::information(this, "提示", "日志文件不存在，无需删除");
    }
}

// ========== 新增：清空日志显示按钮（仅清空面板，不操作磁盘文件） ==========
void MainWindow::on_pushButton_3_clicked()
{
    ui->plainTextEdit->clear();
}

// 轮询读取日志，仅输出文件原生内容，无自定义提示
void MainWindow::readLog()
{
    QString logPath = getLogPath();
    m_logFile.setFileName(logPath);
    QFileInfo info(logPath);
    if (!info.exists() || !m_logFile.open(QIODevice::ReadOnly))
        return;

    qint64 fileSize = m_logFile.size();

    // Unity重启日志重置，清空界面
    if (m_lastPos > fileSize)
    {
        ui->plainTextEdit->clear();
        m_lastPos = 0;
    }

    m_logFile.seek(m_lastPos);
    QTextStream st(&m_logFile);
    QString newText = st.readAll();
    m_lastPos = m_logFile.pos();
    m_logFile.close();

    if (newText.isEmpty())
        return;

    // 定义三种文字格式
    QTextCharFormat fmtNormal;
    fmtNormal.setForeground(QColor(200,200,200)); // 默认浅灰

    QTextCharFormat fmtWarn;
    fmtWarn.setForeground(Qt::yellow); // 警告黄色

    QTextCharFormat fmtError;
    fmtError.setForeground(Qt::red); // 报错红色

    QTextCursor cursor = ui->plainTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);

    // 按行拆分日志逐行上色
    QStringList lines = newText.split('\n', Qt::SkipEmptyParts);
    // 先把普通列表转为常量列表，再遍历
    const QStringList& constLines = lines;
    for (const QString& line : constLines)
    {
        QTextCharFormat targetFmt = fmtNormal;
        QString lowerLine = line.toLower();

        if (lowerLine.contains("error") || lowerLine.contains("exception") || lowerLine.contains("failed"))
        {
            targetFmt = fmtError;
        }
        else if (lowerLine.contains("warning") || lowerLine.contains("warn"))
        {
            targetFmt = fmtWarn;
        }

        cursor.insertText(line + "\n", targetFmt);
    }

    // 滚动到最底部
    ui->plainTextEdit->setTextCursor(cursor);
}

// 获取exe根目录下config.txt路径
QString MainWindow::getConfigPath()
{
    // 获取用户专属可读写目录，不受 Program Files 权限限制
    QString cfgRoot = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir cfgDir(cfgRoot);
    // 目录不存在则自动创建
    if (!cfgDir.exists())
    {
        cfgDir.mkpath(".");
    }
    // 最终配置文件完整路径
    return cfgDir.filePath("config.txt");
}

// 读取本地配置，回填输入框
void MainWindow::loadConfig()
{
    QString path = getConfigPath();
    QFile file(path);
    // 文件不存在直接跳过（第一次打开程序没有txt）
    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    // 第一行=公司名，第二行=产品名
    QString company = stream.readLine().trimmed();
    QString product = stream.readLine().trimmed();
    file.close();

    // 回填输入框
    ui->lineEdit->setText(company);
    ui->lineEdit_2->setText(product);
}

// 保存当前输入框内容到config.txt
void MainWindow::saveConfig()
{
    QString path = getConfigPath();
    QFile file(path);
    // Write模式：不存在自动创建，存在直接覆盖
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    // 第一行存公司，第二行存产品
    stream << ui->lineEdit->text().trimmed() << "\n";
    stream << ui->lineEdit_2->text().trimmed() << "\n";
    file.close();
}