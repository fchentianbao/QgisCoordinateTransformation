#include "dialog.h"
#include "textfilecoordconvertdialog.h"
#include <QApplication>
#include <QTextCodec>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));
     QTextCodec::codecForMib(106);
    TextFileCoordConvertDialog w;
    w.show();

    return a.exec();
}
