#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QTimer>
#include <cstdlib>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
#include <string>
#include <QFileDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Definition of the calss for the software ui
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr); // Constructor
    ~MainWindow();  // Destructor

    void addPoint(double x, double y);
    void clearData();
    void plot();

private slots:
    void on_btn_getData_clicked();

    void on_btn_clear_clicked();

    void on_btn_saveData_clicked();

    void readSerial();

private:
    Ui::MainWindow *ui;

    QVector<double> qv_time, qv_signal;

    double t0;

    QSerialPort *external;

    QString serial_port_name;

    QByteArray serialData;

    QString serialBuffer;
};

#endif // MAINWINDOW_H
