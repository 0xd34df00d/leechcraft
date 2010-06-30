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

#ifndef PLUGINS_LACKMAN_STORAGE_H
#define PLUGINS_LACKMAN_STORAGE_H
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>

class QUrl;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			class RepoInfo;

			class Storage : public QObject
			{
				Q_OBJECT

				QSqlDatabase DB_;

				QSqlQuery QueryCountPackages_;
				QSqlQuery QueryFindRepo_;
				QSqlQuery QueryAddRepo_;
				QSqlQuery QueryAddRepoComponents_;
				QSqlQuery QueryGetRepoComponents_;
			public:
				Storage (QObject* = 0);

				int CountPackages (const QUrl&);
				int FindRepo (const QUrl&);
				int AddRepo (const RepoInfo&);
				QStringList GetComponents (int);
			private:
				void InitTables ();
				void InitQueries ();
			};
		};
	};
};

#endif
