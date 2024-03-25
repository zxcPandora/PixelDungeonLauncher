#ifndef LOADING_H
#define LOADING_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QFrame>
#include <QMovie>

namespace Ui {
class Loading;
}

class Loading : public QWidget
{
    Q_OBJECT

public:
    explicit Loading(QWidget *parent = nullptr);
    ~Loading();

private:
    Ui::Loading *ui;
    QMovie *movie;
    QLabel *label;
    QLabel * tip_label;
    QFrame * background;
    QTimer *timer;
};

#endif // LOADING_H
