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

#include "bodyfetch.h"
#include <QIcon>
#include <interfaces/iscriptloader.h>
#include <plugininterface/util.h>
#include "workerthread.h"

namespace LeechCraft
{
namespace Aggregator
{
namespace BodyFetch
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		StorageDir_ = Util::CreateIfNotExists ("aggregator/bodyfetcher/storage");
		
		const int suffixLength = std::strlen (".html");
		for (int i = 0; i < 10; ++i)
		{
			const QString& name = QString::number (i);
			if (!StorageDir_.exists (name))
				StorageDir_.mkdir (name);
			else
			{
				QDir dir = StorageDir_;
				dir.cd (name);
				Q_FOREACH (QString name, dir.entryList ())
				{
					name.chop (suffixLength);
					FetchedItems_ << name.toLongLong ();
				}
			}
		}

		WT_ = 0;
		Proxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
		WT_ = new WorkerThread (this);
		connect (WT_,
				SIGNAL (started ()),
				this,
				SLOT (handleWTStarted ()),
				Qt::QueuedConnection);
		WT_->start (QThread::LowestPriority);
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Aggregator.BodyFetch";
	}

	void Plugin::Release ()
	{
		if (WT_)
		{
			connect (WT_,
					SIGNAL (finished ()),
					WT_,
					SLOT (deleteLater ()));			
			WT_->quit ();
			WT_ = 0;
		}
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
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Aggregator.GeneralPlugin/1.0";
		return result;
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
	
	void Plugin::hookGotNewItems (IHookProxy_ptr, QVariantList items)
	{
		if (!WT_)
			return;
		
		if (!WT_->IsOK ())
		{
			qWarning () << Q_FUNC_INFO
					<< "worker thread isn't ready yet :(";
			return;
		}
		
		WT_->AppendItems (items);
	}
	
	void Plugin::handleWTStarted ()
	{
		IScriptLoader *loader = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<IScriptLoader*> ().value (0, 0);
		if (!loader)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to find a suitable loader, aborting";
			return;
		}
		
		IScriptLoaderInstance *inst = loader->
				CreateScriptLoaderInstance ("aggregator/recipes/");
		if (!inst)
		{
			qWarning () << Q_FUNC_INFO
					<< "got a null script loader instance";
			return;
		}
		
		inst->AddGlobalPrefix ();
		inst->AddLocalPrefix ();
		
		inst->GetObject ()->moveToThread (WT_);
		WT_->SetLoaderInstance (inst);
		
		connect (WT_->GetWorkingObject (),
				SIGNAL (downloadRequested (QUrl)),
				this,
				SLOT (handleDownload (QUrl)),
				Qt::QueuedConnection);
		connect (WT_->GetWorkingObject (),
				SIGNAL (newBodyFetched (quint64)),
				this,
				SLOT (handleBodyFetched (quint64)),
				Qt::QueuedConnection);
		connect (this,
				SIGNAL (downloadFinished (QUrl, QString)),
				WT_->GetWorkingObject (),
				SLOT (handleDownloadFinished (QUrl, QString)));
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
		int id = -1;
		QObject *obj = 0;
		emit delegateEntity (e, &id, &obj);
		if (id == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< "delegation failed";
			return;
		}
		
		Jobs_ [id] = qMakePair (url, temp);
		
		connect (obj,
				SIGNAL (jobFinished (int)),
				this,
				SLOT (handleJobFinished (int)),
				Qt::UniqueConnection);
	}
	
	void Plugin::handleJobFinished (int id)
	{
		if (!Jobs_.contains (id))
			return;
		
		const QPair<QUrl, QString>& job = Jobs_.take (id);
		
		emit downloadFinished (job.first, job.second);
	}
	
	void Plugin::handleBodyFetched (quint64 id)
	{
		FetchedItems_ << id;
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_aggregator_bodyfetch, LeechCraft::Aggregator::BodyFetch::Plugin);
