#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QTextEdit;

class DeviceSession;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    DeviceSession *m_session;
    QLabel *m_statusLabel;
    QTextEdit *m_logView;

    void appendLog(const QString &line);
};

#endif
