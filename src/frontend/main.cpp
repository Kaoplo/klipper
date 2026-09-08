//
// Created by kaoplo on 8/12/26.
//

#include "frontend/main_window.h"
#include "build_info.h"

#include <QApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("klipper");
    QCoreApplication::setApplicationVersion(QString::fromUtf8(klipper::build::version));
    std::cout << klipper::build::summary << '\n'
              << klipper::build::details << std::endl;

    klipper::MainWindow window;
    window.show();

    return app.exec();
}
