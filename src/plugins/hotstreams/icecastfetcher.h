/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#pragma once

#include <QObject>
#include <QIcon>

class QStandardItem;
class QNetworkAccessManager;

namespace LeechCraft
{
struct Entity;

namespace HotStreams
{
	class IcecastFetcher : public QObject
	{
		Q_OBJECT

		QStandardItem *Root_;
		int JobID_;
		QIcon RadioIcon_;
	public:
		IcecastFetcher (QStandardItem*, QNetworkAccessManager*, QObject* = 0);
	private:
		void FetchList ();
		void ParseList ();
	private slots:
		void handleFetchList ();
		void handleParsed ();
		void handleJobFinished (int);
		void checkDelete (int);
	signals:
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
