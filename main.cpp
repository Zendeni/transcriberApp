#include <QApplication>
#include <QPushButton>
#include <QFileDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include <QProcess>
#include <QDebug>

#include "deepspeech.h"
#include <fstream>
#include <vector>

// Function to convert audio to 16kHz WAV using SoX
bool convertAudioToWav(const QString &inputFile, const QString &outputFile) {
    QProcess process;
    QStringList arguments;
    arguments << inputFile << "-r" << "16000" << outputFile;

    process.start("sox", arguments);
    process.waitForFinished();
    return process.exitCode() == 0;
}

// Function to read the WAV file and convert it to a format DeepSpeech understands
std::vector<int16_t> readWavFile(const QString &filename) {
    std::ifstream file(filename.toStdString(), std::ios::binary);
    file.seekg(44); // Skip WAV header

    std::vector<int16_t> audioData;
    int16_t sample;
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(sample))) {
        audioData.push_back(sample);
    }
    return audioData;
}

QString transcribeAudio(const QString &modelPath, const QString &scorerPath, const std::vector<int16_t> &audioData) {
    ModelState *model;
    int status = DS_CreateModel(modelPath.toStdString().c_str(), &model);
    if (status != 0) {
        throw std::runtime_error("Failed to create DeepSpeech model");
    }
    status = DS_EnableExternalScorer(model, scorerPath.toStdString().c_str());
    if (status != 0) {
        DS_DestroyModel(model);
        throw std::runtime_error("Failed to enable external scorer");
    }

    const char *result = DS_SpeechToText(model, audioData.data(), audioData.size());
    QString transcription = QString::fromUtf8(result);
    DS_FreeString(result);
    DS_DestroyModel(model);

    return transcription;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("TranscriberApp");

    QVBoxLayout *layout = new QVBoxLayout(&window);

    QPushButton *selectButton = new QPushButton("Select Audio File");
    QTextEdit *textEdit = new QTextEdit();
    QPushButton *transcribeButton = new QPushButton("Transcribe");

    layout->addWidget(selectButton);
    layout->addWidget(textEdit);
    layout->addWidget(transcribeButton);

    QString selectedFile;

    QObject::connect(selectButton, &QPushButton::clicked, [&]() {
        selectedFile = QFileDialog::
