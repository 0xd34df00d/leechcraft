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
#include <QHash>
#include <interfaces/iinfo.h>
#include <interfaces/media/iradiostationprovider.h>

namespace LeechCraft
{
namespace HotStreams
{
	class Plugin : public QObject
				 , public IInfo
				 , public Media::IRadioStationProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo Media::IRadioStationProvider)

		ICoreProxy_ptr Proxy_;
		QHash<QString, QStandardItem*> Roots_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QList<QStandardItem*> GetRadioListItems () const;
		Media::IRadioStation_ptr GetRadioStation (QStandardItem* , const QString&);
	protected slots:
		void refreshRadios ();
	signals:
		void delegateEntity (const LeechCraft::Entity& entity, int*, QObject**);
	};
}
}

