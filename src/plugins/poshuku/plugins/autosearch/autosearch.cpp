/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "autosearch.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/poshuku/iurlcompletionmodel.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace Autosearch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
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
		return "org.LeechCraft.Poshuku.Plugin";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku Plugin";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides support for file:// scheme.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/plugins/poshuku/plugins/filescheme/resources/images/poshuku_filescheme.svg");
		return icon;
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
			int historyItems)
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
		reqUrl.addQueryItem ("hl", "en");
		reqUrl.addQueryItem ("output", "toolbar");
		reqUrl.addQueryItem ("q", string);

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

LC_EXPORT_PLUGIN (leechcraft_poshuku_autosearch, LeechCraft::Poshuku::Autosearch::Plugin);
