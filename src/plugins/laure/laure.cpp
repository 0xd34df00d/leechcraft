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
#include "lastfmsubmitter.h"

namespace LeechCraft
{
namespace Laure
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		// Ignotus is a fucking moron with total lack of knowledge and
		// skill and the ability to read documentation. Triple-check his code.
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"lauresettings.xml");

		Proxy_ = proxy;
		LFSubmitter_ = new LastFMSubmitter (proxy, this);
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
		w->Init (Proxy_);
		connect (w,
				SIGNAL (needToClose ()),
				this,
				SLOT (handleNeedToClose ()));

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
