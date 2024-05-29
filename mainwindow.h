#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void selectFile();
    void transcribeFile();

private:
    Ui::MainWindow *ui;
    QPushButton *selectButton;
    QPushButton *transcribeButton;
    QLabel *fileLabel;
    QPlainTextEdit *textEdit;
    QString selectedFile;
    std::string transcribeAudio(const std::string &audioFilePath, const std::string &apiKeyFilePath);
};

#endif // MAINWINDOW_H
