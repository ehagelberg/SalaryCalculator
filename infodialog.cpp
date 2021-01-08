#include "infodialog.hh"
#include "ui_infodialog.h"

infoDialog::infoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::infoDialog)
{
    ui->setupUi(this);
    this->setStyleSheet("background-color: white;");
    this->setWindowTitle("Tietoja");
    ui->closeButton->setStyleSheet("background-color: red");
    ui->label->setText("Palkkalaskuri.\nVersio 0.\n2020.");

}

infoDialog::~infoDialog()
{
    delete ui;
}

void infoDialog::on_closeButton_clicked()
{
    this->close();
}
