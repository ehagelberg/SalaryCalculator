/*
 * Author: Elias Hagelberg
 * File: mainWindow.hh
 * Desc: Header file for the mainWindow class.
 */


#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QString>
#include <string>
#include <boost/lexical_cast.hpp>
#include <map>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_monthlyRadioButton_clicked();

    void on_hourlyRadioButton_clicked();

    void on_calculatePushButton_clicked();

    void on_municipalitiesComboBox_currentTextChanged(const QString &currentMunicipality);

    void on_checkBox_stateChanged(int arg1);

    void on_ageComboBox_currentTextChanged(const QString &arg1);

    void on_exitButton_clicked();

    void on_actionTietoja_triggered();

    void on_actionDark_mode_triggered();

private:
    ///
    /// \brief reads the file necessary tax percentages
    ///
    void readFile();

    ///
    /// \brief calculates taxes if input given as monthly salary
    ///
    void countByMonthly();

    ///
    /// \brief calculates taxes if input given as hourly salary
    ///
    void countByHourly();

    ///
    /// \brief calculateTaxes
    /// \param totalYearly  Total yearly income calculated from user input
    ///
    void calculateTaxes(double totalYearly);

    ///
    /// \brief incomeTax
    /// \param yearlyBeforeTaxes    Total yearly income before taxes taken off
    /// \param totalYearly  Total yearly income calculated from user input
    /// \param yearlyAfterDeduction   Total yearly income after initial deduction
    /// \return   The amount of income tax
    ///
    double incomeTax(double yearlyBeforeTaxes, double totalYearly, double yearlyAfterDeduction);

    ///
    /// \brief municipalTax
    /// \param yearlyBeforeTaxes    Total yearly income before taxes taken off
    /// \param totalYearly  Total yearly income calculated from user input
    /// \param yearlyAfterDeduction   Total yearly income after initial deduction
    /// \return a pair containing the amounts of municipal tax and church tax
    ///
    std::pair<double, double> municipalTax(double yearlyBeforeTaxes, double totalYearly, double yearlyAfterDeduction);

    ///
    /// \brief checks if inputs are in correct form
    /// \param str  input to be checked
    /// \return true if input in correct form, false if not
    ///
    bool isNumber(std::string str);

    ///
    /// \brief smallIncome  Makes the calculations if income is very small
    /// \param totalYearly  Total yearly income calculated from user input
    ///
    void smallIncome(double totalYearly);

    void changeTheme(QString color, QList<QObject*> &list);


    Ui::MainWindow *ui;

    std::map<QString, std::pair<double, double>> taxes;
    bool isHourly = true;
    double churchTax = 0;
    double municipalTaxPercent = 0;
    double TyELpercent = 0.0715;
    double unemploymentPercent = 0.0125;
    double healthcarePercent = 0.0118;

    bool darkMode = false;

};
#endif // MAINWINDOW_HH
