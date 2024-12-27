#include "roundimagelabel.h"
#include "ui_roundimagelabel.h"

RoundImageLabel::RoundImageLabel(QWidget *parent) :
    QPushButton (parent),
    ui(new Ui::RoundImageLabel)
{
    ui->setupUi(this);
}

RoundImageLabel::~RoundImageLabel()
{
    delete ui;
}
