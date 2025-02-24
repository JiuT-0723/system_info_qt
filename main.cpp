#include <QCoreApplication>
#include <QDateTime>
#include <QSocketNotifier>
#include <QTextStream>
#include <QTimer>
#include <memory>

#include "sys_info.h"

// 定义颜色代码
#define RESET "\033[0m"
#define GOLOR "\033[38;2"
#define LIGHT_GREEN "\033[38;2;100;255;0m"
#define YELLOW "\033[38;2;255;230;0m"
#define RED "\033[38;2;255;0;0m"
#define ORINGE "\033[38;2;255;165;0m"

class TerminalMonitor : public QObject {
  Q_OBJECT
 public:
  explicit TerminalMonitor(QObject *parent = nullptr) : QObject(parent) {
    sys_info_ptr = std::make_unique<jiut::classSysInfo>();
    // 设置终端输入监控
    stdinNotifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
    connect(stdinNotifier, &QSocketNotifier::activated, this, &TerminalMonitor::handleInput);

    // 初始化数据刷新定时器
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &TerminalMonitor::refreshData);
    refreshTimer->start(1000);  // 1秒刷新

    // 初始化显示
    QTextStream(stdout) << "\033[2J\033[H";  // 清屏
    updateHeader();
  }

 private slots:
  void refreshData() {
    // 模拟数据采集（替换为实际数据源）
    sys_info_ptr->getNpuInfo(npu_info_);
    sys_info_ptr->getGpuUsage(gpu_occupy_);
    sys_info_ptr->getMemUsage(mem_occupy_);
    sys_info_ptr->getCpuInfo(cpu_info_);
    float core_temp = sys_info_ptr->getCoreTemp();
    uint64_t ddr_freq = sys_info_ptr->getDDRFreq();
    // 使用ANSI转义码控制显示位置
    QTextStream out(stdout);
    out << "\033[6;1H";  // 移动到第6行第1列

    out << RESET << "* 当前时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toUtf8().constData() << "                \033[7;1H";  // 移动到第6行第1列

    out << RESET << "* CPU 使用率: " << setColor(cpu_info_.usage) << QString::number(cpu_info_.usage, 'f', 2) << "%                   "
        << "\033[8;1H";  // 移动到第7行第1列

    out << RESET << "* CPU 频率:" << "                              \033[9;1H";  // 移动到第7行第2列
    out << RESET << "* 小核: " << ORINGE << simpleNum(cpu_info_.freq[0]) << RESET << " 大核1: " << ORINGE << simpleNum(cpu_info_.freq[1])  << RESET
        << " 大核2: " << ORINGE << simpleNum(cpu_info_.freq[2]) << "                         \033[10;1H";  // 移动到第7行第2列

    out << RESET << "* GPU 使用率: " << setColor(gpu_occupy_.usage) << QString::number(gpu_occupy_.usage) << "% " << RESET << "| GPU 频率: " << ORINGE
        << simpleNum(gpu_occupy_.freq) << "Hz                           \033[11;1H";  // 移动到第8行第1列

    out << RESET << "* NPU 使用率: 1: " << setColor(npu_info_.core0) << QString::number(npu_info_.core0) << "%" << RESET << " 2: " << setColor(npu_info_.core1)
        << QString::number(npu_info_.core1) << "% " << RESET << "3: " << setColor(npu_info_.core2) << QString::number(npu_info_.core2) << RESET << "% | 频率: " << ORINGE
        << simpleNum(npu_info_.freq) << "                         \033[12;1H";  // 移动到第9行第1列

    out << RESET << "* 内存 使用情况: " << setColor(mem_occupy_.usage) << QString::number(mem_occupy_.used, 'f', 2) << "GB / " << QString::number(mem_occupy_.total, 'f', 2)
        << "GB " << RESET << " 使用率: " << setColor(mem_occupy_.usage) << QString::number(mem_occupy_.usage, 'f', 2) << "%                              \033[13;1H";  // 移动到第10行第1列

    out << RESET << "* 核心温度: " << setColor(core_temp) << QString::number(core_temp, 'f', 2) << "°C                                \033[14;1H";  // 清除行尾

    out << RESET << "* DDR 频率: " << ORINGE << simpleNum(ddr_freq) << "                        \033[0K"      // 清除行尾
        << RESET << "                                                                      \033[15;1H";  // 移动到命令输入行
    // << "Command: ";

    out.flush();  // 立即刷新输出缓冲区
  }

  void handleInput() {
    QTextStream in(stdin);
    QString cmd = in.readLine().trimmed();

    if (cmd == "q") {
      QTextStream(stdout) << "\033[2J\033[H";  // 清屏
      qApp->quit();
    } else if (cmd.startsWith("interval ")) {
      bool ok;
      int interval = cmd.section(' ', 1).toInt(&ok);
      if (ok && interval > 0) {
        refreshTimer->setInterval(interval);
        updateHeader();
      }
    }
  }

 private:
  std::unique_ptr<jiut::classSysInfo> sys_info_ptr;

  jiut::npu_info_t npu_info_;
  jiut::gpu_occupy_t gpu_occupy_;
  jiut::mem_occupy_t mem_occupy_;
  jiut::cpu_info_t cpu_info_;
  void updateHeader() {
    QTextStream out(stdout);
    out << "\033[1;1H"  // 第1行
        << "* ====================================================="
        << "\033[2;1H"  // 第2行
        << "*                   SYSTEM INFOMATION                  "
        << "\033[3;1H"  // 第3行
        << "* ====================================================="
        << "\033[4;1H"  // 第4行
        << "*                                      更新周期: " << refreshTimer->interval() << "ms"
        << "\033[5;1H"  // 第5行
        << "* "
        << "\033[0K";  // 清除行尾
    out.flush();
  }

  QString setColor(float value) {
    if (value > 60) {
      char color[20];
      sprintf(color, "%s;%d;%d;0m", GOLOR, 255, static_cast<int>(255 - (value - 60.0) / 40.0 * 255));
      return QString::fromUtf8(color);
    } else {
      char color[20];
      sprintf(color, "%s;%d;%d;0m", GOLOR, static_cast<int>(value / 60.0 * 255), 255);
      return QString::fromUtf8(color);
    }
  }

  QString simpleNum(uint64_t num) {
    if (num < 1000) {
      return QString::number(num) + "Hz";
    } else if (num < 1000 * 1000) {
      return QString::number(num / 1000.0) + "kHz";
    } else if (num < 1000 * 1000 * 1000) {
      return QString::number(num / 1000.0 / 1000.0) + "MHz";
    } else {
      return QString::number(num / 1000.0 / 1000.0 / 1000.0) + "GHz";
    }
  }

  QTimer *refreshTimer;
  QSocketNotifier *stdinNotifier;
};

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  // 设置终端模式
  QTextStream(stdout) << "\033[?25l";  // 隐藏光标

  TerminalMonitor monitor;

  // 退出时恢复终端设置
  QObject::connect(&a, &QCoreApplication::aboutToQuit, [] {
    QTextStream(stdout) << "\033[?25h";  // 显示光标
  });

  return a.exec();
}

#include "main.moc"
