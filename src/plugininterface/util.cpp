#include "util.h"
#include <stdexcept>
#include <QString>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QFile>
#include <QDir>
#include <QtDebug>

QTranslator* LeechCraft::Util::InstallTranslator (const QString& baseName)
{
	QTranslator *transl = new QTranslator;
	QString localeName = QString(::getenv ("LANG"));
	if (localeName.isEmpty () || localeName.size () != 5)
		localeName = QLocale::system ().name ();
	localeName = localeName.left (5);

	QString filename = QString (":/leechcraft_");
	if (!baseName.isEmpty ())
		filename.append (baseName).append ("_");
	filename.append (localeName);

	if (!transl->load (filename))
	{
		qWarning () << Q_FUNC_INFO << "could not load translation file for locale" << localeName << filename;
		return 0;
	}
	qApp->installTranslator (transl);

	return transl;
}

void LeechCraft::Util::CreateIfNotExists (const QString& path)
{
	QDir home = QDir::home ();
	home.cd (".leechcraft");
	if (home.exists (path) && !home.cd (path))
	{
		throw std::runtime_error (qPrintable (QObject::tr ("Could not cd into %1")
					.arg (QDir::toNativeSeparators (home.filePath (path)))));
	}
	home = QDir::home ();
	home.cd (".leechcraft");
	if (!home.exists (path) && !home.mkpath (path))
	{
		throw std::runtime_error (qPrintable (QObject::tr ("Could not create %1")
					.arg (QDir::toNativeSeparators (home.filePath (path)))));
	}
}

