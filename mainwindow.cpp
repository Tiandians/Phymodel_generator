#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QTextStream>
#include <QTime>
#include <QString>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <QProcess>
#include <windows.h>
#include <QThread>
#include <ctime>
using namespace std;

//物理常数
#define Phy_R 8.314462618
#define Phy_NA 6.02214076e23
#define Phy_kB 1.380649e-23
#define Phy_Vm 22.71095464e-3
constexpr double Oxy_r = 0.173e-9,
                 Oxy_m = 5.32e-26,
                 Hyd_r = 144.5e-12,
                 Hel_r = 130e-12,
                 Nit_r = 182e-12;
//预分配变量
bool data_is_gas;
int pNum;
QString num, outputFile, algName;
const QString progDir(QDir::toNativeSeparators(QDir::currentPath()).append("\\program"));
const QString workDir(QDir::toNativeSeparators(QDir::currentPath()));
static std::default_random_engine e;

//Qt主窗口
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    curFile = new QLabel;
    curFile->setMinimumWidth(150);
    ui->statusBar->addWidget(curFile);
    connect(ui->algTime, SIGNAL(clicked()), this, SLOT(setProgFile()));
    connect(ui->algEvent, SIGNAL(clicked()), this, SLOT(setProgFile()));
    connect(ui->algIndex, SIGNAL(clicked()), this, SLOT(setProgFile()));
    //default config
    ui->textGenDir->setText(QDir::toNativeSeparators(QDir::currentPath().append("\\data")));
    ui->textOutputDir->setText(QDir::toNativeSeparators(QDir::currentPath().append("\\result")));
    ui->textDataFile->setText(workDir + QString("\\data\\ball_8_default.txt"));
    checkDataFile();
    ui->algTime->setChecked(1);
    setProgFile();
    ui->tabWidget->setCurrentIndex(2);
}

MainWindow::~MainWindow()
{
    delete ui;
}


//工具函数
bool MainWindow::generator_ball()
{
    e.seed(time(0));
    QString genFileName;

    bool bRadiumR = ui->bRadiumR->isChecked(),
         bMassR = ui->bMassR->isChecked(),
         bVelocityR = ui->bVelocityR->isChecked();
    int bNum = ui->bNum->value(),
        bN = pow(bNum, 3);
    double bRadium = ui->bRadium->value(),
           bMass = ui->bMass->value(),
           bVelocity = ui->bVelocity->value(),
           bDistance = ui->bDistance->value(),
           bX0 = ui->cLocX->value() - (bNum - 1) / 2.0 * bDistance,
           bY0 = ui->cLocY->value() - (bNum - 1) / 2.0 * bDistance,
           bZ0 = ui->cLocZ->value() - (bNum - 1) / 2.0 * bDistance;
    static std::uniform_real_distribution<double> uV(-bVelocity, bVelocity);
    static std::normal_distribution<double> nV(bVelocity, 1);
    static std::uniform_real_distribution<double> uM(0, bMass);
    static std::uniform_real_distribution<double> uR(0, bRadium);
    glm::dvec3 random(glm::sphericalRand(bVelocity));
    double bVx = bVelocityR ? uV(e) : random.x,
           bVy = bVelocityR ? uV(e) : random.y,
           bVz = bVelocityR ? uV(e) : random.z;
    bMass = bMassR ? uM(e) : bMass;
    bRadium = bRadiumR ? uR(e) : bRadium;

    //生成文件名
    QTextStream(&genFileName) << "/ball_" << bN << "_";
    genFileName.append(QDateTime::currentDateTime().toString("hhmmss") + QString(".txt")).prepend(QDir::fromNativeSeparators(ui->textGenDir->text()));
    ui->textDataFile->setText(QDir::toNativeSeparators(genFileName));
    checkDataFile();
    setProgFile();
    QFile genFile(genFileName);
    /*
    if (genFile.exists()){
        QMessageBox:: StandardButton result = QMessageBox::information(this, "生成", "数据文件已存在，是否覆盖？", QMessageBox::Ok, QMessageBox::Cancel);
        switch (result)
        {
        case QMessageBox::Ok:
            break;
        case QMessageBox::Cancel:
            goto BALLEND;
            break;
        default:
            goto BALLEND;
            break;
        }
    }//一般情况下生成文件不可能重名*/
    if (!genFile.open(QIODevice::Truncate | QIODevice::Text | QIODevice::WriteOnly))
    {
        QMessageBox::information(this, "生成", "生成文件打开失败，请检查路径或资源占用", QMessageBox::Ok, QMessageBox::NoButton);
        return false;
    }
    QTextStream genStrm(&genFile);

    //控制
    genStrm << ui->if_gl->isChecked() << '\t'
            << ui->if_output->isChecked() << '\t'
            << ui->if_fps->isChecked() << '\t'
            << ui->if_info->isChecked() << Qt::endl;
    //相机
    genStrm << ui->cLocX_2->value() << '\t'
            << ui->cLocY_2->value() << '\t'
            << ui->cLocZ_2->value() << '\t'
            << ui->cX_2->value() << '\t'
            << ui->cY_2->value() << '\t'
            << ui->cZ_2->value() << '\t'
            << ui->cRot_v->value() << '\t'
            << ui->dispRate->value() << Qt::endl;
    //系统
    //容器
    if (ui->if_c->isChecked())
    {
        genStrm << 'C' << '\t' << '1' << Qt::endl;
        genStrm << ui->cLocX->value() << '\t'
                << ui->cLocY->value() << '\t'
                << ui->cLocZ->value() << '\t'
                << ui->cX->value() << '\t'
                << ui->cY->value() << '\t'
                << ui->cZ->value() << Qt::endl;
    }
    //球
    genStrm << 'B' << '\t' << bN << Qt::endl;
    for (int i = 0; i != bNum; i++)
        for (int j = 0; j != bNum; j++)
            for (int k = 0; k != bNum; k++)
            {
                genStrm << bX0 + i * bDistance << '\t'
                        << bY0 + j * bDistance << '\t'
                        << bZ0 + k * bDistance << '\t'
                        << bVx << '\t'
                        << bVy << '\t'
                        << bVz << '\t'
                        << bMass << '\t'
                        << bRadium << '\t' << Qt::endl;
                bVx = bVelocityR ? uV(e) : (random = glm::sphericalRand(bVelocity)).x;
                bVy = bVelocityR ? uV(e) : random.y;
                bVz = bVelocityR ? uV(e) : random.z;
                bMass = bMassR ? uM(e) : bMass;
                bRadium = bRadiumR ? uR(e) : bRadium;
            }

    genFile.close();
    QMessageBox::information(this, "生成", "数据文件已生成", QMessageBox::Ok, QMessageBox::NoButton);
    curFile->setText(QString("生成了球体系统文件：").append(ui->textDataFile->text()));
    return true;
}

