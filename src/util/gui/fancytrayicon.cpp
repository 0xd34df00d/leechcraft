/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fancytrayicon.h"
#include <QMenu>
#include <QtDebug>
#include "fancytrayiconfallback.h"

#ifdef IS_FREEDESKTOP_PLATFORM
#include "fancytrayiconfreedesktop.h"
#endif

namespace LC::Util
{
	FancyTrayIcon::FancyTrayIcon (IconInfo info, QObject *parent)
	: QObject { parent }
	, Info_ { std::move (info) }
	{
		ReinitImpl ();
	}

	FancyTrayIcon::~FancyTrayIcon () = default;

	const FancyTrayIcon::IconInfo& FancyTrayIcon::GetInfo () const
	{
		return Info_;
	}

	void FancyTrayIcon::SetVisible (bool visible)
	{
		if (visible == Visible_)
			return;

		Visible_ = visible;
		if (!visible)
			Impl_.reset ();
		else
			ReinitImpl ();
	}

	void FancyTrayIcon::SetStatus (Status status)
	{
		Status_ = status;
		if (Impl_)
			Impl_->UpdateStatus ();
	}

	FancyTrayIcon::Status FancyTrayIcon::GetStatus () const
	{
		return Status_;
	}

	void FancyTrayIcon::SetIcon (const Icon& icon)
	{
		Icon_ = icon;
		if (Impl_)
			Impl_->UpdateIcon ();
	}

	const FancyTrayIcon::Icon& FancyTrayIcon::GetIcon () const
	{
		return Icon_;
	}

	void FancyTrayIcon::SetToolTip (Tooltip tooltip)
	{
		Tooltip_ = std::move (tooltip);
		if (Impl_)
			Impl_->UpdateTooltip ();
	}

	const FancyTrayIcon::Tooltip& FancyTrayIcon::GetTooltip () const
	{
		return Tooltip_;
	}

	void FancyTrayIcon::SetContextMenu (QMenu *menu)
	{
		Menu_ = menu;
		if (Impl_)
			Impl_->UpdateMenu ();
	}

	QMenu* FancyTrayIcon::GetContextMenu () const
	{
		return Menu_;
	}

	void FancyTrayIcon::ReinitImpl ()
	{
		try
		{
#ifdef IS_FREEDESKTOP_PLATFORM
			Impl_ = std::make_unique<FancyTrayIconFreedesktop> (*this);
#endif
		}
		catch (const std::exception& e)
		{
			qCritical () << "unable to create icon implementation:" << e.what ();
		}

		if (!Impl_)
			Impl_ = std::make_unique<FancyTrayIconFallback> (*this);
	}
}
