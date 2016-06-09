#include "cantool.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    cantool w;
    w.show();

    return a.exec();
}
