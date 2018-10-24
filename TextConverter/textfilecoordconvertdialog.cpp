#include "textfilecoordconvertdialog.h"
#include "ui_textfilecoordconvertdialog.h"

#include "projectionselector.h"
#include "textfileconverter.h"

#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTextCodec>

#pragma execution_character_set("utf-8")

TextFileCoordConvertDialog::TextFileCoordConvertDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextFileCoordConvertDialog),
    m_rowCount(-1)
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf-8"));


    ui->setupUi(this);

    initialTableWidget();

    connect(ui->startRowNumSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotSkipRowNumChanged(int)));
    connect(ui->rangeCheckBox, SIGNAL(stateChanged(int)), this, SLOT(slotPartRangeCheckBox(int)));

    connect(ui->openSrcFileBtn, SIGNAL(clicked()), this, SLOT(slotOpenSrcFile()));
    connect(ui->openDstFileBtn, SIGNAL(clicked()), this, SLOT(slotOpenDstFile()));

    connect(ui->selSrcCoordSysBtn, SIGNAL(clicked()), this, SLOT(slotSelSrcCoordinateSystem()));
    connect(ui->selDstCoordSysBtn, SIGNAL(clicked()), this, SLOT(slotSelDesCoordinateSystem()));

}

TextFileCoordConvertDialog::~TextFileCoordConvertDialog()
{
    delete ui;
}

void TextFileCoordConvertDialog::accept()
{
    if (!validateParameters())
    {
        return;
    }

    if (!convert())
    {
        QMessageBox::information(this, tr("提示!"), tr("坐标转换失败！"), QMessageBox::Ok);
        return;
    }

    QMessageBox::information(this, tr("提示!"), tr("坐标转换完毕！"), QMessageBox::Ok);
    QDialog::accept();
}

void TextFileCoordConvertDialog::slotSkipRowNumChanged(int value)
{
    ui->startSpinBox->setMinimum(value);
    ui->endSpinBox->setMinimum(value);
}

void TextFileCoordConvertDialog::slotPartRangeCheckBox(int state)
{
    if (state == Qt::Checked)
    {
        ui->startSpinBox->setEnabled(true);
        ui->endSpinBox->setEnabled(true);
    }
    else
    {
        ui->startSpinBox->setEnabled(false);
        ui->endSpinBox->setEnabled(false);
    }
}

QStringList GetSrcFileds(const QString fileName)
{
    QStringList srcFieldNames;
    QFile file(fileName);
    if ( !file.open(QIODevice::ReadOnly) )
    {
        qWarning()<<"open file failed."<<file.errorString();
        return srcFieldNames;
    }

    QTextStream textStream(&file);
    while (!textStream.atEnd())
    {
        QString textLine = textStream.readLine();
        if (!textLine.isEmpty())
        {
            if (textLine.contains(","))
            {
                srcFieldNames = textLine.split(",");
            }
            else if (textLine.contains("\t"))
            {
                srcFieldNames = textLine.split("\t");
            }
            else
            {
                srcFieldNames = textLine.split(" ");
            }

            break;
        }
    }

    file.close();

    return srcFieldNames;
}


void TextFileCoordConvertDialog::slotOpenSrcFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Find Files"));

    if (!fileName.isEmpty())
    {
        ui->srcFileLineEdit->setText(fileName);

        QStringList srcFieldNames = GetSrcFileds(fileName);
        updateTableWidgetComboBox(srcFieldNames);

        updateRowLimits();
    }
}

void TextFileCoordConvertDialog::slotOpenDstFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Files"));

    if (!fileName.isEmpty())
    {
      //  Util::writeDefaultPath(Q_FUNC_INFO, QFileInfo(fileName).absolutePath());
        ui->dstFileLineEdit->setText(fileName);
    }
}

void TextFileCoordConvertDialog::slotSelSrcCoordinateSystem()
{
    ProjectionSelector dlgProjtionSelector;
    if (dlgProjtionSelector.exec() == QDialog::Accepted)
    {
        m_srcCoordSys = dlgProjtionSelector.pro4String();
        ui->srcCoordLineEdit->setText(dlgProjtionSelector.projectName());
    }
}

void TextFileCoordConvertDialog::slotSelDesCoordinateSystem()
{
    ProjectionSelector dlgProjtionSelector;
    if (dlgProjtionSelector.exec() == QDialog::Accepted)
    {
        m_dstCoordSys = dlgProjtionSelector.pro4String();
        ui->dstCoordLineEdit->setText(dlgProjtionSelector.projectName());
    }
}