bool MainWindow::generator_gas()
{
    e.seed(time(0));
    QString genFileName;

    double gm, gr;
    switch (ui->gType->currentIndex())
    {
    //case 0:{ gm = Hyd_m;gr = Hyd_r;}
    default:
    case 3:
    {
        gm = Oxy_m;
        gr = Oxy_r;
        break;
    }
    }

    double gLen = ui->gLen->value() * 1e-6,
           gp = ui->gPre->value(),
           gV = pow(gLen, 3),
           gT = ui->gTem->value(),
           gDis = pow(Phy_R * gT / Phy_NA / gp, 1.0 / 3.0), gDisX, gDisY, gDisZ,
           sigma = pow(Phy_kB * gT / gm, 0.5);
    gDisX = gDisY = gDisZ = gDis;
    static std::normal_distribution<double> uV(0, sigma);
    int gN = gp * gV * Phy_NA / Phy_R / gT,
        gNum = gLen / gDis, gNumX, gNumY, gNumZ;
    gNumX = gNumY = gNumZ = gNum;

    //气体分子个数自动调适
    if (gNum * gNum * gNum < gN)
    {
        gNumX++;
        gDisX = (gLen - 2.1 * gr) / gNumX;
        if (gNumX * gNumY * gNumZ < gN)
        {
            gNumY++;
            gDisY = gDisX;
            if (gNumX * gNumY * gNumZ < gN)
            {
                gNumZ++;
                gDisZ = gDisX;
            }
        }
        if (gNumX * gNumY * gNumZ < gN)
        {
            QMessageBox::information(this, "生成", "气体错误", QMessageBox::Ok, QMessageBox::NoButton);
            exit(0);
        }

        QString information;
        QTextStream(&information) << "气体生成偏差，自动纠正:气体分子数：" << gN << "补位生成数：" << gNumX * gNumY * gNumZ << "气体分子呈现不完整矩阵分布";
        QMessageBox::information(this, "生成", information, QMessageBox::Ok, QMessageBox::NoButton);
    }
    double gX0 = 0 - (gNumX - 1) / 2.0 * gDisX,
           gY0 = 0 - (gNumY - 1) / 2.0 * gDisY,
           gZ0 = 0 - (gNumZ - 1) / 2.0 * gDisZ;

    //生成文件名
    QTextStream(&genFileName) << "/gas_" << gN << "_";
    genFileName.append(QDateTime::currentDateTime().toString("hhmmss") + QString(".txt")).prepend(QDir::fromNativeSeparators(ui->textGenDir->text()));
    QFile genFile(genFileName);
    if (!genFile.open(QIODevice::Truncate | QIODevice::Text | QIODevice::WriteOnly))
    {
        QMessageBox::information(this, "生成", "生成文件打开失败，请检查路径或资源占用", QMessageBox::Ok, QMessageBox::NoButton);
        return false;
    }
    ui->textDataFile->setText(QDir::toNativeSeparators(genFileName));
    checkDataFile();
    setProgFile();
    QTextStream genStrm(&genFile);
    genStrm.setRealNumberPrecision(10);
    genStrm.setRealNumberNotation(QTextStream::ScientificNotation);

    //gas省去Line1-2
    genStrm << ui->gTime->text() << Qt::endl;
    //系统
    //容器
    genStrm << 'C' << '\t' << '1' << Qt::endl;
    genStrm << 0 << '\t'
            << 0 << '\t'
            << 0 << '\t'
            << gLen << '\t'
            << gLen << '\t'
            << gLen << Qt::endl;
    //分子
    genStrm << 'B' << '\t' << gN << Qt::endl;
    for (int i = 0; i != gNumX; i++)
    {
        int igNum = i * gNumY * gNumZ;
        for (int j = 0; j != gNumY; j++)
        {
            int jgNum = j * gNumZ;
            for (int k = 0; k != gNumZ; k++)
            {
                if (igNum + jgNum + k == gN)
                    goto gOUT;
                genStrm << gX0 + i * gDisX << '\t'
                        << gY0 + j * gDisY << '\t'
                        << gZ0 + k * gDisZ << '\t'
                        << uV(e) << '\t'
                        << uV(e) << '\t'
                        << uV(e) << '\t'
                        << gm << '\t'
                        << gr << '\t' << Qt::endl;
            }
        }
    }
gOUT:;
    QMessageBox::information(this, "生成", "数据文件已生成", QMessageBox::Ok, QMessageBox::NoButton);
    curFile->setText(QString("生成了气体文件：").append(ui->textDataFile->text()));

    return true;
}

