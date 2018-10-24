#ifndef TEXTFILECOORDCONVERTDIALOG_H
#define TEXTFILECOORDCONVERTDIALOG_H

#include <QDialog>

namespace Ui {
class TextFileCoordConvertDialog;
}

class TextFileCoordConvertDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextFileCoordConvertDialog(QWidget *parent = 0);
    ~TextFileCoordConvertDialog();
    
public Q_SLOTS:
    virtual void accept();

private slots:
    void slotSkipRowNumChanged(int value);
    void slotPartRangeCheckBox(int state);
    void slotOpenSrcFile();
    void slotOpenDstFile();

    void slotSelSrcCoordinateSystem();
    void slotSelDesCoordinateSystem();

    bool validateParameters();
    bool convert() const;

    QString getSrcFileName() const;
    QString getDstFileName() const;

    QString getSrcCoordSys() const;
    QString getDstCoordSys() const;

    int getSkipRowNum() const;

    int getStartRow() const;
    int getEndRow() const;

    int getXColumnIndex() const;
    int getYColumnIndex() const;

private:
    void initialTableWidget();
    void updateTableWidgetComboBox(const QStringList &srcFieldNames);
    void updateRowLimits();

private:
    Ui::TextFileCoordConvertDialog *ui;

    QString m_srcCoordSys;
    QString m_dstCoordSys;

    int m_rowCount;
};

#endif // TEXTFILECOORDCONVERTDIALOG_H
