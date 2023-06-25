/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "clargs.h"
#include <boost/program_options.hpp>
#include <QDir>
#include <QStringList>
#include <QUrl>
#include <QtDebug>
#include <interfaces/structures.h>
#include <util/xpc/util.h>

namespace bpo = boost::program_options;

namespace LC::CL
{
	namespace
	{
		bpo::options_description GetOptions ()
		{
			bpo::options_description invisible ("Invisible options");
			invisible.add_options ()
					("entity,E", bpo::wvalue<std::vector<std::wstring>> (), "the entity to handle");

			bpo::options_description desc ("Allowed options");
			desc.add_options ()
					("help", "show the help message")
					("version,v", "print LC version")
					("download,D", "only choose downloaders for the entity: it should be downloaded but not handled")
					("handle,H", "only choose handlers for the entity: it should be handled but not downloaded")
					("type,T", bpo::value<std::string> (), "the type of the entity: url, url_encoded, file (for file paths) and such")
					("additional,A", bpo::value<std::vector<std::string>> (), "parameters for the Additional entity vmap in the form of name:value")
					("automatic", "the entity is a result of some automatic stuff, not user's actions")
					("bt", "print backtraces for warning messages into warning.log")
					("plugin,P", bpo::value<std::vector<std::string>> (), "load only given plugin and ignore already running instances of LC")
					("multiprocess,M", "load plugins in separate processes")
					("nolog", "disable custom file logger and print everything to stdout/stderr")
					("clear-socket", "clear stalled socket, use if you believe previous LC instance has terminated but failed to close its local socket properly")
					("no-app-catch", "disable exceptions catch-all in QApplication::notify(), useful for debugging purposes")
					("safe-mode", "disable all plugins so that you can manually enable them in Settings later")
					("list-plugins", "list all non-adapted plugins that were found and exit (this one doesn't check if plugins are valid and loadable)")
					("no-resource-caching", "disable caching of dynamic loadable resources (useful for stuff like Azoth themes development)")
					("autorestart", "automatically restart LC if it's closed (not guaranteed to work everywhere, especially on Windows and Mac OS X)")
					("minimized", "start LC minimized to tray")
					("no-splash-screen", "do not show the splash screen")
					("restart", "restart the LC");

			bpo::options_description all;
			all.add (desc);
			all.add (invisible);
			return all;
		}

		bpo::variables_map Parse (bpo::wcommand_line_parser& parser)
		{
			bpo::variables_map vm;

			bpo::positional_options_description pdesc;
			pdesc.add ("entity", -1);

			bpo::store (parser
						.options (GetOptions ())
						.positional (pdesc)
						.allow_unregistered ()
						.run (),
					vm);
			bpo::notify (vm);

			return vm;
		}

		QStringList ToStringList (const bpo::variables_map& vmap, const std::string& key)
		{
			if (!vmap.count (key))
				return {};

			QStringList result;
			for (const auto& str : vmap [key].as<std::vector<std::string>> ())
				result << QString::fromStdString (str);
			return result;
		}

		QString GetType (const bpo::variables_map& map)
		{
			const auto pos = map.find ("type");
			if (pos == map.end ())
				return {};

			return QString::fromStdString (pos->second.as<std::string> ());
		}

		QVariantMap GetAdditionalMap (const bpo::variables_map& map)
		{
			QVariantMap addMap;

			const auto pos = map.find ("additional");
			if (pos == map.end ())
				return addMap;

			for (const auto& add : pos->second.as<std::vector<std::string>> ())
			{
				const auto& str = QString::fromStdString (add);
				const auto& name = str.section (':', 0, 0);
				const auto& value = str.section (':', 1);
				if (value.isEmpty ())
				{
					qWarning () << "malformed Additional parameter:"
							<< str;
					continue;
				}

				addMap [name] = value;
			}
			return addMap;
		}

		TaskParameters GetTaskParams (const bpo::variables_map& map)
		{
			TaskParameters tp { FromCommandLine };
			if (map.count ("automatic"))
				tp |= AutoAccept;
			else
				tp |= FromUserInitiated;

			if (map.count ("handle"))
			{
				tp |= OnlyHandle;
				tp |= AutoAccept;
			}

			if (map.count ("download"))
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

		QVector<Entity> ParseEntities (const bpo::variables_map& map, const QString& curDir)
		{
			if (!map.count ("entity"))
				return {};

			const auto& tp = GetTaskParams (map);
			const auto& type = GetType (map);
			const auto& addMap = GetAdditionalMap (map);

			const auto& rawEntities = map ["entity"].as<std::vector<std::wstring>> ();
			QVector<Entity> result;
			result.reserve (rawEntities.size ());
			for (const auto& rawEntity : rawEntities)
			{
				QVariant ve;

				const auto& entity = QString::fromWCharArray (rawEntity.c_str ());

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

	Args Parse (const QStringList& args, const QString& curDir)
	{
		std::vector<std::wstring> strings;
		for (const auto& arg : args)
			strings.push_back (arg.toStdWString ());
		bpo::wcommand_line_parser parser (strings);
		auto vmap = Parse (parser);

		return
		{
			.Entities_ = ParseEntities (vmap, curDir),

			.HelpRequested_ = vmap.contains ("help"),
			.VersionRequested_ = vmap.contains ("version"),
			.ListPluginsRequested_ = vmap.contains ("list-plugins"),

			.NoResourceCaching_ = vmap.contains ("no-resource-caching"),
			.NoSplashScreen_ = vmap.contains ("no-splash-screen"),
			.Minimized_ = vmap.contains ("minimized"),

			.SafeMode_ = vmap.contains ("safe-mode"),

			.CatchExceptions_ = !vmap.contains ("no-app-catch"),
			.NoLog_ = vmap.contains ("nolog"),
			.Backtrace_ = vmap.contains ("bt"),

			.ClearSocket_ = vmap.contains ("clear-socket"),

			.Multiprocess_ = vmap.contains ("multiprocess"),

			.Restart_ = vmap.contains ("restart"),

			.Plugins_ = ToStringList (vmap, "plugin"),
		};
	}

	std::string GetHelp ()
	{
		const auto& opts = GetOptions ();
		std::stringstream ss;
		ss << opts;
		return ss.str ();
	}
}
