// Definition of methods for the MainWindow class

#include "mainwindow.h"
#include "ui_mainwindow.h"


// Constructor of the MainWindow class
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initial set up of the plot widget
    ui->plotWidget->setInteraction(QCP::iRangeDrag, true);
    ui->plotWidget->setInteraction(QCP::iRangeZoom, true);
    ui->plotWidget->addGraph();
    ui->plotWidget->graph(0)->setScatterStyle(QCPScatterStyle::ssCircle);
    ui->plotWidget->xAxis->setLabel("Time (s)");
    ui->plotWidget->yAxis->setLabel("Signal");

    // The serial port is instantiated
    external = new QSerialPort(this);

    // Initialize the QString serialBuffer, used later on to store read serial data
    serialBuffer = "";

    // Look for available serial ports and populate the Combo Box cbox_ports with them
    foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        ui->cbox_ports->addItem(serialPortInfo.portName());
    }

    // Populate the Combo Box cbox_baud with all baud rates allowed in Qt
    ui->cbox_baud->addItems({"1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200"});
}

// Destructor of the MainWindow class
MainWindow::~MainWindow()
{
    // Before closing the ui, it closes the serial port (if it is opened)
    if (external->isOpen())
    {
        external->close();
    }
    delete ui;
}


// Method to be executed if the push button btn_getData is clicked
void MainWindow::on_btn_getData_clicked()
{
    // btn_getData is a checkeable push button.
    if(ui->btn_getData->isChecked())
    {
        // If the button is initially not checked i.e., its label reads "Start",
        // its label will change to "Stop"
        // the port name will be obtained from that selected in the Combo Box cbox_ports
        // the serial port is opened
        // the baud rate will be obtained from that selected in the Combo Box cbox_baud
        // all other serial port parameters are fixed
        // Finally, the QSerialPort pointer is connected to the slot readSerial()

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
        // If the button is initially checked i.e., its label reads "Stop",
        // its label will change to "Start"
        // the serial port is closed
        ui->btn_getData->setText("Start");
        external->close();

    }
}

// Private method (slot) that reads the serial port when the push button
// btn_getData is checked
//
// This is the most hard coded function in the software:
// It is set for serials that transmit double numbers separated by a comma.
// It also has the specificity that it will only registed one every two values.
// The reason is the tendency of arduino-like microcontrollers to transmit a weird first read value
// The solution I found for this was to:
// 1- store in a buffer sets of two values
// 2- selected for these buffers of two values the last one
void MainWindow::readSerial()
{
    QStringList bufferSplit = serialBuffer.split(",");

    if (bufferSplit.length() < 3)
    {
        // if the buffer contains less than 3 values, keep storing read values in the buffer (as QStrings)
        serialData = external->readAll();
        serialBuffer += QString::fromStdString((serialData.toStdString()));

    } else
    {
        // if the buffer contains 3 values (2 numerical ones plus a comma):
        // 1- select the second one i.e., bufferSplit[1]
        // 2- transform this parameter, a QString, to a double
        // 3- get the current time (a double, in seconds)
        // 4- clean the buffer
        // 5- add the time and signal values to the QVectors qv_time and qv_signal by calling addPoint()
        // 6- plot the data
        double y = bufferSplit[1].toDouble();
        double x = QDateTime::currentDateTimeUtc().toTime_t();
        serialBuffer = "";
        addPoint(x, y);
        plot();
    }
}

// Public method that adds double values for the time and the registered signal
// to the QVectors qv_time and qv_signal
void MainWindow::addPoint(double x, double y)
{
    // If it is the first read value, it initiates the time offset (t0)
    if (qv_time.size()==0){
        t0 = QDateTime::currentDateTimeUtc().toTime_t();
    }

    // It appends the time passed with respect to the offset t0
    qv_time.append(x-t0);

    qv_signal.append(y);

    // Updates the labels timeLabel and signalLabel with the most recent values
    ui->timeLabel->setText(QString::number(x-t0, 'f', 2));
    ui->signalLabel->setText(QString::number(y, 'f', 3));
}

// Public method to plot the QVectors qv_time and qv_signal in plotWidget
void MainWindow::plot()
{
    ui->plotWidget->graph(0)->setData(qv_time, qv_signal);

    // updates the plot range for better view
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

// Method to be executed if the push button btn_clear is clicked
//
// It empties the QVectors qv_time and qv_signal and clears the plotWidget
void MainWindow::on_btn_clear_clicked()
{
    clearData();
    plot();
}

// Method, call when clicking btn_clear, that:
// 1- Clears the QVectors qv_time and qv_signal
// 2- Clears the text form the labels timeLabel and signalLabel
void MainWindow::clearData()
{
    qv_time.clear();
    qv_signal.clear();
    ui->timeLabel->setText("-");
    ui->signalLabel->setText("-");
}

// Method to be executed if the push button btn_saveData is clicked
//
// Saves the QVectors qv_time and qv_signal in a csv file
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

