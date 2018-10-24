#include "textfileconverter.h"


#include <QDebug>

TextFileConverter::TextFileConverter(const QString &srcFileName,
                                     const QString &dstFileName,
                                     const QString &srcCoordSys,
                                     const QString &dstCoordSys) :
    m_srcFileName(srcFileName),
    m_dstFileName(dstFileName),
    m_coordTransform(srcCoordSys, dstCoordSys),
    m_xColIndex(-1), m_yColIndex(-1),
    m_startRowNum(0), m_endRowNum(0)
{
}

void TextFileConverter::setRowRange(int startRow, int endRow)
{
    m_startRowNum = startRow;
    m_endRowNum = endRow;
}

void TextFileConverter::setXYColumnIndex(int xColIndex, int yColIndex)
{
    m_xColIndex = xColIndex;
    m_yColIndex = yColIndex;
}

bool TextFileConverter::convert()
{
    qDebug()<<"m_srcFileName="<<m_srcFileName;
    qDebug()<<"m_dstFileName="<<m_dstFileName;

    qDebug()<<"m_xColIndex="<<m_xColIndex;
    qDebug()<<"m_yColIndex="<<m_yColIndex;

    qDebug()<<"m_startRowNum="<<m_startRowNum;
    qDebug()<<"m_endRowNum="<<m_endRowNum;

    QFile srcFile(m_srcFileName);
    if (!srcFile.open(QIODevice::ReadOnly))
    {
        return false;
    }
    QTextStream srcTextStream(&srcFile);

    QFile dstFile(m_dstFileName);
    if (!dstFile.open(QIODevice::WriteOnly))
    {
        return false;
    }
    QTextStream dstTextStream(&dstFile);

    int rowCnt = 0;
    while (!srcTextStream.atEnd())
    {
        ++rowCnt;
        const QString oldLine = srcTextStream.readLine();
        QString newLine;
        if (rowCnt>m_startRowNum && rowCnt<m_endRowNum)
            newLine = lineConvert(oldLine);
        else
            newLine = oldLine;

        dstTextStream << newLine << endl;
    }
    srcFile.close();
    dstFile.close();

    return true;
}

QStringList splitTextLine(const QString &textLine, bool ignoreExtraSpaces = false)
{
    QString::SplitBehavior behavior = QString::KeepEmptyParts;
    if (ignoreExtraSpaces)
        behavior = QString::SkipEmptyParts;

    QStringList results;
    if (textLine.contains(","))
    {
        results = textLine.split(",", behavior);
    }
    else if (textLine.contains("\t"))
    {
        results = textLine.split("\t", behavior);
    }
    else
    {
        results = textLine.split(" ", behavior);
    }

    // 将字段值内容的前后空格去掉
    for (QStringList::size_type i=0; i<results.size(); ++i)
    {
        results[i] = results.at(i).trimmed();
    }

    return results;
}


QString TextFileConverter::lineConvert(const QString &oldLine)
{
    QStringList lines = splitTextLine(oldLine);
    double x = lines.at(m_xColIndex).trimmed().toDouble();
    double y = lines.at(m_yColIndex).trimmed().toDouble();

    QPointF des = m_coordTransform.transform(x, y);
    lines.replace(m_xColIndex, QString::number(des.x(), 'f', 4));
    lines.replace(m_yColIndex, QString::number(des.y(), 'f', 4));

    return lines.join("\t");
}
