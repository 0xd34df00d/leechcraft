/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <stdexcept>
#include <QString>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QTime>
#include <QSettings>
#include <QTextCodec>
#include <QUrl>
#include <QAction>
#include <QBuffer>
#include <QAction>
#include <QModelIndexList>
#include <QtDebug>

namespace LC::Util
{
	QString GetAsBase64Src (const QImage& pix)
	{
		QBuffer buf;
		buf.open (QIODevice::ReadWrite);
		const auto compression = 100;
		pix.save (&buf, "PNG", compression);
		return QString ("data:image/png;base64,%1")
				.arg (QString (buf.buffer ().toBase64 ()));
	}

	namespace
	{
		QString MakePrettySizeWith (qint64 sourceSize, const QStringList& units)
		{
			int strNum = 0;
			long double size = sourceSize;

			for (; strNum < 3 && size >= 1024; ++strNum, size /= 1024)
				;

			return QString::number (size, 'f', 1) + units.value (strNum);
		}
	}

	QString MakePrettySize (qint64 sourcesize)
	{
		static QStringList units
		{
			QObject::tr (" b"),
			QObject::tr (" KiB"),
			QObject::tr (" MiB"),
			QObject::tr (" GiB")
		};

		return MakePrettySizeWith (sourcesize, units);
	}

	QString MakePrettySizeShort (qint64 sourcesize)
	{
		static const QStringList units
		{
			QObject::tr ("b", "Short one-character unit for bytes."),
			QObject::tr ("K", "Short one-character unit for kilobytes."),
			QObject::tr ("M", "Short one-character unit for megabytes."),
			QObject::tr ("G", "Short one-character unit for gigabytes.")
		};

		return MakePrettySizeWith (sourcesize, units);
	}

	QString MakeTimeFromLong (ulong time)
	{
		int d = time / 86400;
		time -= d * 86400;
		QString result;
		if (d)
			result += QObject::tr ("%n day(s), ", "", d);
		result += QTime (0, 0, 0).addSecs (time).toString ();
		return result;
	}

	QTranslator* LoadTranslator (const QString& baseName,
			const QString& localeName,
			const QString& prefix,
			const QString& appName)
	{
		auto filename = prefix;
		filename.append ("_");
		if (!baseName.isEmpty ())
			filename.append (baseName).append ("_");
		filename.append (localeName);

		auto transl = new QTranslator;
	#ifdef Q_OS_WIN32
		Q_UNUSED (appName)
		if (transl->load (filename, ":/") ||
				transl->load (filename,
						QCoreApplication::applicationDirPath () + "/translations"))
	#elif defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
		Q_UNUSED (appName)
		if (transl->load (filename, ":/") ||
				transl->load (filename,
						QCoreApplication::applicationDirPath () + "/../Resources/translations"))
	#elif defined (INSTALL_PREFIX)
		if (transl->load (filename, ":/") ||
				transl->load (filename,
						QString (INSTALL_PREFIX "/share/%1/translations").arg (appName)))
	#else
		if (transl->load (filename, ":/") ||
				transl->load (filename,
						QString ("/usr/local/share/%1/translations").arg (appName)) ||
				transl->load (filename,
						QString ("/usr/share/%1/translations").arg (appName)))
	#endif
			return transl;

		delete transl;

		return nullptr;
	}

	QTranslator* InstallTranslator (const QString& baseName,
			const QString& prefix,
			const QString& appName)
	{
		const auto& localeName = GetLocaleName ();
		if (auto transl = LoadTranslator (baseName, localeName, prefix, appName))
		{
			qApp->installTranslator (transl);
			return transl;
		}

		qWarning () << Q_FUNC_INFO
				<< "could not load translation file for locale"
				<< localeName
				<< baseName
				<< prefix
				<< appName;
		return nullptr;
	}

	QString GetLocaleName ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName ());
		QString localeName = settings.value ("Language", "system").toString ();

		if (localeName == "system")
		{
			localeName = QString (::getenv ("LANG")).left (5);

			if (localeName == "C" || localeName.isEmpty ())
				localeName = "en_US";

			if (localeName.isEmpty () || localeName.size () != 5)
				localeName = QLocale::system ().name ();
			localeName = localeName.left (5);
		}

		if (localeName.size () == 2)
		{
			auto lang = QLocale (localeName).language ();
			const auto& cs = QLocale::countriesForLanguage (lang);
			if (cs.isEmpty ())
				localeName += "_00";
			else
				localeName = QLocale (lang, cs.at (0)).name ();
		}

		return localeName;
	}

	QString GetInternetLocaleName (const QLocale& locale)
	{
		if (locale.language () == QLocale::AnyLanguage)
			return "*";

		auto locStr = locale.name ();
		locStr.replace ('_', '-');
		return locStr;
	}

	QString GetLanguage ()
	{
		return GetLocaleName ().left (2);
	}

	QModelIndexList GetSummarySelectedRows (QObject *sender)
	{
		QAction *senderAct = qobject_cast<QAction*> (sender);
		if (!senderAct)
		{
			QString debugString;
			{
				QDebug d (&debugString);
				d << "sender is not a QAction*"
						<< sender;
			}
			throw std::runtime_error (qPrintable (debugString));
		}

		return senderAct->
				property ("SelectedRows").value<QList<QModelIndex>> ();
	}

	QAction* CreateSeparator (QObject *parent)
	{
		QAction *result = new QAction (parent);
		result->setSeparator (true);
		return result;
	}
}
