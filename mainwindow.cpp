#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout(this);

    selectButton = new QPushButton("Select Audio File", this);
    connect(selectButton, &QPushButton::clicked, this, &MainWindow::selectFile);

    transcribeButton = new QPushButton("Transcribe", this);
    connect(transcribeButton, &QPushButton::clicked, this, &MainWindow::transcribeFile);

    fileLabel = new QLabel("No file selected", this);

    textEdit = new QPlainTextEdit(this);
    textEdit->setReadOnly(true);

    layout->addWidget(selectButton);
    layout->addWidget(fileLabel);
    layout->addWidget(transcribeButton);
    layout->addWidget(textEdit);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::selectFile() {
    selectedFile = QFileDialog::getOpenFileName(this, "Select Audio File", "", "Audio Files (*.wav *.mp3 *.m4a)");
    if (!selectedFile.isEmpty()) {
        fileLabel->setText("Selected file: " + selectedFile);
    } else {
        fileLabel->setText("No file selected");
    }
}

void MainWindow::transcribeFile() {
    if (selectedFile.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select an audio file first.");
        return;
    }

    try {
        std::string transcript = transcribeAudio(selectedFile.toStdString(), "path/to/your/api_key.json");
        textEdit->setPlainText(QString::fromStdString(transcript));
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Error", e.what());
    }
}

std::string MainWindow::transcribeAudio(const std::string &audioFilePath, const std::string &apiKeyFilePath) {
    // Read API key from the file
    std::ifstream keyFile(apiKeyFilePath);
    if (!keyFile.is_open()) {
        throw std::runtime_error("Could not open API key file.");
    }

    nlohmann::json keyJson;
    keyFile >> keyJson;
    std::string apiKey = keyJson["api_key"];

    // Read audio file content
    std::ifstream audioFile(audioFilePath, std::ios::binary);
    if (!audioFile.is_open()) {
        throw std::runtime_error("Could not open audio file.");
    }
    std::ostringstream audioStream;
    audioStream << audioFile.rdbuf();
    std::string audioContent = audioStream.str();

    // Set up libcurl
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize libcurl.");
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    nlohmann::json requestJson = {
        {"config", {
            {"encoding", "LINEAR16"},
            {"sampleRateHertz", 16000},
            {"languageCode", "en-US"}
        }},
        {"audio", {
            {"content", nlohmann::json::binary_t(audioContent)}
        }}
    };

    std::string requestData = requestJson.dump();
    std::string responseBuffer;

    curl_easy_setopt(curl, CURLOPT_URL, ("https://speech.googleapis.com/v1/speech:recognize?key=" + apiKey).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char *ptr, size_t size, size_t nmemb, std::string *data) {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_cleanup(curl);

    nlohmann::json responseJson = nlohmann::json::parse(responseBuffer);
    return responseJson["results"][0]["alternatives"][0]["transcript"];
}
