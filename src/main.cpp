#include "application.h"

int main (int argc, char **argv)
{
	int author = 0xd34df00d;
	Q_UNUSED (author);

	LeechCraft::Application app (argc, argv);
	return app.exec ();
}

