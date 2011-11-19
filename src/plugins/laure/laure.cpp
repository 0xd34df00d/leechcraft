/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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

#include "laure.h"
#include <QIcon>
#include <QUrl>
#include <QCoreApplication>
#include <util/util.h>
#include "laurewidget.h"
#include "xmlsettingsmanager.h"
#ifdef HAVE_LASTFM
#include "lastfmsubmitter.h"
#endif

namespace LeechCraft
{
namespace Laure
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"lauresettings.xml");

		Proxy_ = proxy;
#ifdef HAVE_LASTFM
		LFSubmitter_ = new LastFMSubmitter (this);
		
		handleSubmitterInit ();
		
		QList<QByteArray> propNames;
		propNames.push_back ("lastfm.login");
		propNames.push_back ("lastfm.password");
		
		XmlSettingsManager::Instance ().RegisterObject (propNames,
				this, "handleSubmitterInit");
#endif
		LaureWidget::SetParentMultiTabs (this);

		TabClassInfo tabClass =
		{
			"Laure",
			"Laure",
			GetInfo (),
			GetIcon (),
			50,
			TabFeatures (TFOpenableByRequest | TFByDefault)
		};
		TabClasses_ << tabClass;
	}
	
#ifdef HAVE_LASTFM	
	void Plugin::handleSubmitterInit ()
	{
		LFSubmitter_->SetUsername (XmlSettingsManager::Instance ()
				.property ("lastfm.login").toString ());
		LFSubmitter_->SetPassword (XmlSettingsManager::Instance ()
				.property ("lastfm.password").toString ());
		
		LFSubmitter_->Init (Proxy_->GetNetworkAccessManager ());
	}
#endif

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Laure";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Laure";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Media player based on libvlc");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return TabClasses_;
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (tabClass == "Laure")
			CreateTab ();
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
		}
	}

	void Plugin::handleNeedToClose ()
	{
		LaureWidget *w = qobject_cast<LaureWidget*> (sender ());
		if (!w)
		{
			qWarning () << Q_FUNC_INFO
				<< "not a LaureWidget*"
				<< sender ();
			return;
		}
		emit removeTab (w);
		Others_.removeAll (w);
		w->deleteLater ();
	}

	LaureWidget* Plugin::CreateTab ()
	{
		LaureWidget *w = new LaureWidget ();
		
		connect (w,
				SIGNAL (needToClose ()),
				this,
				SLOT (handleNeedToClose ()));
#ifdef HAVE_LASTFM
		connect (w,
				SIGNAL (currentTrackMeta (MediaMeta)),
				LFSubmitter_,
				SLOT (sendTrack (MediaMeta)));
		connect (w,
				SIGNAL (trackFinished ()),
				LFSubmitter_,
				SLOT (submit ()));
#endif
		connect (w,
				SIGNAL (gotEntity (Entity)),
				this,
				SIGNAL (gotEntity (Entity)));
		connect (w,
				SIGNAL (delegateEntity (Entity, int*, QObject**)),
				this,
				SIGNAL (delegateEntity (Entity, int*, QObject**)));

		Others_ << w;
		emit addNewTab (tr ("Laure"), w);
		emit changeTabIcon (w, QIcon ());
		emit raiseTab (w);
		return w;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}


	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		bool stat = false;
		if (entity.Mime_ == "application/ogg" ||
			entity.Mime_.startsWith ("audio/") ||
			entity.Mime_.startsWith ("video/"))
				stat = true;

		return stat ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity entity)
	{
		const QString& dest = entity.Entity_.toString ();
		LaureWidget *w = CreateTab ();
		w->handleOpenMediaContent (dest);
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_laure, LeechCraft::Laure::Plugin);
