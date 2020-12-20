#include "mainwindow.hh"
#include "ui_mainwindow.h"

//Const variables for values that don't depend on user input
const int UNCHECKED = 0;
const double DEDUCTION = 750.0;
const double YLETAX = 0.025;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setStyleSheet("background-color: white;");
    this->setWindowTitle("Palkkalaskuri");

    // some initial settings for ui
    ui->ageComboBox->addItem ("17-52");
    ui->ageComboBox->addItem ("53-62");
    ui->ageComboBox->addItem ("yli 62");

    ui->monthlyLineEdit->setEnabled(false);
    ui->exitButton->setStyleSheet("background-color: red");
    ui->calculatePushButton->setStyleSheet("background-color: green");

    readFile();
    municipalTaxPercent = taxes.at(ui->municipalitiesComboBox->currentText()).first;
    ui->percentageLabel->setText(QString::number(municipalTaxPercent)+ " %");
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

      }
      inputFile.close();
    }
}

void MainWindow::countByMonthly()
{
    QString total = ui->monthlyLineEdit->text();

    // check if input is a number
    if(!isNumber(total.toStdString())){
        return;
    }

    // check if input is negative value
    if(total.toDouble() < 0){
        ui->errorTextLabel->setText("VIRHE! Syötä vain positiivisia arvoja!");
        ui->errorTextLabel->setStyleSheet("QLabel {color : red; }");
        return;
    }

    ui->errorTextLabel->setText("");

    ui->monthlyWageLabel->setText(total + " €");
    double totalYearly = total.toDouble()*12;
    calculateTaxes(totalYearly);
}

void MainWindow::countByHourly()
{
    // check if input is a number
    if(!isNumber((ui->hourlyLineEdit->text().toStdString())) ||
            !isNumber(ui->hoursLineEdit->text().toStdString())){
        return;
    }
    double hourlyWage = ui->hourlyLineEdit->text().toDouble();
    double hours = ui->hoursLineEdit->text().toDouble();

    // check if input is negative value
    if(hourlyWage < 0 || hours < 0){
        ui->errorTextLabel->setText("VIRHE! Syötä vain positiivisia arvoja!");
        ui->errorTextLabel->setStyleSheet("QLabel {color : red; }");
        return;
    }

    ui->errorTextLabel->setText("");

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

    if(yearlyAfterDeduction < 0){
        allZeros(totalYearly);
        return;
    }

    double yleTax = 0.0;

    // Yle-tax
    yleTax += (yearlyAfterDeduction - 14000) * YLETAX;
    if (yleTax > 163) {
        yleTax = 163.0;
    }
    else if (yleTax < 0) {
        yleTax = 0;
    }
    ui->yleTaxLabel->setText(QString::number(yleTax, 'f', 2) + " €");


    // Pension, unemployment and healthcare payments
    double TyEL = totalYearly * TyELpercent;
    ui->TyELLabel->setText(QString::number(TyEL, 'f', 2) + " €");

    double unemp = totalYearly * unemploymentPercent;
    ui->unemploymentLabel->setText(QString::number(unemp, 'f', 2) + " €");

    double healthcare = totalYearly* healthcarePercent;
    if(totalYearly < 14574){
        healthcare = 0;
    }
    ui->healthcareLabel->setText(QString::number(healthcare, 'f', 2) + " €");

    double yearlyBeforeTaxes = yearlyAfterDeduction - TyEL- unemp - healthcare;


    // Get state income tax
    double incomeTaxAmount = incomeTax(yearlyBeforeTaxes, totalYearly, yearlyAfterDeduction);


    // Get municipal tax and church tax
    std::pair<double, double> tax = municipalTax(yearlyBeforeTaxes, totalYearly, yearlyAfterDeduction);

    double yearlyNet = yearlyBeforeTaxes - incomeTaxAmount - tax.first - tax.second - yleTax +  DEDUCTION;
    ui->yearlyNetLabel->setText(QString::number((yearlyNet))+ " €");
    ui->monthlyNetLabel->setText(QString::number((yearlyNet/12))+ " €");
}

