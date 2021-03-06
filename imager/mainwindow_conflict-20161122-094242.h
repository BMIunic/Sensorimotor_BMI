/*************************************************************
/IMAGER (2016)
/mainwindow.h
/Decleration of function used in mainwindow.cpp
/Automatically generated by Qt
/Author: Aamir Abbasi
**************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QDebug>
#include <QObject>
#include <QProgressBar>
#include "imagegrab.h"
#include "fileio.h"
#include "imagesequence.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    QStringList pathList;

    //Trial counters initialisation for every whisker
    int counter_Alpha = 0;
    int counter_A1 = 0;
    int counter_A2 = 0;
    int counter_A3 = 0;
    int counter_A4 = 0;
    int counter_Beta = 0;
    int counter_B1 = 0;
    int counter_B2 = 0;
    int counter_B3 = 0;
    int counter_B4 = 0;
    int counter_Gamma = 0;
    int counter_C1 = 0;
    int counter_C2 = 0;
    int counter_C3 = 0;
    int counter_C4 = 0;
    int counter_Delta = 0;
    int counter_D1 = 0;
    int counter_D2 = 0;
    int counter_D3 = 0;
    int counter_D4 = 0;
    int counter_E1 = 0;
    int counter_E2 = 0;
    int counter_E3 = 0;
    int counter_E4 = 0;


private slots:

    void on_Load_Configuration_triggered();

    void on_Grab_Single_clicked();

    void on_Grab_Sequence_clicked();

    void on_progressBar_numberTrials_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    QThread *thread;
    ImageSequence *worker;
};

#endif // MAINWINDOW_H
