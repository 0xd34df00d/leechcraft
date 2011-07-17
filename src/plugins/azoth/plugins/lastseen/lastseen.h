/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_LASTSEEN_LASTSEEN_H
#define PLUGINS_AZOTH_PLUGINS_LASTSEEN_LASTSEEN_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/azothcommon.h>

class QTranslator;

namespace LeechCraft
{
namespace Azoth
{
namespace LastSeen
{
	typedef QHash<QString, QDateTime> LastHash_t;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)
		
		boost::shared_ptr<QTranslator> Translator_;

		QHash<QString, State> LastState_;
		LastHash_t LastAvailable_;
		LastHash_t LastOnline_;
		
		bool SaveScheduled_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;
	private:
		void ScheduleSave ();
		void Load ();
	private slots:
		void save ();
	public slots:
		void hookEntryStatusChanged (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry,
				QString variant);
		void hookTooltipBeforeVariants (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
	};
}
}
}

#endif
