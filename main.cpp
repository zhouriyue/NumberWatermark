#include "homewindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

void loadStyle(const QString &strQssFile)
{
    QString qss;
    QFile file(strQssFile);
    if (file.open(QFile::ReadOnly))
    {
        QStringList list;
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line;
            in >> line;
            list << line;
        }

        file.close();
        qss = list.join("\n");
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(paletteColor));
        qApp->setStyleSheet(qss);
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //LoginWindow w;
    //w.show();
    HomeWindow home("admin");

    home.show();
    //加载样式
    loadStyle(":/qss/lightblue.css");
    return a.exec();
}