void MainWindow::setProgFile()
{

    if (data_is_gas)
    {
        algName = "gas.exe";
    }
    else
    {
        if (ui->algTime->isChecked())
            algName = "time.exe";
        else if (ui->algEvent->isChecked())
            algName = "event.exe";
        else if (ui->algIndex->isChecked())
            algName = "index.exe";
    };
    ui->textRunFile->setText(QDir::toNativeSeparators(progDir).append("\\").append(algName));
    curFile->setText(QString("程序位置：").append(ui->textRunFile->text()));
}

void MainWindow::checkDataFile()
{
    if (ui->textDataFile->text().contains("\\gas_"))
    {
        num = ui->textDataFile->text().section("_", 1, 1);

        QString info("理想气体数据\n");
        info.append("分子个数：").append(num);
        ui->textDataInfo->setText(info);
        ui->algTime->setChecked(0);
        ui->algTime->setCheckable(0);
        ui->algEvent->setChecked(0);
        ui->algEvent->setCheckable(0);
        ui->algIndex->setChecked(0);
        ui->algIndex->setCheckable(0);
        ui->btnRun->setCheckable(1);
        ui->gTime->setReadOnly(0);
        data_is_gas = true;
    }
    else if (ui->textDataFile->text().contains("\\ball_"))
    {
        num = ui->textDataFile->text().section("_", 1, 1);
        QString info("球体系统数据\n");
        info.append("球体个数：").append(num);
        ui->textDataInfo->setText(info);
        ui->algTime->setCheckable(1);
        ui->algTime->setChecked(1);
        ui->algEvent->setCheckable(1);
        ui->algIndex->setCheckable(1);
        ui->btnRun->setCheckable(1);
        ui->gTime->setReadOnly(1);
        data_is_gas = false;
    }
    else
    {
        ui->textDataInfo->setText("数据文件识别失败");
        ui->btnRun->setCheckable(0);
    }
}

//生成选项卡
//控制可选的生成参数
void MainWindow::on_tabGenArgs_currentChanged(int index)
{
    if (index == 0)
    {
        ui->if_gl->setCheckable(1);
        ui->if_fps->setCheckable(1);
        ui->cLocX_2->setEnabled(1);
        ui->cLocY_2->setEnabled(1);
        ui->cLocZ_2->setEnabled(1);
        ui->cX_2->setEnabled(1);
        ui->cY_2->setEnabled(1);
        ui->cZ_2->setEnabled(1);
        ui->cRot_v->setEnabled(1);
        ui->dispRate->setEnabled(1);
    }
    if (index == 1)
    {
        ui->if_gl->setChecked(0);
        ui->if_gl->setCheckable(0);
        ui->if_output->setChecked(1);
        ui->if_fps->setChecked(0);
        ui->if_fps->setCheckable(0);
        ui->if_info->setChecked(1);
        ui->cLocX_2->setEnabled(0);
        ui->cLocY_2->setEnabled(0);
        ui->cLocZ_2->setEnabled(0);
        ui->cX_2->setEnabled(0);
        ui->cY_2->setEnabled(0);
        ui->cZ_2->setEnabled(0);
        ui->cRot_v->setEnabled(0);
        ui->dispRate->setEnabled(0);
    }
}

