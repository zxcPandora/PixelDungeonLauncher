#include "loading.h"
#include "ui_loading.h"
#include <QScreen>

Loading::Loading(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Loading)
{
    ui->setupUi(this);
    this->setFixedSize(200,150);

    background = new QFrame(this);
    background->setStyleSheet("background-color:#fff;border-radius:10px;");
    background->setGeometry(0, 0, 200, 150);
    ui->label = new QLabel(background);
    ui->label->setGeometry(0, 0, 200, 150);
    ui->label->setText("aaaaaaaaaaa");
    movie = new QMovie("loding.gif");
    //movie->setFileName(":/loding.gif");
    movie->setScaledSize(QSize(200,150));
    ui->label->setScaledContents(true);
    ui->label->setMovie(movie);
    movie->start();
    qDebug()<<"loading";
}

Loading::~Loading()
{
    delete background;
    delete label;
    delete movie;
    delete ui;
}
