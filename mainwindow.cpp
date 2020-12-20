#include "mainwindow.hh"
#include "ui_mainwindow.h"


const int UNCHECKED = 0;
const double DEDUCTION = 750.0;
const double YLETAX = 0.025;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Palkkalaskuri");

    ui->ageComboBox->addItem ("17-52");
    ui->ageComboBox->addItem ("52-62");
    ui->ageComboBox->addItem ("yli 62");

    ui->monthlyLineEdit->setEnabled(false);

    readFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::readFile(){

    QFile inputFile("../SalaryCalculator/taxes.txt");
    if (inputFile.open(QIODevice::ReadOnly))
    {
      QTextStream in(&inputFile);
      while (!in.atEnd())
      {
        QString line = in.readLine();

        QString municipality = line.split(";").at(0);
        QString tax = line.split(";").at(1);
        QString churchTax = line.split(";").at(2);

        taxes[municipality] = {tax.toDouble(), churchTax.toDouble()};

        ui->municipalitiesComboBox->addItem (municipality);
        ui->percentageLabel->setText(tax + " %");

      }
      inputFile.close();
    }
}

void MainWindow::countByMonthly()
{
    QString total = ui->monthlyLineEdit->text();

    if(!isNumber(total.toStdString())){
        return;
    }
    ui->errorTextLabel->setText("");

    ui->monthlyWageLabel->setText(total + " €");
    ui->yearlyWageLabel->setText(total + " €");

    double totalYearly = total.toDouble()*12;

    calculateTaxes(totalYearly);
}

void MainWindow::countByHourly()
{
    if(!isNumber((ui->hourlyLineEdit->text().toStdString())) ||
            !isNumber(ui->hoursLineEdit->text().toStdString())){
        return;
    }
    ui->errorTextLabel->setText("");

    double hourlyWage = ui->hourlyLineEdit->text().toDouble();
    double hours = ui->hoursLineEdit->text().toDouble();
    QString minMonthlyWage = QString::number(4*hourlyWage*hours);
    QString maxMonthlyWage = QString::number(4.4*hourlyWage*hours);
    ui->monthlyWageLabel->setText(minMonthlyWage + " -  " + maxMonthlyWage + " €");

    double totalYearly = hourlyWage*hours*52;

    calculateTaxes(totalYearly);
}

void MainWindow::calculateTaxes(double totalYearly)
{
    ui->yearlyWageLabel->setText(QString::number(totalYearly) + " €");
    double yearlyAfterDeduction = totalYearly - DEDUCTION;

    double taxesPaid = 0.0;

    // yle-vero
    taxesPaid += (yearlyAfterDeduction - 14000) * YLETAX;
    if (taxesPaid > 163) {
        taxesPaid = 163.0;
    }
    else if (taxesPaid < 0) {
        taxesPaid = 0;
    }
    ui->yleTaxLabel->setText(QString::number(taxesPaid, 'f', 2) + " €");

    double TyEL = totalYearly * TyELpercent;
    ui->TyELLabel->setText(QString::number(TyEL, 'f', 2) + " €");

    double unemp = totalYearly * unemploymentPercent;
    ui->unemploymentLabel->setText(QString::number(unemp, 'f', 2) + " €");

    double healthcare = totalYearly* healthcarePercent;
    ui->healthcareLabel->setText(QString::number(healthcare, 'f', 2) + " €");

    double yearlyBeforeTaxes = yearlyAfterDeduction - TyEL- unemp - healthcare;


    // Valtion tulovero
    double incomeTa = incomeTax(yearlyBeforeTaxes, totalYearly, yearlyAfterDeduction);


    // Kunnallisvero
    double deduction = (7230-2500) * 0.51 + (yearlyAfterDeduction - 7230)*0.28;
    if(deduction > 3570){
        if(yearlyAfterDeduction > 14000 && yearlyAfterDeduction < 93333){
            deduction = 3570-(yearlyAfterDeduction-14000)*0.045;
        }
        else if(yearlyAfterDeduction > 93333){
            deduction = 0;
        }
        else{
            deduction = 3570;
        }
    }

    QString currentMunicipality = ui->municipalitiesComboBox->currentText();
    double municipalityTaxAmount = (yearlyBeforeTaxes-deduction)*(taxes.at(currentMunicipality).first/100);
    ui->municipalityTaxLabel->setText(QString::number((municipalityTaxAmount))+ " €");

    double churchTaxAmount = (yearlyBeforeTaxes-deduction)*(taxes.at(currentMunicipality).second/100);
    ui->churchTaxAmountLabel->setText(QString::number((churchTaxAmount))+ " €");


    double temp = (totalYearly - 2500) * 0.125;

    if(temp > 1770){
        temp = 1770 -(yearlyAfterDeduction - 33000) * 0.0172;
    }

    double yearlyNet = yearlyBeforeTaxes - (incomeTa-temp) - municipalityTaxAmount - taxesPaid +  DEDUCTION;
    ui->yearlyNetLabel->setText(QString::number((yearlyNet))+ " €");
    ui->monthlyNetLabel->setText(QString::number((yearlyNet/12))+ " €");
}

