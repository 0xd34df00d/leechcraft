/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "application.h"
#include <thread>
#include <chrono>
#include <QFont>
#include <QSysInfo>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QtDebug>

#if defined(Q_OS_MAC) && !defined(USE_UNIX_LAYOUT)
#include <mach-o/dyld.h>

#include "util/sys/util.h"

namespace
{
	void SetupLibraryPaths ()
	{
		if (!LC::Util::IsOSXLoadFromBundle ())
			return;

		char path [1024] = { 0 };
		uint32_t pathLength = sizeof (path);
		if (const auto rc = _NSGetExecutablePath (path, &pathLength))
		{
			qCritical () << Q_FUNC_INFO
					<< "cannot get executable path:"
					<< rc;
			return;
		}

		auto dir = QFileInfo { path }.dir ();
		dir.cdUp ();
		dir.cd ("PlugIns");
		qDebug () << Q_FUNC_INFO
				<< "setting"
				<< dir.absolutePath ();
		QCoreApplication::setLibraryPaths ({ dir.absolutePath () });

		auto setDefaultEnv = [] (const char *name, const QByteArray& value)
		{
			if (qgetenv (name).isEmpty ())
				qputenv (name, value);
		};

		// the bundled OpenSSL would load its providers, engines and config from Homebrew's prefix otherwise
		const auto& opensslModules = dir.absoluteFilePath ("openssl").toUtf8 ();
		setDefaultEnv ("OPENSSL_MODULES", opensslModules);
		setDefaultEnv ("OPENSSL_ENGINES", opensslModules);
		setDefaultEnv ("OPENSSL_CONF", "/dev/null");
	}
}
#endif

namespace
{
	void CheckDelay ()
	{
		const auto& delayVar = qgetenv ("LC_STARTUP_DELAY");
		if (delayVar.isEmpty ())
			return;

		bool ok = false;
		const auto delayVal = delayVar.toInt (&ok);
		if (!ok)
			return;

		std::this_thread::sleep_for (std::chrono::seconds (delayVal));
	}
}

int main (int argc, char **argv)
{
	int author = 0xd34df00d;
	Q_UNUSED (author);

	CheckDelay ();

#if defined(Q_OS_MAC) && !defined(USE_UNIX_LAYOUT)
	SetupLibraryPaths ();
#endif

	QCoreApplication::setAttribute (Qt::AA_ShareOpenGLContexts);

	LC::Application app (argc, argv);
	return app.exec ();
}