bool TextFileCoordConvertDialog::validateParameters()
{
    if (getSrcFileName().isEmpty())
    {
        QMessageBox::information(this, tr("提示!"), tr("源文件名不能为空！"), QMessageBox::Ok);
        ui->srcFileLineEdit->setFocus();
        return false;
    }
    if (getDstFileName().isEmpty())
    {
        QMessageBox::information(this, tr("提示!"), tr("结果文件名不能为空！"), QMessageBox::Ok);
        ui->dstFileLineEdit->setFocus();
        return false;
    }
    if (getStartRow()>=getEndRow())
    {
        QMessageBox::information(this, tr("提示!"), tr("转换范围设置不正确！"), QMessageBox::Ok);
        ui->startSpinBox->setFocus();
        ui->endSpinBox->setFocus();
        return false;
    }
    if (ui->srcCoordLineEdit->text().isEmpty())
    {
        QMessageBox::information(this, tr("提示!"), tr("请设置源坐标系统！"), QMessageBox::Ok);
        ui->selSrcCoordSysBtn->setFocus();
        return false;
    }
    if (ui->dstCoordLineEdit->text().isEmpty())
    {
        QMessageBox::information(this, tr("提示!"), tr("请设置目标坐标系统！"), QMessageBox::Ok);
        ui->selDstCoordSysBtn->setFocus();
        return false;
    }
    QString xColName;
    QComboBox *xComboBox = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(0, 1));
    if (xComboBox)
        xColName  = xComboBox->currentText();
    if (xColName.isEmpty())
    {
        QMessageBox::information(this, tr("提示!"), tr("请设置字段在文件中对应的列号！"), QMessageBox::Ok);
        return false;
    }
    QString yColName;
    QComboBox *yComboBox = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(1, 1));
    if (yComboBox)
        yColName  = yComboBox->currentText();
    if (yColName.isEmpty())
    {
        QMessageBox::information(this, tr("提示!"), tr("请设置字段在文件中对应的列号！"), QMessageBox::Ok);
        return false;
    }

    return true;
}

bool TextFileCoordConvertDialog::convert() const
{
    if (m_srcCoordSys.isEmpty())
        return false;

    if (m_dstCoordSys.isEmpty())
        return false;

    TextFileConverter textFileConverter(getSrcFileName(), getDstFileName(),
                                        getSrcCoordSys(), getDstCoordSys());

    textFileConverter.setXYColumnIndex(getXColumnIndex(), getYColumnIndex());
    textFileConverter.setRowRange(getStartRow(), getEndRow());

    return textFileConverter.convert();
}

QString TextFileCoordConvertDialog::getSrcFileName() const
{
    return ui->srcFileLineEdit->text().trimmed();
}

QString TextFileCoordConvertDialog::getDstFileName() const
{
    return ui->dstFileLineEdit->text().trimmed();
}

QString TextFileCoordConvertDialog::getSrcCoordSys() const
{
    return m_srcCoordSys;
}

QString TextFileCoordConvertDialog::getDstCoordSys() const
{
    return m_dstCoordSys;
}

int TextFileCoordConvertDialog::getSkipRowNum() const
{
    return ui->startRowNumSpinBox->value() - 1;
}

int TextFileCoordConvertDialog::getStartRow() const
{
    if (ui->rangeCheckBox->isChecked())
    {
        return ui->startSpinBox->value();
    }
    else
    {
        return getSkipRowNum();
    }
}

int TextFileCoordConvertDialog::getEndRow() const
{
    if (ui->rangeCheckBox->isChecked())
    {
        return ui->endSpinBox->value();
    }
    else
    {
        return m_rowCount;
    }
}

int TextFileCoordConvertDialog::getXColumnIndex() const
{
    int xIndex = 0;
    QComboBox *xComboBox = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(0, 1));
    if (xComboBox)
        xIndex  = xComboBox->currentIndex();

    return xIndex - 1;
}

int TextFileCoordConvertDialog::getYColumnIndex() const
{
    int yIndex = 0;
    QComboBox *yComboBox = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(1, 1));
    if (yComboBox)
        yIndex  = yComboBox->currentIndex();

    return yIndex - 1;
}

void TextFileCoordConvertDialog::initialTableWidget()
{
    QStringList headerLabels;
    headerLabels << QString::fromLocal8Bit("目标字段") << QString::fromLocal8Bit("源字段");
    ui->tableWidget->setColumnCount(headerLabels.size());
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
    ui->tableWidget->setColumnWidth(0, 120);

    QStringList fieldNames;
    fieldNames << QString::fromLocal8Bit("X坐标/经度（*）") << QString::fromLocal8Bit("Y坐标/纬度（*）");
    int rowNum = fieldNames.size();
    ui->tableWidget->setRowCount(rowNum);
    for (int iRow=0; iRow<rowNum; ++iRow)
    {
        QString dstFieldName(fieldNames.at(iRow));
        QTableWidgetItem *item0 = new QTableWidgetItem(dstFieldName);
        //Util::setItemIsEditable(item0, false);

        ui->tableWidget->setItem(iRow, 0, item0);
        ui->tableWidget->setCellWidget(iRow, 1, new QComboBox());
    }
}

void TextFileCoordConvertDialog::updateTableWidgetComboBox(const QStringList &srcFieldNames)
{
    QStringList nullsrcFieldNames(srcFieldNames);
    nullsrcFieldNames.prepend(QString(" "));

    QComboBox *xComboBox = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(0, 1));
    if (xComboBox)
    {
        xComboBox->clear();
        xComboBox->addItems(nullsrcFieldNames);
    }

    QComboBox *yComboBox = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(1, 1));
    if (yComboBox)
    {
        yComboBox->clear();
        yComboBox->addItems(nullsrcFieldNames);
    }
}

void TextFileCoordConvertDialog::updateRowLimits()
{
    const QString fileName(getSrcFileName());
    QFile file(fileName);
    if ( !file.open(QIODevice::ReadOnly) )
    {
        qWarning()<<"open file failed."<<file.errorString();
        return;
    }
    m_rowCount = 0;
    QTextStream textStream(&file);
    while (!textStream.atEnd())
    {
        textStream.readLine();
        ++m_rowCount;
    }
    file.close();

    ui->startSpinBox->setRange(getSkipRowNum(), m_rowCount);
    ui->endSpinBox->setRange(getSkipRowNum(), m_rowCount);
}
