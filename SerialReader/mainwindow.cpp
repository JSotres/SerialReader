#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->plotWidget->setInteraction(QCP::iRangeDrag, true);
    ui->plotWidget->setInteraction(QCP::iRangeZoom, true);

    ui->plotWidget->addGraph();
    ui->plotWidget->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plotWidget->xAxis->setLabel("Time (s)");
    ui->plotWidget->yAxis->setLabel("Signal");

    external = new QSerialPort(this);

    serialBuffer = "";

    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->cbox_ports->addItem(serialPortInfo.portName());
    }

    ui->cbox_baud->addItems({"1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"});
}

MainWindow::~MainWindow()
{
    if (external->isOpen())
    {
        external->close();
    }
    delete ui;
}


void MainWindow::on_btn_getData_clicked()
{
    if(ui->btn_getData->isChecked())
    {
        ui->btn_getData->setText("Stop");
        external->setPortName(ui->cbox_ports->currentText());
        external->open(QSerialPort::ReadOnly);
        if (ui->cbox_baud->currentText()=="1200")
        {
            external->setBaudRate(QSerialPort::Baud1200);
        } else if (ui->cbox_baud->currentText()=="2400")
        {
            external->setBaudRate(QSerialPort::Baud2400);
        } else if (ui->cbox_baud->currentText()=="4800")
        {
            external->setBaudRate(QSerialPort::Baud4800);
        } else if (ui->cbox_baud->currentText()=="9600")
        {
            external->setBaudRate(QSerialPort::Baud9600);
        } else if (ui->cbox_baud->currentText()=="19200")
        {
            external->setBaudRate(QSerialPort::Baud19200);
        } else if (ui->cbox_baud->currentText()=="38400")
        {
            external->setBaudRate(QSerialPort::Baud38400);
        } else if (ui->cbox_baud->currentText()=="57600")
        {
            external->setBaudRate(QSerialPort::Baud57600);
        } else if (ui->cbox_baud->currentText()=="115200")
        {
            external->setBaudRate(QSerialPort::Baud115200);
        }

        external->setDataBits((QSerialPort::Data8));
        external->setParity(QSerialPort::NoParity);
        external->setStopBits(QSerialPort::OneStop);
        external->setFlowControl(QSerialPort::NoFlowControl);
        QObject::connect(external, SIGNAL(readyRead()), this, SLOT(readSerial()));
    } else
    {
        ui->btn_getData->setText("Start");
        external->close();

    }


}

void MainWindow::readSerial()
{
    QStringList bufferSplit = serialBuffer.split(",");

    if (bufferSplit.length() < 3)
    {
        serialData = external->readAll();
        serialBuffer += QString::fromStdString((serialData.toStdString()));
    } else
    {
        //bufferSplit[1] is a good value
        double y = bufferSplit[1].toDouble();
        double x = QDateTime::currentDateTimeUtc().toTime_t();
        serialBuffer = "";
        addPoint(x, y);
        plot();
    }
}

void MainWindow::addPoint(double x, double y)
{
    if (qv_time.size()==0){
        t0 = QDateTime::currentDateTimeUtc().toTime_t();
    }
    qv_time.append(x-t0);
    qv_signal.append(y);
    ui->timeLabel->setText(QString::number(x-t0, 'f', 2));
    ui->signalLabel->setText(QString::number(y, 'f', 3));


}

void MainWindow::plot()
{
    ui->plotWidget->graph(0)->setData(qv_time, qv_signal);
    double min_y = *std::min_element(qv_signal.begin(), qv_signal.end());
    double max_y = *std::max_element(qv_signal.begin(), qv_signal.end());
    double range_y = (max_y - min_y)/2;

    if (range_y == 0 || range_y == 0.0)
    {
        min_y = min_y - min_y * 0.1;
        max_y = max_y + max_y * 0.1;
    } else
    {
        min_y = min_y - range_y * 0.1;
        max_y = max_y + range_y * 0.1;
    }

    ui->plotWidget->xAxis->setRange(0, *std::max_element(qv_time.begin(), qv_time.end())+1);
    ui->plotWidget->yAxis->setRange(min_y, max_y);

    ui->plotWidget->replot();
    ui->plotWidget->update();
}


void MainWindow::on_btn_clear_clicked()
{
    clearData();
    plot();
}

void MainWindow::clearData()
{
    qv_time.clear();
    qv_signal.clear();
    ui->timeLabel->setText("-");
    ui->signalLabel->setText("-");
}


void MainWindow::on_btn_saveData_clicked()
{
    QString file_name = QFileDialog::getSaveFileName(this, "Choose a File", "C://");
    QFile file(file_name);

    if (file.open(QFile::WriteOnly | QFile::Text)){
        QTextStream stream(&file);
        stream << "Time (s)" << "," << "Signal\n";
        for (int i=0; i< qv_time.size(); i++){
            stream << qv_time[i] << "," << qv_signal[i] << "\n";
        }

    }

    file.close();
}

