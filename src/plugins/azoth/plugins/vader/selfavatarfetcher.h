/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
#include <QImage>
#include <QDateTime>

class QTimer;
class QUrl;

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
	class SelfAvatarFetcher : public QObject
	{
		Q_OBJECT

		QTimer *Timer_;

		QString Name_;
		QString Domain_;

		QDateTime PreviousDateTime_;
	public:
		SelfAvatarFetcher (QObject* = 0);

		void Restart (const QString&);
	private:
		QUrl GetReqURL () const;
	private slots:
		void refetch ();
		void handleHeadFinished ();
		void handleGetFinished ();
	signals:
		void gotImage (const QImage&);
	};
}
}
}
