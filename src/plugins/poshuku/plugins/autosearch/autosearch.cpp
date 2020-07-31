/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "autosearch.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/poshuku/iurlcompletionmodel.h>
#include <util/util.h>
#include <util/sll/urloperator.h>

namespace LC
{
namespace Poshuku
{
namespace Autosearch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_autosearch");

		Proxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.Autosearch";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku Autosearch";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides support for Google search suggestions.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	void Plugin::hookURLCompletionNewStringRequested (IHookProxy_ptr,
			QObject *model,
			const QString& string,
			int)
	{
		if (Model2Reply_.contains (model))
		{
			auto reply = Model2Reply_.take (model);
			Reply2Model_.remove (reply);
			delete reply;
		}

		if (string.isEmpty ())
			return;

		QUrl reqUrl ("http://clients1.google.com/complete/search");
		Util::UrlOperator { reqUrl }
				("hl", "en")
				("output", "toolbar")
				("q", string);

		auto reply = Proxy_->GetNetworkAccessManager ()->get (QNetworkRequest (reqUrl));
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReply ()));
		Model2Reply_ [model] = reply;
		Reply2Model_ [reply] = model;
	}

	void Plugin::handleReply ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		reply->deleteLater ();

		if (!Reply2Model_.contains (reply))
		{
			qWarning () << Q_FUNC_INFO
					<< "stall reply detected";
			return;
		}

		const auto& data = reply->readAll ();

		auto model = Reply2Model_.take (reply);
		Model2Reply_.remove (model);

		QDomDocument doc;
		if (!doc.setContent (data))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to read reply";
			return;
		}

		auto iURLCompleter = qobject_cast<IURLCompletionModel*> (model);

		auto suggestion = doc.documentElement ().firstChildElement ("CompleteSuggestion");
		auto pos = 5;
		while (!suggestion.isNull ())
		{
			const auto& str = suggestion.firstChildElement ("suggestion").attribute ("data");

			iURLCompleter->AddItem (str, str, pos++);

			suggestion = suggestion.nextSiblingElement ("CompleteSuggestion");
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_autosearch, LC::Poshuku::Autosearch::Plugin);
