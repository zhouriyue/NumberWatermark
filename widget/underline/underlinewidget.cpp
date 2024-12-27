#include "underlinewidget.h"
#include "ui_underlinewidget.h"

UnderlineWidget::UnderlineWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UnderlineWidget)
{
    ui->setupUi(this);
}

UnderlineWidget::~UnderlineWidget()
{
    delete ui;
}
