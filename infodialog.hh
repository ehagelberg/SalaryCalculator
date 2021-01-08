#ifndef INFODIALOG_HH
#define INFODIALOG_HH

#include <QDialog>

namespace Ui {
class infoDialog;
}

class infoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit infoDialog(QWidget *parent = nullptr);
    ~infoDialog();

private slots:
    void on_closeButton_clicked();

private:
    Ui::infoDialog *ui;
};

#endif // INFODIALOG_HH
