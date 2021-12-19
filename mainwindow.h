#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <QProcess>
#include <QMessageBox>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

/*
class Worker : public QObject
{
    Q_OBJECT

public slots:
    void doWork(const QString &parameter) {


        QString result;
        QFile output(parameter);
        if (!output.open(QIODevice::Text | QIODevice::ReadOnly))
        {
            result = QString("bad result");
            emit resultReady(result);
            return;
        }
        QTextStream outStrm(&output);
        result = outStrm.readAll();
        emit resultReady(result);
    }

signals:
    void resultReady(const QString &result);
};

class Controller : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller() {
        Worker *worker = new Worker;
        worker->moveToThread(&workerThread);
        connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
        connect(this, &Controller::operate, worker, &Worker::doWork);
        connect(worker, &Worker::resultReady, this, &Controller::handleResults);
        workerThread.start();
    }
    ~Controller() {
        workerThread.quit();
        workerThread.wait();
    }
    const QString & read(const QString & readFileName){emit operate(readFileName); return temp;}
public slots:
    void handleResults(const QString & result){ temp = result; }
signals:
    void operate(const QString &);
private:
    QString temp = QString("no temp");
};


    /*解读一下创建Controller时会发生什么：
    1.新建Worker对象，移动线程
    2.链接槽函数，有3个：finished, operate
    3.开启线程
    Controller发送operate信号时，doWork执行。疑问？什么时候发送operate？
    那么现在描述一下我们所希望的程序执行进程：
    1. 主程序以分离方式调用OpenGL程序，主程序与子程序完全无关。OpenGL程序不断向磁盘文件写入输出
    2. OpenGL程序启动后，主程序启动子线程Worker，不断读取OpenGL程序的输出，并将其显示到文件框
    3. 子线程如何结束？以分离方式启动的子程序无法使用qt中的进程交互，因此输出文件需要给出程序运行结束的信息。我们指定"#"符号作为终止信号。当子线程读取到该符号时，子线程离开循环，自动结束
    设计：
    Controller:
    无需与Worker交互，只需创建信号启动Worker，根据信号终止worker
    调用controller.read()，发送信号，Worker子线程槽函数响应，执行循环，controller只剩结束线程的功能

    */


class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QLabel *curFile; //状态栏上显示活动文件

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots: //在这里自定义所有响应函数

    void on_btnOpenGenFile_clicked();

    bool generator_ball();

    bool generator_gas();

    void setProgFile();

    void checkDataFile();

    void on_tabGenArgs_currentChanged(int index);

    void on_btnGen_clicked();

    void on_btnDataFile_clicked();

    void on_btnRun_clicked();

    void on_btnOutputDir_clicked();

    void on_textDataFile_editingFinished();

    void on_btnRunExp_clicked();

    void on_tabWidget_currentChanged(int index);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
