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
#include <QUrl>
#include <QAction>
#include <QBuffer>
#include <QAction>
#include <QModelIndexList>
#include <QtDebug>
#include <util/sll/qtutil.h>

namespace LC::Util
{
	const QString LCLowercase = QStringLiteral ("leechcraft");

	QString GetAsBase64Src (const QImage& pix)
	{
		QBuffer buf;
		buf.open (QIODevice::ReadWrite);
		const auto compression = 100;
		pix.save (&buf, "PNG", compression);
		return QStringLiteral ("data:image/png;base64,") + buf.buffer ().toBase64 ();
	}

	namespace
	{
		QString MakePrettySizeWith (qint64 sourceSize, const QStringList& units, const QString& space)
		{
			int strNum = 0;
			long double size = sourceSize;

			for (; strNum < 3 && size >= 1024; ++strNum, size /= 1024)
				;

			return QString::number (size, 'f', 1) + space + units.value (strNum);
		}
	}

	QString MakePrettySize (qint64 sourcesize)
	{
		static QStringList units
		{
			QObject::tr ("B", "The unit for bytes."),
			QObject::tr ("KiB", "The unit for kilobytes."),
			QObject::tr ("MiB", "The unit for megabytes."),
			QObject::tr ("GiB", "The unit for gigabytes.")
		};

		return MakePrettySizeWith (sourcesize, units, " "_qs);
	}

	QString MakePrettySizeShort (qint64 sourcesize)
	{
		static const QStringList units
		{
			QObject::tr ("B", "Short one-character unit for bytes."),
			QObject::tr ("K", "Short one-character unit for kilobytes."),
			QObject::tr ("M", "Short one-character unit for megabytes."),
			QObject::tr ("G", "Short one-character unit for gigabytes.")
		};

		return MakePrettySizeWith (sourcesize, units, {});
	}

	QString MakeTimeFromLong (ulong time)
	{
		const auto secsPerDay = 86400;
		int d = time / secsPerDay;
		time -= d * secsPerDay;
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
			QCoreApplication::installTranslator (transl);
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

	QLocale GetLocale ()
	{
		const QSettings settings { QCoreApplication::organizationName (), QCoreApplication::applicationName () };
		const auto& localeName = settings.value ("Language"_qs, "system"_qs).toString ();
		if (localeName == "system"_ql)
			return QLocale {};

		QLocale locale { localeName };
		if (locale.language () == QLocale::AnyLanguage)
		{
			qWarning () << "unable to parse" << localeName;
			return QLocale {};
		}
		return locale;
	}

	QString GetLocaleName ()
	{
		return GetLocale ().name ();
	}

	QString GetInternetLocaleName (const QLocale& locale)
	{
		if (locale.language () == QLocale::AnyLanguage)
			return QStringLiteral ("*");

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
		const auto senderAct = qobject_cast<QAction*> (sender);
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

		return senderAct->property ("SelectedRows").value<QList<QModelIndex>> ();
	}

	QAction* CreateSeparator (QObject *parent)
	{
		const auto result = new QAction (parent);
		result->setSeparator (true);
		return result;
	}
}
