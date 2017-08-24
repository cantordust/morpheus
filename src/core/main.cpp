#include "MainWindow.hpp"
#include <QApplication>
#include "Globals.hpp"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	Morpheus::MainWindow main_window;
	main_window.show();

	return app.exec();
}
