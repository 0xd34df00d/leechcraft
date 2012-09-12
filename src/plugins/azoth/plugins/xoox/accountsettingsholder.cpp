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

#include "accountsettingsholder.h"
#include <QTimer>
#include <interfaces/azoth/iproxyobject.h>
#include "glooxaccountconfigurationdialog.h"
#include "glooxaccount.h"
#include "core.h"
#include "accstatusrestorer.h"
#include "clientconnection.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	AccountSettingsHolder::AccountSettingsHolder (GlooxAccount *parent)
	: QObject (parent)
	, ReconnectScheduled_ (false)
	, Account_ (parent)
	, Port_ (-1)
	, KAParams_ (qMakePair (90, 60))
	, FileLogEnabled_ (false)
	, Priority_ (5)
	{
		connect (this,
				SIGNAL (jidChanged (QString)),
				this,
				SLOT (scheduleReconnect ()));
		connect (this,
				SIGNAL (resourceChanged (QString)),
				this,
				SLOT (scheduleReconnect ()));
		connect (this,
				SIGNAL (hostChanged (QString)),
				this,
				SLOT (scheduleReconnect ()));
		connect (this,
				SIGNAL (portChanged (int)),
				this,
				SLOT (scheduleReconnect ()));
	}

	void AccountSettingsHolder::Serialize (QDataStream& ostr) const
	{
		ostr << JID_
			<< Nick_
			<< Resource_
			<< Priority_
			<< Host_
			<< Port_
			<< KAParams_
			<< OurPhotoHash_
			<< FileLogEnabled_;
	}

	void AccountSettingsHolder::Deserialize (QDataStream& in, quint16 version)
	{
		in >> JID_
			>> Nick_
			>> Resource_
			>> Priority_;
		if (version >= 2)
			in >> Host_
				>> Port_;
		if (version >= 3)
			in >> KAParams_;
		if (version >= 4)
			in >> OurPhotoHash_;
		if (version >= 5)
			in >> FileLogEnabled_;
	}

	void AccountSettingsHolder::OpenConfigDialog ()
	{
		std::auto_ptr<GlooxAccountConfigurationDialog> dia (new GlooxAccountConfigurationDialog (0));
		if (!JID_.isEmpty ())
			dia->W ()->SetJID (JID_);
		if (!Nick_.isEmpty ())
			dia->W ()->SetNick (Nick_);
		if (!Resource_.isEmpty ())
			dia->W ()->SetResource (Resource_);
		if (!Host_.isEmpty ())
			dia->W ()->SetHost (Host_);
		if (Port_ >= 0)
			dia->W ()->SetPort (Port_);
		dia->W ()->SetPriority (Priority_);

		dia->W ()->SetKAInterval (KAParams_.first);
		dia->W ()->SetKATimeout (KAParams_.second);

		dia->W ()->SetFileLogEnabled (FileLogEnabled_);

		if (dia->exec () == QDialog::Rejected)
			return;

		FillSettings (dia->W ());
	}

	void AccountSettingsHolder::FillSettings (GlooxAccountConfigurationWidget *w)
	{
		SetJID (w->GetJID ());
		SetNick (w->GetNick ());
		SetResource (w->GetResource ());
		SetPriority (w->GetPriority ());
		SetHost (w->GetHost ());
		SetPort (w->GetPort ());
		SetFileLogEnabled (w->GetFileLogEnabled ());

		const QString& pass = w->GetPassword ();
		if (!pass.isNull ())
			Core::Instance ().GetPluginProxy ()->SetPassword (pass, this);

		SetKAParams (qMakePair (w->GetKAInterval (), w->GetKATimeout ()));

		emit accountSettingsChanged ();
	}

	QString AccountSettingsHolder::GetJID () const
	{
		return JID_;
	}

	void AccountSettingsHolder::SetJID (const QString& jid)
	{
		if (jid == JID_)
			return;

		JID_ = jid;
		emit jidChanged (JID_);
	}

	QString AccountSettingsHolder::GetNick () const
	{
		return Nick_.isEmpty () ? JID_ : Nick_;
	}

	void AccountSettingsHolder::SetNick (const QString& nick)
	{
		if (nick == Nick_)
			return;

		Nick_ = nick;
		emit nickChanged (Nick_);
	}

	QString AccountSettingsHolder::GetResource () const
	{
		return Resource_;
	}

	void AccountSettingsHolder::SetResource (const QString& resource)
	{
		if (resource == Resource_)
			return;

		Resource_ = resource;
		emit resourceChanged (Resource_);
	}

	QString AccountSettingsHolder::GetFullJID () const
	{
		return JID_ + "/" + Resource_;
	}

	QString AccountSettingsHolder::GetHost () const
	{
		return Host_;
	}

	void AccountSettingsHolder::SetHost (const QString& host)
	{
		if (host == Host_)
			return;

		Host_ = host;
		emit hostChanged (Host_);
	}

	int AccountSettingsHolder::GetPort () const
	{
		return Port_;
	}

	void AccountSettingsHolder::SetPort (int port)
	{
		if (port == Port_)
			return;

		Port_ = port;
		emit portChanged (Port_);
	}

	QByteArray AccountSettingsHolder::GetPhotoHash () const
	{
		return OurPhotoHash_;
	}

	void AccountSettingsHolder::SetPhotoHash (const QByteArray& hash)
	{
		if (hash == OurPhotoHash_)
			return;

		OurPhotoHash_ = hash;
		emit photoHashChanged (hash);
		emit accountSettingsChanged ();
	}

	QPair<int, int> AccountSettingsHolder::GetKAParams () const
	{
		return KAParams_;
	}

	void AccountSettingsHolder::SetKAParams (const QPair<int, int>& params)
	{
		if (params == KAParams_)
			return;

		KAParams_ = params;
		emit kaParamsChanged (KAParams_);
	}

	bool AccountSettingsHolder::GetFileLogEnabled () const
	{
		return FileLogEnabled_;
	}

	void AccountSettingsHolder::SetFileLogEnabled (bool enabled)
	{
		if (enabled == FileLogEnabled_)
			return;

		FileLogEnabled_ = enabled;
		emit fileLogChanged (FileLogEnabled_);
	}

	int AccountSettingsHolder::GetPriority () const
	{
		return Priority_;
	}

	void AccountSettingsHolder::SetPriority (int prio)
	{
		if (prio == Priority_)
			return;

		Priority_ = prio;
		emit priorityChanged (Priority_);
	}

	void AccountSettingsHolder::scheduleReconnect ()
	{
		if (ReconnectScheduled_)
			return;

		ReconnectScheduled_ = true;
		QTimer::singleShot (10,
				this,
				SLOT (handleReconnect ()));
	}

	void AccountSettingsHolder::handleReconnect ()
	{
		ReconnectScheduled_ = false;

		if (Account_->GetState ().State_ == SOffline)
			return;

		auto conn = Account_->GetClientConnection ();
		const auto state = conn->GetLastState ();
		Account_->ChangeState (EntryStatus (SOffline, Account_->GetState ().StatusString_));
		new AccStatusRestorer (state, conn);
	}
}
}
}
