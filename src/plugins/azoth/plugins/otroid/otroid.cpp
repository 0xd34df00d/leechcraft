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

#include "otroid.h"
#include <QCoreApplication>
#include <QIcon>
#include <libotr/privkey.h>
#include <interfaces/iaccount.h>
#include <interfaces/iclentry.h>
#include <interfaces/imessage.h>
#include <util/util.h>

namespace LeechCraft
{
namespace Azoth
{
namespace OTRoid
{
	namespace OTR
	{
		int IsLoggedIn (void *opData, const char *accName,
				const char*, const char *recipient)
		{
			Plugin *p = static_cast<Plugin*> (opData);
			return p->IsLoggedIn (QString::fromUtf8 (accName),
					QString::fromUtf8 (recipient));
		}

		void InjectMessage (void *opData, const char *accName,
				const char*, const char *recipient, const char *msg)
		{
			Plugin *p = static_cast<Plugin*> (opData);
			p->InjectMsg (QString::fromUtf8 (accName),
					QString::fromUtf8 (recipient),
					QString::fromUtf8 (msg));
		}
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
		OTRL_INIT;

		OtrDir_ = Util::CreateIfNotExists (".leechcraft/azoth/otr/");

		UserState_ = otrl_userstate_create ();

		otrl_privkey_read (UserState_, GetOTRFilename ("privkey"));
		otrl_privkey_read_fingerprints (UserState_,
				GetOTRFilename ("fingerprints"), NULL, NULL);

		memset (&OtrOps_, 0, sizeof (OtrOps_));
		OtrOps_.is_logged_in = &OTR::IsLoggedIn;
		OtrOps_.inject_message = &OTR::InjectMessage;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.OTRoid";
	}

	void Plugin::Release ()
	{
		otrl_userstate_free (UserState_);
	}

	QString Plugin::GetName () const
	{
		return "Azoth OTRoid";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Azoth OTRoid adds support for Off-the-Record deniable encryption system.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	int Plugin::IsLoggedIn (const QString& accId, const QString& entryId)
	{
		QObject *entryObj = AzothProxy_->GetEntry (entryId, accId);
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		if (!entry)
			return -1;

		return entry->Variants ().isEmpty () ? 0 : 1;
	}

	void Plugin::InjectMsg (const QString& accId,
			const QString& entryId, const QString& body)
	{
		QObject *entryObj = AzothProxy_->GetEntry (entryId, accId);
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		if (!entry)
			return;

		QObject *msgObj = entry->CreateMessage (IMessage::MTChatMessage,
				QString (), body);
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
			return;

		msg->Send ();
	}

	void Plugin::Notify (const QString& accId, const QString& entryId,
			Priority prio, const QString& title,
			const QString& prim, const QString& sec)
	{
		QString text = prim;
		if (!sec.isEmpty ())
			text += "<br />" + sec;

		emit gotEntity (Util::MakeNotification (title, text, prio));
	}

	void Plugin::initPlugin (QObject *obj)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (obj);
	}

	const char* Plugin::GetOTRFilename (const QString& fname) const
	{
		return OtrDir_.absoluteFilePath (fname).toUtf8 ().constData ();
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_otroid, LeechCraft::Azoth::OTRoid::Plugin);
