/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "urllistscript.h"
#include <QUrl>
#include <QtDebug>
#include <QSettings>
#include <QCoreApplication>

namespace LC
{
namespace XProxy
{
	bool operator== (const HostInfo& left, const HostInfo& right)
	{
		return left.Port_ == right.Port_ &&
				left.Host_ == right.Host_ &&
				left.Scheme_ == right.Scheme_;
	}

	uint qHash (const HostInfo& info)
	{
		return qHash (info.Host_ + info.Scheme_) + info.Port_;
	}

	UrlListScript::UrlListScript (const IScript_ptr& script, QObject *parent)
	: QObject { parent }
	, Script_ { script }
	{
		script->AddQObject (this, "xproxy");
		ListName_ = Script_->InvokeMethod ("getListName").toString ();

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy_SavedScripts" };
		settings.beginGroup (GetListId ());
		SetUrlsImpl (settings.value ("Urls").toStringList ());
		LastUpdate_ = settings.value ("LastUpdate").toDateTime ();
		settings.endGroup ();
	}

	QByteArray UrlListScript::GetListId () const
	{
		return ListName_.toUtf8 ();
	}

	QString UrlListScript::GetListName () const
	{
		return ListName_;
	}

	void UrlListScript::SetEnabled (bool enabled)
	{
		if (enabled == IsEnabled_)
			return;

		IsEnabled_ = enabled;
		if (!IsEnabled_)
			return;

		if (!LastUpdate_.isValid () || LastUpdate_.secsTo (QDateTime::currentDateTime ()) > 60 * 60)
		{
			refresh ();
			LastUpdate_ = QDateTime::currentDateTime ();
		}
	}

	bool UrlListScript::Accepts (const QString& host, int port, const QString& proto)
	{
		return Hosts_.contains ({ host, port, proto }) ||
				Hosts_.contains ({ host, -1, proto });
	}

	void UrlListScript::setUrls (const QStringList& urls)
	{
		qDebug () << Q_FUNC_INFO << GetListId () << urls.size () << "; was" << Hosts_.size ();

		SetUrlsImpl (urls);

		LastUpdate_ = QDateTime::currentDateTime ();

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_XProxy_SavedScripts" };
		settings.beginGroup (GetListId ());
		settings.setValue ("Urls", urls);
		settings.setValue ("LastUpdate", LastUpdate_);
		settings.endGroup ();
	}

	void UrlListScript::SetUrlsImpl (const QStringList& urls)
	{
		Hosts_.clear ();
		for (const auto& urlStr : urls)
		{
			const auto& url = QUrl::fromEncoded (urlStr.toUtf8 ());
			Hosts_.insert ({ url.host (), url.port (), url.scheme () });
		}
	}

	void UrlListScript::refresh ()
	{
		qDebug () << Q_FUNC_INFO << GetListId ();
		Script_->InvokeMethod ("refresh");
	}
}
}
