/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clargs.h"
#include <QCommandLineParser>
#include <QDir>
#include <QStringList>
#include <QUrl>
#include <QtDebug>
#include <interfaces/structures.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>

namespace LC::CL
{
	namespace
	{
		void SetupParser (QCommandLineParser& parser)
		{
			QCommandLineOption entity { { "E"_qs, "entity"_qs }, "the entity to handle"_qs, "entity"_qs };
			entity.setFlags (QCommandLineOption::HiddenFromHelp);

			parser.addOptions ({
					{ "help"_qs, "show the help message"_qs },
					{ { "v"_qs, "version"_qs }, "print LC version"_qs },
					{ { "D"_qs, "download"_qs }, "only choose downloaders for the entity: it should be downloaded but not handled"_qs },
					{ { "H"_qs, "handle"_qs }, "only choose handlers for the entity: it should be handled but not downloaded"_qs },
					{ { "T"_qs, "type"_qs }, "the type of the entity: url, url_encoded, file (for file paths) and such"_qs, "type"_qs },
					{ { "A"_qs, "additional"_qs }, "parameters for the Additional entity vmap in the form of name:value"_qs, "name:value"_qs },
					{ "automatic"_qs, "the entity is a result of some automatic stuff, not user's actions"_qs },
					{ "bt"_qs, "print backtraces for warning messages into warning.log"_qs },
					{ { "P"_qs, "plugin"_qs }, "load only given plugin and ignore already running instances of LC"_qs, "plugin"_qs },
					{ { "M"_qs, "multiprocess"_qs }, "load plugins in separate processes"_qs },
					{ "nolog"_qs, "disable custom file logger and print everything to stdout/stderr"_qs },
					{ "clear-socket"_qs, "clear stalled socket, use if you believe previous LC instance has terminated but failed to close its local socket properly"_qs },
					{ "no-app-catch"_qs, "disable exceptions catch-all in QApplication::notify(), useful for debugging purposes"_qs },
					{ "safe-mode"_qs, "disable all plugins so that you can manually enable them in Settings later"_qs },
					{ "list-plugins"_qs, "list all non-adapted plugins that were found and exit (this one doesn't check if plugins are valid and loadable)"_qs },
					{ "no-resource-caching"_qs, "disable caching of dynamic loadable resources (useful for stuff like Azoth themes development)"_qs },
					{ "autorestart"_qs, "automatically restart LC if it's closed (not guaranteed to work everywhere, especially on Windows and Mac OS X)"_qs },
					{ "desktop"_qs, "run LC as the desktop environment (used by leechcraft-session)"_qs },
					{ "minimized"_qs, "start LC minimized to tray"_qs },
					{ "no-splash-screen"_qs, "do not show the splash screen"_qs },
					{ "restart"_qs, "restart the LC"_qs },
					entity,
				});
			parser.addPositionalArgument ("entity"_qs, "the entity to handle"_qs, "[entity...]"_qs);
		}

		QVariantMap GetAdditionalMap (const QCommandLineParser& parser)
		{
			QVariantMap addMap;
			for (const auto& str : parser.values ("additional"_qs))
			{
				const auto& name = str.section (':', 0, 0);
				const auto& value = str.section (':', 1);
				if (value.isEmpty ())
				{
					qWarning () << "malformed Additional parameter:"
							<< str;
					continue;
				}

				if (name == " Tags")
				{
					qWarning () << "ignoring" << name << "since tag IDs can't be given on the command line";
					continue;
				}

				addMap [name] = value;
			}
			return addMap;
		}

		TaskParameters GetTaskParams (const QCommandLineParser& parser)
		{
			TaskParameters tp { FromCommandLine };
			if (parser.isSet ("automatic"_qs))
				tp |= AutoAccept;
			else
				tp |= FromUserInitiated;

			if (parser.isSet ("handle"_qs))
			{
				tp |= OnlyHandle;
				tp |= AutoAccept;
			}

			if (parser.isSet ("download"_qs))
			{
				tp |= OnlyDownload;
				tp |= AutoAccept;
			}

			return tp;
		}

		QUrl ResolveLocalFile (const QString& entity, const QString& curDir)
		{
			if (QDir::isAbsolutePath (entity))
				return QUrl::fromLocalFile (entity);

			if (!QFileInfo { curDir }.isDir ())
				return QUrl::fromLocalFile (entity);

			return QUrl::fromLocalFile (QDir { curDir }.filePath (entity));
		}

		QVector<Entity> ParseEntities (const QCommandLineParser& parser, const QString& curDir)
		{
			const auto& rawEntities = parser.values ("entity"_qs) + parser.positionalArguments ();
			if (rawEntities.isEmpty ())
				return {};

			const auto& tp = GetTaskParams (parser);
			const auto& type = parser.value ("type"_qs);
			const auto& addMap = GetAdditionalMap (parser);

			QVector<Entity> result;
			result.reserve (rawEntities.size ());
			for (const auto& entity : rawEntities)
			{
				QVariant ve;

				if (type == "url")
					ve = QUrl (entity);
				else if (type == "url_encoded")
					ve = QUrl::fromEncoded (entity.toUtf8 ());
				else if (type == "file")
					ve = ResolveLocalFile (entity, curDir);
				else
				{
					if (QFile::exists (entity))
						ve = QUrl::fromLocalFile (entity);
					else if (QUrl::fromEncoded (entity.toUtf8 ()).isValid ())
						ve = QUrl::fromEncoded (entity.toUtf8 ());
					else
						ve = entity;
				}

				auto e = Util::MakeEntity (ve,
						{},
						tp);
				e.Additional_ = addMap;
				qDebug () << e.Entity_ << e.Additional_;
				result << e;
			}
			return result;
		}
	}

	Util::Either<QString, Args> Parse (const QStringList& args, const QString& curDir)
	{
		QCommandLineParser parser;
		SetupParser (parser);

		// parse () skips the first argument as the program name
		if (!parser.parse (QStringList { QString {} } + args))
			return { Util::AsLeft, parser.errorText () };

		return Args
		{
			.Entities_ = ParseEntities (parser, curDir),

			.HelpRequested_ = parser.isSet ("help"_qs),
			.VersionRequested_ = parser.isSet ("version"_qs),
			.ListPluginsRequested_ = parser.isSet ("list-plugins"_qs),

			.NoResourceCaching_ = parser.isSet ("no-resource-caching"_qs),
			.NoSplashScreen_ = parser.isSet ("no-splash-screen"_qs),
			.Minimized_ = parser.isSet ("minimized"_qs),

			.SafeMode_ = parser.isSet ("safe-mode"_qs),

			.CatchExceptions_ = !parser.isSet ("no-app-catch"_qs),
			.NoLog_ = parser.isSet ("nolog"_qs),
			.Backtrace_ = parser.isSet ("bt"_qs),

			.ClearSocket_ = parser.isSet ("clear-socket"_qs),

			.Multiprocess_ = parser.isSet ("multiprocess"_qs),

			.Restart_ = parser.isSet ("restart"_qs),

			.Plugins_ = parser.values ("plugin"_qs),
		};
	}

	QString GetHelp ()
	{
		QCommandLineParser parser;
		SetupParser (parser);
		return parser.helpText ();
	}
}
