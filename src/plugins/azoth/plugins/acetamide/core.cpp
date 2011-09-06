/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#include "core.h"
#include <QStandardItemModel>
#include <QRegExp>
#include <interfaces/iaccount.h>
#include <interfaces/iproxyobject.h>
#include "ircprotocol.h"
#include "nickservidentifywidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	Core::Core ()
	: PluginProxy_ (0)
	{
		IrcProtocol_.reset (new IrcProtocol (this));
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::Init ()
	{
		Model_ = new QStandardItemModel (this);
		NickServIdentifyWidget_ = new NickServIdentifyWidget (Model_);
	}

	void Core::SecondInit ()
	{
		IrcProtocol_->SetProxyObject (PluginProxy_);
		IrcProtocol_->Prepare ();
	}

	void Core::Release ()
	{
		IrcProtocol_.reset ();
	}

	QList<QObject*> Core::GetProtocols () const
	{
		QList<QObject*> result;
		result << qobject_cast<QObject*> (IrcProtocol_.get ());
		return result;
	}

	void Core::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	void Core::SetProxy (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	void Core::SendEntity (const LeechCraft::Entity& e)
	{
		emit gotEntity (e);
	}

	void Core::handleItemsAdded (const QList<QObject*>&)
	{
	}

	NickServIdentifyWidget* Core::GetNickServIdentifyWidget () const
	{
		return NickServIdentifyWidget_;
	}

	QStandardItemModel* Core::GetNickServIdentifyModel () const
	{
		return Model_;
	}

	void Core::AddNickServIdentify (const NickServIdentify& nsi)
	{
		if (NickServIdentifyList_.contains (nsi))
			return;

		NickServIdentifyList_ << nsi;
	}

	QList<NickServIdentify> Core::GetAllNickServIdentify () const
	{
		return NickServIdentifyList_;
	}

	QList<NickServIdentify> Core::GetNickServIdentifyWithNick (const QString& nick) const
	{
		QList<NickServIdentify> list;
		Q_FOREACH ( const NickServIdentify& nsi, NickServIdentifyList_)
			if (nsi.Nick_ == nick)
				list << nsi;
		return list;
	}

	QList<NickServIdentify> Core::GetNickServIdentifyWithNickServ (const QString& nickserv) const
	{
		QList<NickServIdentify> list;
		Q_FOREACH ( const NickServIdentify& nsi, NickServIdentifyList_)
		if (nsi.NickServNick_ == nickserv)
			list << nsi;
		return list;
	}

	QList<NickServIdentify> Core::GetNickServIdentifyWithServ (const QString& server) const
	{
		QList<NickServIdentify> list;
		Q_FOREACH ( const NickServIdentify& nsi, NickServIdentifyList_)
		if (nsi.Server_ == server)
			list << nsi;
		return list;
	}

	QList<NickServIdentify> Core::GetNickServIdentifyWithMainParams (const QString& server,
			const QString& nick, const QString& nickserv) const
	{
		QList<NickServIdentify> list;
		Q_FOREACH ( const NickServIdentify& nsi, NickServIdentifyList_)
		{
			QRegExp nickMask (nsi.NickServNick_,
					Qt::CaseInsensitive,
					QRegExp::Wildcard);
			if ((nsi.Server_ == server) &&
					(nsi.Nick_ == nick) &&
					(nickMask.indexIn (nickserv) == 0))
				list << nsi;
		}
		return list;
	}

}
}
}