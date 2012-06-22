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
#endif

#include "trayview.h"

namespace LeechCraft
{
namespace Vrooby
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Backend_ = 0;
		TrayView_ = new TrayView (proxy);

#ifdef ENABLE_UDISKS
		Backend_ = new UDisks::Backend (this);
#endif

		ActionDevices_ = new QAction (tr ("Removable devices..."), this);
		ActionDevices_->setProperty ("ActionIcon", "drive-removable-media-usb");

		ActionDevices_->setCheckable (true);
		connect (ActionDevices_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (showTrayView (bool)));

		if (Backend_)
			TrayView_->SetDevModel (Backend_->GetDevicesModel ());
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
		return QIcon ();
	}

	QAbstractItemModel* Plugin::GetDevicesModel () const
	{
		return Backend_ ? Backend_->GetDevicesModel () : 0;
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> result;
		if (aep == ActionsEmbedPlace::LCTray)
			result << ActionDevices_;
		return result;
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
