#include "AutoHeader.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AutoHeader w;
    w.show();

    return a.exec();
}
