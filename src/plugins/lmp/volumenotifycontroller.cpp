/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "volumenotifycontroller.h"
#include <QTimer>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include "engine/output.h"
#include "core.h"

namespace LC
{
namespace LMP
{
	VolumeNotifyController::VolumeNotifyController (Output *out, QObject *parent)
	: QObject (parent)
	, Output_ (out)
	, NotifyTimer_ (new QTimer (this))
	{
		NotifyTimer_->setSingleShot (true);
		NotifyTimer_->setInterval (200);
		connect (NotifyTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (notify ()));
	}

	void VolumeNotifyController::volumeUp ()
	{
		Output_->setVolume (Output_->GetVolume () + 0.05);

		NotifyTimer_->start ();
	}

	void VolumeNotifyController::volumeDown ()
	{
		const auto val = std::max (Output_->GetVolume () - 0.05, 0.);
		Output_->setVolume (val);

		NotifyTimer_->start ();
	}

	void VolumeNotifyController::notify ()
	{
		auto e = Util::MakeNotification ("LMP",
				tr ("LMP volume has been changed to %1%.")
					.arg (static_cast<int> (Output_->GetVolume () * 100)),
				Priority::Info);
		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.LMP";
		e.Additional_ ["org.LC.AdvNotifications.EventID"] = "VolumeChange";
		Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}
}
}
