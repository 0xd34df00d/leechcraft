/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "anhero.h"
#include <QCoreApplication>
#include <QFile>
#include <QIcon>
#include <QTranslator>
#include <charconv>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <cerrno>
#include <util/util.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <lcconfig.h>

namespace LC::AnHero
{
	namespace
	{
		QByteArray AppPath_;
		QByteArray AppDir_;
		QByteArray AppVersion_;
		QByteArray AppArgs_;
		QByteArray TsFile_;
		bool IsShuttingDown_ = false;

		void CloseFiles ()
		{
			rlimit rlp;
			getrlimit (RLIMIT_NOFILE, &rlp);
			for (rlim_t i = 3; i < rlp.rlim_cur; ++i)
				close (i);
		}

		void Exec (const char **argv)
		{
			pid_t pid = fork ();
			switch (pid)
			{
			case -1:
				fprintf (stderr, "%s: failed to fork(), errno: %d, str: %s\n", Q_FUNC_INFO, errno, strerror (errno));
				break;
			case 0:
				CloseFiles ();
				execvp (argv [0], const_cast<char**> (argv));
				fprintf (stderr, "%s: failed to exec(), errno: %d, str :%s\n", Q_FUNC_INFO, errno, strerror (errno));
				_exit (253);
				break;
			default:
				alarm (0);
				while (waitpid (pid, nullptr, 0) != pid)
					;
				break;
			}
		}

		namespace
		{
			auto ToChars (auto val)
			{
				constexpr auto maxLen = 10;
				std::array<char, maxLen> arr { { 0 } };
				std::to_chars (arr.begin (), arr.end (), val);
				return arr;
			}
		}

		void DefaultCrashHandler (int signal)
		{
			static uint8_t RecGuard = 0;
			if (RecGuard++)
				return;

			const auto sigtxt = ToChars (signal);
			const auto pidtxt = ToChars (QCoreApplication::applicationPid ());

#if defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
			std::array<char, 1024> crashprocess { {} };
			snprintf (crashprocess.data (), crashprocess.size (), "%s/lc_anhero_crashprocess" LC_LIBSUFFIX, AppDir_.constData ());
#endif

			const char *argv [] =
			{
#if defined (Q_OS_MAC) && !defined (USE_UNIX_LAYOUT)
				crashprocess.data (),
#else
				"lc_anhero_crashprocess" LC_LIBSUFFIX,
#endif
				"--signal",
				&sigtxt [0],
				"--pid",
				&pidtxt [0],
				"--path",
				AppPath_.constData (),
				"--version",
				AppVersion_.constData (),
				"--cmdline",
				AppArgs_.constData (),
				"--tsfile",
				TsFile_.constData (),
				"--suggest_restart",
				IsShuttingDown_ ? "0" : "1",
				nullptr
			};

			Exec (argv);
			const auto crashExitCode = 255;
			_exit (crashExitCode);
		}

		void SetCrashHandler (void (*handler) (int))
		{
			sigset_t mask;
			sigemptyset (&mask);

			auto add = [&mask, handler] (int sig)
			{
				signal (sig, handler);
				sigaddset (&mask, sig);
			};
#ifdef SIGSEGV
			add (SIGSEGV);
#endif
#ifdef SIGBUS
			add (SIGBUS);
#endif
#ifdef SIGFPE
			add (SIGFPE);
#endif
#ifdef SIGILL
			add (SIGILL);
#endif
#ifdef SIGABRT
			add (SIGABRT);
#endif

			sigprocmask (SIG_UNBLOCK, &mask, 0);
		}
	}

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		const std::unique_ptr<QTranslator> translator { Util::InstallTranslator ("anhero"_qs) };

		auto args = QCoreApplication::arguments ();
		if (args.contains ("-noanhero"_qs))
			return;

#ifdef Q_OS_MAC
		if (!QFile::exists ("/usr/bin/gdb"))
			return;
#endif

		args.removeFirst ();

		AppPath_ = QCoreApplication::applicationFilePath ().toUtf8 ();
		AppDir_ = QCoreApplication::applicationDirPath ().toUtf8 ();
		AppVersion_ = proxy->GetVersion ().toUtf8 ();
		AppArgs_ = args.join (' ').toUtf8 ();
		TsFile_ = translator ? translator->filePath ().toUtf8 () : QByteArray {};
		SetCrashHandler (DefaultCrashHandler);
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.AnHero";
	}

	QString Plugin::GetName () const
	{
		return "AnHero"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Crash handler");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	void Plugin::HandleShutdownInitiated ()
	{
		IsShuttingDown_ = true;
	}
}

LC_EXPORT_PLUGIN (leechcraft_anhero, LC::AnHero::Plugin);
