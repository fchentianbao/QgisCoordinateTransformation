#ifndef TEXTFILECONVERTER_H
#define TEXTFILECONVERTER_H

#include "coordinatetransform.h"

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>

class TextFileConverter
{
public:
    explicit TextFileConverter(const QString &srcFileName,
                               const QString &dstFileName,
                               const QString &srcCoordSys,
                               const QString &dstCoordSys);

    void setXYColumnIndex(int xColIndex, int yColIndex);

    void setRowRange(int startRow, int endRow);

    bool convert();

private:
    QString lineConvert(const QString &oldLine);

private:
    const QString m_srcFileName;
    const QString m_dstFileName;
    const CoordinateTransform m_coordTransform;

    int m_xColIndex;
    int m_yColIndex;

    int m_startRowNum;
    int m_endRowNum;
};

#endif // TEXTFILECONVERTER_H
