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

#ifndef PLUGINS_AZOTH_PLUGINS_OTROID_OTROID_H
#define PLUGINS_AZOTH_PLUGINS_OTROID_OTROID_H
#include <QObject>
#include <QDir>

extern "C"
{
#include <libotr/proto.h>
#include <libotr/message.h>
}

#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iproxyobject.h>

class QTranslator;

namespace LeechCraft
{
namespace Azoth
{
namespace OTRoid
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		IProxyObject *AzothProxy_;

		OtrlUserState UserState_;
		OtrlMessageAppOps OtrOps_;

		QHash<QObject*, QAction*> Entry2Action_;

		QDir OtrDir_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		int IsLoggedIn (const QString& accId, const QString& entryId);
		void InjectMsg (const QString& accId,
				const QString& entryId, const QString& msg);
		void Notify (const QString& accId, const QString& entryId,
				Priority, const QString& title,
				const QString& primary, const QString& secondary);
		void WriteFingerprints ();
		void LogMsg (const QString&);
		QString GetAccountName (const QString& accId);
	private:
		const char* GetOTRFilename (const QString&) const;
		void CreateActions (QObject*);
	public slots:
		void initPlugin (QObject*);

		void hookEntryActionAreasRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);
		void hookEntryActionsRemoved (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
		void hookEntryActionsRequested (LeechCraft::IHookProxy_ptr proxy,
				QObject *entry);
		void hookGotMessage (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
		void hookMessageCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *message);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}

#endif
