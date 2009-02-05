#include "application.h"
#include <QEvent>
#include <QtDebug>

using namespace LeechCraft;

Application::Application (int& argc, char **argv)
: QApplication (argc, argv)
{
}

bool Application::notify (QObject *obj, QEvent *event)
{
	try
	{
		return QApplication::notify (obj, event);
	}
	catch (const std::exception& e)
	{
		qWarning () << "GLOBALLY" << e.what () << "for" << obj << event << event->type ();
	}
	catch (...)
	{
		qWarning () << "GLOBALL caught something" << obj << event << event->type ();
	}
	return false;
}

