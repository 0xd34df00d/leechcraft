/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bodyfetch.h"
#include <cstring>
#include <QIcon>
#include <interfaces/iscriptloader.h>
#include <interfaces/idownload.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/xpc/util.h>
#include <util/sys/paths.h>
#include <util/sll/prelude.h>
#include <util/sll/either.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include "workerobject.h"

namespace LC
{
namespace Aggregator
{
namespace BodyFetch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		StorageDir_ = Util::CreateIfNotExists ("aggregator/bodyfetcher/storage");

		static const auto suffixLength = std::strlen (".html");
		for (int i = 0; i < 10; ++i)
		{
			const QString& name = QString::number (i);
			if (!StorageDir_.exists (name))
				StorageDir_.mkdir (name);
			else
			{
				QDir dir = StorageDir_;
				dir.cd (name);
				FetchedItems_ = Util::MapAs<QSet> (dir.entryList (),
						[] (QString& name)
						{
							name.chop (suffixLength);
							return name.toULongLong ();
						});
			}
		}

		Proxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
		WO_ = new WorkerObject (AggregatorProxy_, this);

		IScriptLoader *loader = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IScriptLoader*> ().value (0, 0);
		if (!loader)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find a suitable loader, aborting";
			return;
		}

		const auto& inst = loader->CreateScriptLoaderInstance ("aggregator/recipes/");
		if (!inst)
		{
			qWarning () << Q_FUNC_INFO
					<< "got a null script loader instance";
			return;
		}

		inst->AddGlobalPrefix ();
		inst->AddLocalPrefix ();

		WO_->SetLoaderInstance (inst);

		connect (WO_,
				SIGNAL (downloadRequested (QUrl)),
				this,
				SLOT (handleDownload (QUrl)));
		connect (WO_,
				SIGNAL (newBodyFetched (quint64)),
				this,
				SLOT (handleBodyFetched (quint64)),
				Qt::QueuedConnection);
		connect (this,
				SIGNAL (downloadFinished (QUrl, QString)),
				WO_,
				SLOT (handleDownloadFinished (QUrl, QString)));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Aggregator.BodyFetch";
	}

	void Plugin::Release ()
	{
		delete WO_;
	}

	QString Plugin::GetName () const
	{
		return "Aggregator BodyFetch";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Fetches full bodies of news items following links in them.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Aggregator.GeneralPlugin/1.0";
		return result;
	}

	void Plugin::InitPlugin (IProxyObject *proxy)
	{
		AggregatorProxy_ = proxy;
	}

	void Plugin::hookItemLoad (IHookProxy_ptr, Item *item)
	{
		if (!FetchedItems_.contains (item->ItemID_))
			return;

		const quint64 id = item->ItemID_;
		if (!ContentsCache_.contains (id))
		{
			QDir dir = StorageDir_;

			dir.cd (QString::number (id % 10));
			QFile file (dir.filePath (QString ("%1.html").arg (id)));
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open file:"
						<< file.fileName ()
						<< file.errorString ();
				return;
			}

			ContentsCache_ [id] = QString::fromUtf8 (file.readAll ());
		}

		const QString& contents = ContentsCache_ [id];
		if (!contents.isEmpty ())
			item->Description_ = contents;
	}

	void Plugin::hookItemAdded (IHookProxy_ptr, const Item& item)
	{
		if (!WO_ || !WO_->IsOk ())
			return;

		WO_->AppendItem (item);
	}

	void Plugin::handleDownload (QUrl url)
	{
		const QString& temp = Util::GetTemporaryName ("agg_bodyfetcher");

		const Entity& e = Util::MakeEntity (url,
				temp,
				Internal |
					DoNotNotifyUser |
					DoNotSaveInHistory |
					OnlyDownload |
					NotPersistent);
		auto result = Proxy_->GetEntityManager ()->DelegateEntity (e);
		if (!result)
		{
			qWarning () << Q_FUNC_INFO
					<< "delegation failed";
			return;
		}

		Util::Sequence (this, result.DownloadResult_) >>
				Util::Visitor
				{
					[this, url, temp] (IDownload::Success) { emit downloadFinished (url, temp); },
					[] (const IDownload::Error&) {}
				};
	}

	void Plugin::handleBodyFetched (quint64 id)
	{
		FetchedItems_ << id;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_aggregator_bodyfetch, LC::Aggregator::BodyFetch::Plugin);