void MainWindow::on_btnOpenGenFile_clicked()
{
    ui->textGenDir->setText(QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, "选择生成文件夹", QDir::currentPath())));
}

void MainWindow::on_btnGen_clicked()
{
    if (ui->tabGenArgs->currentIndex() == 0)
        generator_ball();
    else
        generator_gas();
}

//启动选项卡
//数据
void MainWindow::on_btnDataFile_clicked()
{
    ui->textDataFile->setText(QDir::toNativeSeparators(QFileDialog::getOpenFileName(this, "选择数据文件", QDir::currentPath(), QString("文本文件(*.txt)"))));
    checkDataFile();
    setProgFile();
}

void MainWindow::on_textDataFile_editingFinished()
{
    checkDataFile();
    setProgFile();
}

//输出
void MainWindow::on_btnOutputDir_clicked()
{
    ui->textOutputDir->setText(QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, "选择输出文件夹", QDir::currentPath())));
}

//运行
void MainWindow::on_btnRun_clicked()
{
    outputFile = QDir::toNativeSeparators(ui->textOutputDir->text().append("\\" + algName).append(QDateTime::currentDateTime().toString("hhmmss") + ".txt"));
    QMessageBox::information(this, "启动", ui->textRunFile->text(), QMessageBox::Ok, QMessageBox::NoButton);

    QProcess *program = new QProcess(this);
    program->setWorkingDirectory(QDir::fromNativeSeparators(progDir));

    if(data_is_gas)
        program->start(ui->textRunFile->text(), QStringList() << QDir::toNativeSeparators(ui->textDataFile->text()) << num << outputFile << ui->gTime->text());
    else
        program->start(ui->textRunFile->text(), QStringList() << QDir::toNativeSeparators(ui->textDataFile->text()) << num << outputFile);

    if (program->state() == QProcess::NotRunning)
    {
        QMessageBox::information(this, "启动", "程序未启动，请检查配置", QMessageBox::Ok, QMessageBox::NoButton);
    }

    program->waitForFinished();

    QFile output(outputFile);
    if (!output.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        QMessageBox::information(this, "输出", "输出文件打开失败，请检查路径或资源占用", QMessageBox::Ok, QMessageBox::NoButton);
    }
    QTextStream outStrm(&output);
    ui->textRunInfo->setText(outStrm.readAll());
}

void MainWindow::on_btnRunExp_clicked()
{
    if(ui->exp1->isChecked())
        algName = "exp1.exe";
    else if(ui->exp2->isChecked())
        algName = "exp2.exe";
    else if(ui->exp3->isChecked())
        algName = "exp3.exe";
    else
        QMessageBox::information(this, "实验", "请选择实验", QMessageBox::Ok, QMessageBox::NoButton);
    outputFile = QDir::toNativeSeparators(ui->textOutputDir->text().append("\\" + algName).append(QDateTime::currentDateTime().toString("hhmmss") + ".txt"));
    QString expFile(progDir+QString("\\")+algName);
    QMessageBox::information(this, "启动", expFile, QMessageBox::Ok, QMessageBox::NoButton);

    QProcess *program = new QProcess(this);
    program->setWorkingDirectory(QDir::fromNativeSeparators(progDir));
    program->start(expFile, QStringList() << outputFile);

    if (program->state() == QProcess::NotRunning)
    {
        QMessageBox::information(this, "启动", "程序未启动，请检查配置", QMessageBox::Ok, QMessageBox::NoButton);
    }
    program->waitForFinished();
    QFile output(outputFile);
    if (!output.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        QMessageBox::information(this, "输出", "输出文件打开失败，请检查路径或资源占用", QMessageBox::Ok, QMessageBox::NoButton);
    }
    QTextStream outStrm(&output);
    ui->textRunInfo->setText(outStrm.readAll());
}


void MainWindow::on_tabWidget_currentChanged(int index)
{
    switch(index)
    {
    case 0:
        curFile->setText(QString("数据文件生成路径：").append(ui->textGenDir->text()));
        break;
    case 2:
        setProgFile();
        break;
    case 1:
        curFile->setText(QString("实验程序路径:").append("todo"));
        break;
    case 3:
        curFile->setText(QString("运行状态文件：").append(outputFile));
        break;
    default:
        break;
    }
}

