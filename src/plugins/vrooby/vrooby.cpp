/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vrooby.h"
#include <QIcon>
#include <QAction>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/gui/geometry.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/sll/delayedexecutor.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>

#ifdef ENABLE_UDISKS
#include "backends/udisks/udisksbackend.h"
#endif
#ifdef ENABLE_UDISKS2
#include "backends/udisks2/udisks2backend.h"
#endif

#include "devbackend.h"
#include "trayview.h"

namespace LC
{
namespace Vrooby
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("vrooby");

		TrayView_ = new TrayView (proxy);
		new Util::UnhoverDeleteMixin (TrayView_, SLOT (hide ()));

		QList<std::shared_ptr<DevBackend>> candidates;

#ifdef ENABLE_UDISKS2
		candidates << std::make_shared<UDisks2::Backend> (proxy);
#endif
#ifdef ENABLE_UDISKS
		candidates << std::make_shared<UDisks::Backend> (proxy);
#endif

		QStringList allBackends;
		for (const auto& cand : candidates)
		{
			allBackends << cand->GetBackendName ();
			if (cand->IsAvailable ())
			{
				qDebug () << Q_FUNC_INFO
						<< "selecting"
						<< cand->GetBackendName ();
				Backend_ = cand;
				break;
			}
		}

		if (!Backend_)
		{
			const auto& e = Util::MakeNotification ("Vrooby",
					tr ("No backends are available, tried the following: %1.")
						.arg (allBackends.join ("; ")),
					Priority::Critical);
			Util::ExecuteLater ([e, proxy] { proxy->GetEntityManager ()->HandleEntity (e); });
		}
	}

	void Plugin::SecondInit ()
	{
		if (!Backend_)
			return;

		Backend_->Start ();

		TrayView_->SetBackend (Backend_.get ());
		connect (TrayView_,
				SIGNAL (hasItemsChanged ()),
				this,
				SLOT (checkAction ()));

		checkAction ();
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
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	bool Plugin::SupportsDevType (DeviceType type) const
	{
		return type == DeviceType::MassStorage;
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
			return;
		}

		ActionDevices_.reset (new QAction (tr ("Removable devices..."), this));
		ActionDevices_->setProperty ("ActionIcon", "drive-removable-media-usb");

		connect (ActionDevices_.get (),
				SIGNAL (triggered ()),
				this,
				SLOT (showTrayView ()));
		emit gotActions ({ ActionDevices_.get () }, ActionsEmbedPlace::LCTray);
	}

	void Plugin::showTrayView ()
	{
		const auto shouldShow = !TrayView_->isVisible ();
		if (shouldShow)
			TrayView_->move (Util::FitRectScreen (QCursor::pos (),
					TrayView_->size (), Util::FitFlag::NoOverlap));
		TrayView_->setVisible (shouldShow);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_vrooby, LC::Vrooby::Plugin);
