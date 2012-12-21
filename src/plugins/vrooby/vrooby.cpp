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

#include "vrooby.h"
#include <QIcon>
#include <QAction>
#include <util/util.h>

#ifdef ENABLE_UDISKS
#include "backends/udisks/udisksbackend.h"
#else
#include "devbackend.h"
#endif

#include "trayview.h"

namespace LeechCraft
{
namespace Vrooby
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("vrooby");

		Backend_ = 0;
		TrayView_ = new TrayView (proxy);

#ifdef ENABLE_UDISKS
		Backend_ = new UDisks::Backend (this);
		connect (Backend_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));
#endif

		if (!Backend_)
			return;

		TrayView_->SetBackend (Backend_);
		connect (TrayView_,
				SIGNAL (hasItemsChanged ()),
				this,
				SLOT (checkAction ()));

		checkAction ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Vrooby";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Vrooby";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Removable storage devices manager for LeechCraft.");
	}

	QIcon Plugin::GetIcon () const
	{
		static QIcon icon (":/vrooby/resources/images/vrooby.svg");
		return icon;
	}

	QAbstractItemModel* Plugin::GetDevicesModel () const
	{
		return Backend_ ? Backend_->GetDevicesModel () : 0;
	}

	void Plugin::MountDevice (const QString& id)
	{
		if (Backend_)
			Backend_->MountDevice (id);
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == ActionsEmbedPlace::LCTray && ActionDevices_)
			result << ActionDevices_.get ();
		return result;
	}

	void Plugin::checkAction ()
	{
		if (!Backend_)
			return;

		if (TrayView_->HasItems () == static_cast<bool> (ActionDevices_))
			return;

		if (!TrayView_->HasItems ())
		{
			ActionDevices_.reset ();
			showTrayView (false);
			return;
		}

		ActionDevices_.reset (new QAction (tr ("Removable devices..."), this));
		ActionDevices_->setProperty ("ActionIcon", "drive-removable-media-usb");

		ActionDevices_->setCheckable (true);
		connect (ActionDevices_.get (),
				SIGNAL (toggled (bool)),
				this,
				SLOT (showTrayView (bool)));
		emit gotActions ({ ActionDevices_.get () }, ActionsEmbedPlace::LCTray);
	}

	void Plugin::showTrayView (bool show)
	{
		if (show && !TrayView_->isVisible ())
			TrayView_->move (Util::FitRectScreen (QCursor::pos (), TrayView_->size ()));
		TrayView_->setVisible (show);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_vrooby, LeechCraft::Vrooby::Plugin);
