/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QStandardItemModel>
#include <QRegExp>
#include <interfaces/azoth/iaccount.h>
#include <interfaces/azoth/iproxyobject.h>
#include "ircprotocol.h"
#include "nickservidentifywidget.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{
	Core::Core ()
	: IrcProtocol_ { std::make_shared<IrcProtocol> () }
	, Model_ { new QStandardItemModel { this } }
	, NickServIdentifyWidget_ { new NickServIdentifyWidget { Model_ } }
	{
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
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
		result << IrcProtocol_.get ();
		return result;
	}

	void Core::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	IProxyObject* Core::GetPluginProxy () const
	{
		return qobject_cast<IProxyObject*> (PluginProxy_);
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
		for (const auto& nsi : NickServIdentifyList_)
			if (nsi.Nick_ == nick)
				list << nsi;
		return list;
	}

	QList<NickServIdentify> Core::GetNickServIdentifyWithNickServ (const QString& nickserv) const
	{
		QList<NickServIdentify> list;
		for (const auto& nsi : NickServIdentifyList_)
			if (nsi.NickServNick_ == nickserv)
				list << nsi;
		return list;
	}

	QList<NickServIdentify> Core::GetNickServIdentifyWithServ (const QString& server) const
	{
		QList<NickServIdentify> list;
		for (const auto& nsi : NickServIdentifyList_)
			if (nsi.Server_ == server)
				list << nsi;
		return list;
	}

	QList<NickServIdentify> Core::GetNickServIdentifyWithMainParams (const QString& server,
			const QString& nick, const QString& nickserv) const
	{
		QList<NickServIdentify> list;
		for (const auto& nsi : NickServIdentifyList_)
		{
			QRegExp nickMask (nsi.NickServNick_,
					Qt::CaseInsensitive,
					QRegExp::Wildcard);
			if (nsi.Server_ == server &&
					nsi.Nick_ == nick &&
					nickMask.indexIn (nickserv) == 0)
				list << nsi;
		}
		return list;
	}
}
}
}
