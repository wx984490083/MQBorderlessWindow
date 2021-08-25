#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : AppWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // set custom caption for windows app
#ifdef Q_OS_WIN
    // you need to implement your caption widget, and replace 'QWidget' here ~~
    auto captionOnWindows = new QWidget();
    captionOnWindows->resize(1, 24);
    setCustomCaption(captionOnWindows, true, captionOnWindows->height());
#endif

    // setup the content widget
    auto contentWidget = new QWidget(this);
    auto palette = contentWidget->palette();
    palette.setColor(QPalette::Window, Qt::white);
    contentWidget->setPalette(palette);
    contentWidget->setAutoFillBackground(true);
    setContent(contentWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