double MainWindow::incomeTax(double yearlyBeforeTaxes, double totalYearly,
                             double yearlyAfterDeduction)
{
    double incomeTax = 0;
    if (yearlyBeforeTaxes < 18100) {
        incomeTax += 0.0;
    }
    else if (yearlyBeforeTaxes > 18100 && yearlyBeforeTaxes < 27200) {
        incomeTax =  8.0 + 0.06 * (yearlyBeforeTaxes - 18100);
    }
    else if (yearlyBeforeTaxes > 27200 && yearlyBeforeTaxes < 44800) {
        incomeTax = 554.0 + 0.1725 * (yearlyBeforeTaxes - 27200);
    }
    else if (yearlyBeforeTaxes > 44800 && yearlyBeforeTaxes < 78500) {
        incomeTax = 3590.0 + 0.2125 * (yearlyBeforeTaxes - 44800);
    }
    else if (yearlyBeforeTaxes > 78500) {
        incomeTax = 10751.25 + 0.3125 * (yearlyBeforeTaxes - 78500);
    }

    double deduction = (totalYearly - 2500) * 0.125;

    if(deduction > 1770){
        if(yearlyAfterDeduction > 33000){
            deduction = 1770 -(yearlyAfterDeduction - 33000) * 0.0184;
        }
        else{
            deduction = 1770;
        }
    }
    else if(deduction < 0){
        deduction = 0;
    }

    if(incomeTax - deduction < 0){
        incomeTax = 0;
    }
    else{
        incomeTax = incomeTax - deduction;
    }

    ui->incomeTaxLabel->setText(QString::number(incomeTax, 'f', 2) + " €");

    return incomeTax;
}

std::pair<double, double> MainWindow::municipalTax(double yearlyBeforeTaxes, double totalYearly, double yearlyAfterDeduction)
{

    double deduction = (7230-2500) * 0.51 + (totalYearly - 7230)*0.28;
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

    double basicDeduction = 0;
    if(yearlyBeforeTaxes-deduction <= 3540){
        basicDeduction = 3540;
    }
    else if(yearlyBeforeTaxes-deduction > 3540 && yearlyBeforeTaxes-deduction < 23200){
        basicDeduction = 3540 - 0.18*(yearlyBeforeTaxes-deduction-3540);
    }

    QString currentMunicipality = ui->municipalitiesComboBox->currentText();
    double municipalityTaxAmount = (yearlyBeforeTaxes-deduction-basicDeduction)*municipalTaxPercent/100;

    if(municipalityTaxAmount < 0){
        municipalityTaxAmount = 0;
    }
    ui->municipalityTaxLabel->setText(QString::number(municipalityTaxAmount, 'f', 2)+ " €");


    // church tax
    double churchTaxAmount = 0;
    if(ui->checkBox->checkState() == UNCHECKED){
        churchTaxAmount = 0;
        ui->churchTaxAmountLabel->setText(QString::number(churchTaxAmount, 'f', 2)+ " €");
    }else{
        churchTaxAmount = (yearlyBeforeTaxes-deduction)*(taxes.at(currentMunicipality).second/100);

        if(churchTaxAmount < 0){
            churchTaxAmount = 0;
        }
        ui->churchTaxAmountLabel->setText(QString::number(churchTaxAmount, 'f', 2)+ " €");
    }

    return {municipalityTaxAmount, churchTaxAmount};
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

void MainWindow::allZeros(double totalYearly)
{
    ui->municipalityTaxLabel->setText("0 €");
    ui->churchTaxLabel->setText("0 €");
    ui->TyELLabel->setText(QString::number(totalYearly*TyELpercent, 'f', 2)+ " €");
    ui->unemploymentLabel->setText(QString::number(totalYearly*unemploymentPercent, 'f', 2) + " €");
    ui->healthcareLabel->setText("0 €");
    ui->yleTaxLabel->setText("0 €");

    double net = totalYearly - totalYearly*TyELpercent - totalYearly*unemploymentPercent;
    ui->yearlyNetLabel->setText(QString::number(net, 'f', 2)+ " €");
    ui->monthlyNetLabel->setText(QString::number(net/12, 'f', 2)+ " €");
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

void MainWindow::on_municipalitiesComboBox_currentTextChanged(const QString &currentMunicipality)
{
    double taxPercentage = taxes.at(currentMunicipality).first;
    municipalTaxPercent = taxPercentage;
    QString taxPercentageStr = QString::number(taxes.at(currentMunicipality).first) + " %";
    ui->percentageLabel->setText(taxPercentageStr);

    churchTax = taxes.at(currentMunicipality).second;

    on_checkBox_stateChanged(ui->checkBox->checkState());

}

void MainWindow::on_checkBox_stateChanged(int state)
{
    churchTax = taxes.at(ui->municipalitiesComboBox->currentText()).second;
    if(state == UNCHECKED){
        churchTax = 0;
        ui->churchTaxLabel->setText("");
    }
    else{
        QString churchTaxString = "Kirkollisveroprosentti: " +
                QString::number(churchTax) + " %";
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

void MainWindow::on_exitButton_clicked()
{
     QCoreApplication::quit();
}
