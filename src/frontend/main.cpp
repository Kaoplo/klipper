//
// Created by kaoplo on 8/12/26.
//

#include "frontend/main_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    klipper::MainWindow window;
    window.show();

    return app.exec();
}