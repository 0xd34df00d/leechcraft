/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "storage.h"
#include <stdexcept>
#include <QDir>
#include <QSqlError>
#include <plugininterface/dblock.h>
#include <plugininterface/util.h>
#include "repoinfo.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			namespace
			{
				QString LoadQuery (const QString& name)
				{
					QFile file (QString (":/resources/sql/%1.sql").arg (name));
					if (!file.open (QIODevice::ReadOnly))
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to open file"
								<< name
								<< "for reading";
						return QString ();
					}
					return file.readAll ();
				}

				QUrl Slashize (const QUrl& url)
				{
					if (url.path ().endsWith ('/'))
						return url;
					else
					{
						QUrl tmp = url;
						tmp.setPath (tmp.path () + '/');
						return tmp;
					}
				}
			}

			Storage::Storage (QObject *parent)
			: QObject (parent)
			, DB_ (QSqlDatabase::addDatabase ("QSQLITE", "LackManConnectionAvailable"))
			{
				QDir dir;
				try
				{
					dir = Util::CreateIfNotExists ("lackman");
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO << e.what ();
					throw;
				}

				DB_.setDatabaseName (dir.filePath ("availablepackages.db"));

				if (!DB_.open ())
				{
					qWarning () << Q_FUNC_INFO;
					Util::DBLock::DumpError (DB_.lastError ());
					throw std::runtime_error (qPrintable (QString ("Could not initialize database: %1")
								.arg (DB_.lastError ().text ())));
				}

				InitTables ();
				InitQueries ();
			}

			int Storage::CountPackages (const QUrl& repoUrl)
			{
				QueryCountPackages_.bindValue (":repo_url",
						Slashize (repoUrl).toEncoded ());
				if (!QueryCountPackages_.exec ())
				{
					Util::DBLock::DumpError (QueryCountPackages_);
					throw std::runtime_error ("Query execution failed.");
				}

				int value = 0;
				if (!QueryCountPackages_.next ())
					qWarning () << Q_FUNC_INFO
							<< "strange, next() returns false.";
				else
					value = QueryCountPackages_.value (0).toInt ();

				QueryCountPackages_.finish ();

				return value;
			}

			int Storage::FindRepo (const QUrl& repoUrl)
			{
				QueryFindRepo_.bindValue (":repo_url",
						Slashize (repoUrl).toEncoded ());
				if (!QueryFindRepo_.exec ())
				{
					Util::DBLock::DumpError (QueryFindRepo_);
					throw std::runtime_error ("Query execution failed.");
				}

				int value = -1;
				if (QueryFindRepo_.next ())
					value = QueryFindRepo_.value (0).toInt ();

				QueryFindRepo_.finish ();

				return value;
			}

			int Storage::AddRepo (const RepoInfo& ri)
			{
				Util::DBLock lock (DB_);
				try
				{
					lock.Init ();
				}
				catch (const std::runtime_error& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "could not acquire DB lock";
					throw;
				}

				QueryAddRepo_.bindValue (":url", Slashize (ri.GetUrl ()).toEncoded ());
				QueryAddRepo_.bindValue (":name", ri.GetName ());
				QueryAddRepo_.bindValue (":description", ri.GetShortDescr ());
				QueryAddRepo_.bindValue (":longdescr", ri.GetLongDescr ());
				QueryAddRepo_.bindValue (":maint_name", ri.GetMaintainer ().Name_);
				QueryAddRepo_.bindValue (":maint_email", ri.GetMaintainer ().Email_);
				if (!QueryAddRepo_.exec ())
				{
					Util::DBLock::DumpError (QueryAddRepo_);
					throw std::runtime_error ("Query execution failed.");
				}

				QueryAddRepo_.finish ();

				int repoId = FindRepo (Slashize (ri.GetUrl ()));
				if (repoId == -1)
				{
					qWarning () << Q_FUNC_INFO
							<< "OH SHI~, just inserted repo cannot be found!";
					throw std::runtime_error ("Just inserted repo cannot be found.");
				}

				Q_FOREACH (const QString& component, ri.GetComponents ())
				{
					QueryAddRepoComponents_.bindValue (":repo_id", repoId);
					QueryAddRepoComponents_.bindValue (":component", component);
					if (!QueryAddRepoComponents_.exec ())
					{
						Util::DBLock::DumpError (QueryAddRepoComponents_);
						throw std::runtime_error ("Query execution failed.");
					}

					QueryAddRepoComponents_.finish ();
				}

				lock.Good ();

				return repoId;
			}

			QStringList Storage::GetComponents (int repoId)
			{
				QueryGetRepoComponents_.bindValue (":repo_id", repoId);
				if (!QueryGetRepoComponents_.exec ())
				{
					Util::DBLock::DumpError (QueryGetRepoComponents_);
					throw std::runtime_error ("Query execution failed");
				}

				QStringList result;
				while (QueryGetRepoComponents_.next ())
					result << QueryGetRepoComponents_.value (0).toString ();

				QueryGetRepoComponents_.finish ();

				return result;
			}

			void Storage::InitTables ()
			{
				QSqlQuery query (DB_);
				QStringList names;
				names << "packages"
						<< "deps"
						<< "infos"
						<< "locations"
						<< "images"
						<< "tags"
						<< "repos"
						<< "components";
				Q_FOREACH (const QString& name, names)
					if (!DB_.tables ().contains (name))
						if (!query.exec (LoadQuery (QString ("create_table_%1").arg (name))))
						{
							Util::DBLock::DumpError (query);
							throw std::runtime_error ("Query execution failed.");
						}
			}

			void Storage::InitQueries ()
			{
				QueryCountPackages_ = QSqlQuery (DB_);
				QueryCountPackages_.prepare ("SELECT COUNT (package_id) "
						"FROM locations WHERE repo_url = :repo_url;");

				QueryFindRepo_ = QSqlQuery (DB_);
				QueryFindRepo_.prepare ("SELECT repo_id "
						"FROM repos WHERE url = :repo_url");

				QueryAddRepo_ = QSqlQuery (DB_);
				QueryAddRepo_.prepare (LoadQuery ("insert_repo"));

				QueryAddRepoComponents_ = QSqlQuery (DB_);
				QueryAddRepoComponents_.prepare ("INSERT INTO components (repo_id, component) "
						"VALUES (:repo_id, :component);");

				QueryGetRepoComponents_ = QSqlQuery (DB_);
				QueryGetRepoComponents_.prepare ("SELECT component "
						"FROM components WHERE repo_id = :repo_id;");
			}
		}
	}
}
