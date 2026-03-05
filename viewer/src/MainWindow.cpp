#include "MainWindow.h"

#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "DeviceSession.h"
#include "protocol_v1.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    m_session = new DeviceSession();

    setWindowTitle(QString("XIAO RP2350 Scope+FG (Protocol v%1)").arg(PROTOCOL_V1_VERSION));
    resize(1200, 760);

    auto *root = new QWidget(this);
    auto *rootLayout = new QVBoxLayout(root);

    auto *toolbarLayout = new QHBoxLayout();
    auto *connectButton = new QPushButton("Connect", root);
    auto *disconnectButton = new QPushButton("Disconnect", root);
    auto *pingButton = new QPushButton("Ping", root);
    auto *capsButton = new QPushButton("Get Caps", root);
    m_statusLabel = new QLabel("Status: Disconnected", root);
    toolbarLayout->addWidget(connectButton);
    toolbarLayout->addWidget(disconnectButton);
    toolbarLayout->addWidget(pingButton);
    toolbarLayout->addWidget(capsButton);
    toolbarLayout->addSpacing(16);
    toolbarLayout->addWidget(m_statusLabel);
    toolbarLayout->addStretch();
    rootLayout->addLayout(toolbarLayout);

    auto *tabs = new QTabWidget(root);

    auto *scopeTab = new QWidget(tabs);
    auto *scopeLayout = new QVBoxLayout(scopeTab);

    auto *scopeControls = new QGroupBox("Scope Controls", scopeTab);
    auto *scopeForm = new QFormLayout(scopeControls);

    auto *runMode = new QComboBox(scopeControls);
    runMode->addItems({"Auto", "Normal", "Single"});

    auto *triggerSource = new QComboBox(scopeControls);
    triggerSource->addItems({"CH1", "CH2"});

    auto *triggerEdge = new QComboBox(scopeControls);
    triggerEdge->addItems({"Rising", "Falling"});

    auto *sampleRate = new QComboBox(scopeControls);
    sampleRate->addItems({"50 kS/s", "100 kS/s", "250 kS/s", "500 kS/s (1CH)"});

    scopeForm->addRow("Run Mode", runMode);
    scopeForm->addRow("Trigger Source", triggerSource);
    scopeForm->addRow("Trigger Edge", triggerEdge);
    scopeForm->addRow("Sample Rate", sampleRate);

    auto *scopeActions = new QHBoxLayout();
    auto *startStreamButton = new QPushButton("Start Stream", scopeControls);
    auto *stopStreamButton = new QPushButton("Stop Stream", scopeControls);
    scopeActions->addWidget(startStreamButton);
    scopeActions->addWidget(stopStreamButton);
    scopeActions->addStretch();

    scopeLayout->addWidget(scopeControls);
    scopeLayout->addLayout(scopeActions);
    scopeLayout->addWidget(new QLabel("Waveform canvas placeholder", scopeTab));
    scopeLayout->addStretch();

    auto *fgTab = new QWidget(tabs);
    auto *fgLayout = new QVBoxLayout(fgTab);

    auto *fgControls = new QGroupBox("Function Generator", fgTab);
    auto *fgForm = new QFormLayout(fgControls);

    auto *waveform = new QComboBox(fgControls);
    waveform->addItems({"Sine", "Square", "Triangle", "Sawtooth", "DC"});

    auto *freqHz = new QSpinBox(fgControls);
    freqHz->setRange(1, 500000);
    freqHz->setValue(1000);

    auto *ampMvpp = new QSpinBox(fgControls);
    ampMvpp->setRange(0, 3300);
    ampMvpp->setValue(1000);

    auto *offsetMv = new QSpinBox(fgControls);
    offsetMv->setRange(-1650, 1650);
    offsetMv->setValue(0);

    fgForm->addRow("Waveform", waveform);
    fgForm->addRow("Frequency (Hz)", freqHz);
    fgForm->addRow("Amplitude (mVpp)", ampMvpp);
    fgForm->addRow("Offset (mV)", offsetMv);

    auto *fgActions = new QHBoxLayout();
    fgActions->addWidget(new QPushButton("Apply", fgControls));
    fgActions->addWidget(new QPushButton("Enable", fgControls));
    fgActions->addWidget(new QPushButton("Disable", fgControls));
    fgActions->addStretch();

    fgLayout->addWidget(fgControls);
    fgLayout->addLayout(fgActions);
    fgLayout->addStretch();

    tabs->addTab(scopeTab, "Oscilloscope");
    tabs->addTab(fgTab, "Function Generator");
    rootLayout->addWidget(tabs);

    auto *logGroup = new QGroupBox("Session Log", root);
    auto *logLayout = new QVBoxLayout(logGroup);
    m_logView = new QTextEdit(logGroup);
    m_logView->setReadOnly(true);
    logLayout->addWidget(m_logView);
    rootLayout->addWidget(logGroup);

    setCentralWidget(root);

    appendLog("UI ready. Using mock transport backend.");

    connect(connectButton, &QPushButton::clicked, this, [this]() {
        QString status;
        if (m_session->connectDevice(status)) {
            m_statusLabel->setText(status);
            appendLog("Connected to device (mock).");
        } else {
            appendLog("Connect failed.");
        }
    });

    connect(disconnectButton, &QPushButton::clicked, this, [this]() {
        QString status;
        m_session->disconnectDevice(status);
        m_statusLabel->setText(status);
        appendLog("Disconnected.");
    });

    connect(pingButton, &QPushButton::clicked, this, [this]() {
        QString line;
        if (!m_session->ping(line)) {
            appendLog(QString("ERROR: %1").arg(line));
            return;
        }
        appendLog(line);
    });

    connect(capsButton, &QPushButton::clicked, this, [this]() {
        QString line;
        if (!m_session->getCaps(line)) {
            appendLog(QString("ERROR: %1").arg(line));
            return;
        }
        appendLog(line);
    });

    connect(startStreamButton, &QPushButton::clicked, this, [this]() {
        QString line;
        if (!m_session->startStream(line)) {
            appendLog(QString("ERROR: %1").arg(line));
            return;
        }
        appendLog(line);
    });

    connect(stopStreamButton, &QPushButton::clicked, this, [this]() {
        QString line;
        if (!m_session->stopStream(line)) {
            appendLog(QString("ERROR: %1").arg(line));
            return;
        }
        appendLog(line);
    });
}

void MainWindow::appendLog(const QString &line) {
    m_logView->append(line);
}
