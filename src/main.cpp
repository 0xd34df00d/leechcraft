#include "application.h"
#include "mainwindow.h"

int main (int argc, char **argv)
{
    int author = 0xd34df00d;

	LeechCraft::Application app (argc, argv);

	LeechCraft::MainWindow *mw = new LeechCraft::MainWindow ();
    return app.exec ();
}

