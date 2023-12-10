/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QApplication>
#include <QCommandLineParser>
#include <QTranslator>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include "appinfo.h"
#include "crashdialog.h"

namespace CrashProcess = LC::AnHero::CrashProcess;

namespace
{
	CrashProcess::AppInfo ParseOptions ()
	{
		using namespace LC;

		QCommandLineParser parser;
		parser.setApplicationDescription ("LeechCraft crash handler process (which is not intended to be run manually)."_qs);
		parser.addHelpOption ();

		const auto mkOpt = [&] (const QCommandLineOption& option)
		{
			parser.addOption (option);
			return option;
		};

		const auto strOpt = [&] (const QString& name, const QString& descr) { return mkOpt ({ name, descr, name }); };
		const auto optSignal = strOpt ("signal"_qs, "The signal that triggered the crash handler."_qs);
		const auto optPid = strOpt ("pid"_qs, "The PID of the crashed process."_qs);
		const auto optPath = strOpt ("path"_qs, "The application path of the crashed process."_qs);
		const auto optVersion = strOpt ("version"_qs, "The LeechCraft version at the moment of the crash."_qs);
		const auto optCmdline = strOpt ("cmdline"_qs, "The command line LeechCraft was started with."_qs);
		const auto optTsfile = strOpt ("tsfile"_qs, "The translation file to use for the UI strings."_qs);
		const auto optRestart = mkOpt ({ "suggest_restart"_qs, "Suggest restarting LeechCraft."_qs, "0|1"_qs });
		parser.process (*QCoreApplication::instance ());

		const auto require = [&] (const QCommandLineOption& option)
		{
			const auto& values = parser.values (option);
			if (values.size () != 1)
			{
				qWarning () << "expected exactly one value for" << option.names ().value (0);
				parser.showHelp ();
			}
			return values.value (0);
		};
		const auto getNum = [&] (auto getter, const QString& value)
		{
			bool ok = false;
			const auto res = (value.*getter) (&ok, 10);
			if (ok)
				return res;
			if (value.isEmpty ())
				return decltype (res) {};

			qWarning () << value << "unexpected";
			parser.showHelp ();
		};

		return
		{
			.Signal_ = getNum (&QString::toInt, parser.value (optSignal)),
			.PID_ = getNum (&QString::toULongLong, require (optPid)),
			.Path_ = require (optPath),
			.Version_ = require (optVersion),
			.ExecLine_ = parser.value (optCmdline),
			.TsFile_ = parser.value (optTsfile),
			.SuggestRestart_ = getNum (&QString::toInt, parser.value (optRestart)) != 0,
		};
	}
}

int main (int argc, char **argv)
{
	const QApplication app { argc, argv };

	const auto& info = ParseOptions ();

	QTranslator translator;
	if (!translator.load (info.TsFile_, {}, {}, ""))
		qWarning () << "unable to load translations from"
				<< info.TsFile_;

	QApplication::installTranslator (&translator);

	new CrashProcess::CrashDialog (info);
	return app.exec ();
}
