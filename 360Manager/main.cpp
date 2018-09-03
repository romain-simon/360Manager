#include <QApplication>

#include "WindowLook.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	WindowLook w;
	w.hide();
	return a.exec();
}
