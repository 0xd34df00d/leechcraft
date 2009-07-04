#include "application.h"
#include "mainwindow.h"

int main (int argc, char **argv)
{
	int author = 0xd34df00d;
	Q_UNUSED (author);

	LeechCraft::Application app (argc, argv);

	new LeechCraft::MainWindow ();
	return app.exec ();
}

