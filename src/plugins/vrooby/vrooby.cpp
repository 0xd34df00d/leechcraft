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
#include <QTimer>
#include <util/gui/geometry.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ientitymanager.h>
#include "backends/udisks2/udisks2backend.h"
#include "devbackend.h"
#include "trayview.h"

namespace LC
{
namespace Vrooby
{
	namespace
	{
		template<DevBackendType... Backends>
		auto SelectBackend ()
		{
			struct Result
			{
				QStringList Attempted_;
				std::shared_ptr<DevBackend> Selected_;
			};
			Result result;

			(([&]
			{
				result.Attempted_ << Backends::GetBackendName ();
				if (!Backends::IsAvailable ())
					return false;

				result.Selected_ = std::make_shared<Backends> ();
				qDebug () << "selecting" << Backends::GetBackendName ();
				return true;
			} ()) || ...);

			return result;
		}
	}
	void Plugin::Init (ICoreProxy_ptr)
	{
		TrayView_ = new TrayView ();
		new Util::UnhoverDeleteMixin (TrayView_, SLOT (hide ()));

		const auto& result = SelectBackend<UDisks2::Backend> ();
		Backend_ = result.Selected_;

		if (!Backend_)
		{
			qWarning () << "no backends are available";
			const auto& e = Util::MakeNotification ("Vrooby",
					tr ("No backends are available, tried the following: %1.").arg (result.Attempted_.join ("; ")),
					Priority::Critical);
			QTimer::singleShot (0, this, [e] { GetProxyHolder ()->GetEntityManager ()->HandleEntity (e); });
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
		return "org.LeechCraft.Vrooby"_qba;
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Vrooby"_qs;
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
		ActionDevices_->setProperty ("ActionIcon", "drive-removable-media-usb"_qs);

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