double MainWindow::incomeTax(double yearlyBeforeTaxes, double totalYearly, double yearlyAfterDeduction)
{
    double incomeTax = 0;
    if (yearlyBeforeTaxes < 18100) {
        incomeTax += 0.0;
    }
    else if (yearlyBeforeTaxes > 18100 && yearlyBeforeTaxes < 27200) {
        incomeTax =  8.0 + 0.06 * (yearlyBeforeTaxes - 18100);
    }
    else if (yearlyBeforeTaxes > 27200 && yearlyBeforeTaxes < 44800) {
        incomeTax = 536.0 + 0.1725 * (yearlyBeforeTaxes - 27200);
    }
    else if (yearlyBeforeTaxes > 44800 && yearlyBeforeTaxes < 78500) {
        incomeTax = 3590.0 + 0.2125 * (yearlyBeforeTaxes - 44800);
    }
    else if (yearlyBeforeTaxes > 78500) {
        incomeTax = 10751.25 + 0.3125 * (yearlyBeforeTaxes - 78500);
    }

    double temp = (totalYearly - 2500) * 0.125;

    if(temp > 1770){
        if(yearlyAfterDeduction > 33000){
            temp = 1770 -(yearlyAfterDeduction - 33000) * 0.0184;
        }
        else{
            temp = 1770;
        }
    }

    if(incomeTax - temp < 0){
        incomeTax = 0;
    }
    else{
        incomeTax = incomeTax - temp;
    }

    ui->incomeTaxLabel->setText(QString::number(incomeTax, 'f', 2) + " €");

    return incomeTax;
}

bool MainWindow::isNumber(std::string str)
{
    bool is_a_number = false;
    try
    {
       boost::lexical_cast<double>(str);
       is_a_number = true;
    }
    catch(boost::bad_lexical_cast &)
    {
       ui->errorTextLabel->setText("VIRHE! Syötä arvot desimaaleina pisteellä erotettuna!");
       ui->errorTextLabel->setStyleSheet("QLabel {color : red; }");
    }
    return is_a_number;
}

void MainWindow::on_monthlyRadioButton_clicked()
{
    //Enable and disable right lineEdit boxes
    ui->monthlyLineEdit->setEnabled(true);
    ui->hourlyLineEdit->setEnabled(false);
    ui->hoursLineEdit->setEnabled(false);

    isHourly = false;
}

void MainWindow::on_hourlyRadioButton_clicked()
{
    //Enable and disable right lineEdit boxes
    ui->monthlyLineEdit->setEnabled(false);
    ui->hourlyLineEdit->setEnabled(true);
    ui->hoursLineEdit->setEnabled(true);

    isHourly = true;
}

void MainWindow::on_calculatePushButton_clicked()
{
    if(isHourly){
        countByHourly();
    }
    else{
        countByMonthly();
    }
}

void MainWindow::on_municipalitiesComboBox_currentTextChanged(const QString &arg1)
{
    QString taxPercentage = QString::number(taxes.at(arg1).first) + " %";
    municipalityTax = taxPercentage.toDouble();
    ui->percentageLabel->setText(taxPercentage);
}

void MainWindow::on_checkBox_stateChanged(int state)
{
    if(state == UNCHECKED){
        ui->churchTaxLabel->setText("");
    }
    else{

        QString currentMunicipality = ui->municipalitiesComboBox->currentText();
        QString churchTaxString = "Kirkollisveroprosentti: " +
                QString::number(taxes.at(currentMunicipality).second) + " %";
        churchTax = taxes.at(currentMunicipality).second;
        ui->churchTaxLabel->setText(churchTaxString);
    }
}

void MainWindow::on_ageComboBox_currentTextChanged(const QString &ageGroup)
{
    if(ageGroup == "17-52" || ageGroup == "yli 62"){
        TyELpercent = 0.0715;
    }
    else if(ageGroup == "53-62"){
        TyELpercent = 0.0865;
    }
}
