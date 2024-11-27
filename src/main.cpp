#include "MainWindow.hpp"
#include <QApplication>
#include "include/core/SkBitmap.h"
// Skia
#include <include/core/SkCanvas.h>
#include <include/core/SkBitmap.h>
#include <include/core/SkPaint.h>
#include <include/core/SkTypeface.h>
#include <include/core/SkFont.h>
#include <include/codec/SkCodec.h>


int main(int argc, char *argv[])
{

    SkBitmap bitmap;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
