/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <variant>
#include <QObject>
#include <QIcon>
#include <QPointer>
#include "guiconfig.h"

class QMenu;

namespace LC::Util
{
	class FancyTrayIconImpl;

	class UTIL_GUI_API FancyTrayIcon : public QObject
	{
		Q_OBJECT
	public:
		struct IconInfo
		{
			QString Id_;
			QString Title_;
		};

		struct Tooltip
		{
			QString PlainText_;
			QString HTML_;
		};

		using Icon = std::variant<QString, QIcon>;
	private:
		std::unique_ptr<FancyTrayIconImpl> Impl_;

		const IconInfo Info_;

		bool Visible_ = true;
		Icon Icon_;
		Tooltip Tooltip_;
		QPointer<QMenu> Menu_;
	public:
		explicit FancyTrayIcon (IconInfo info, QObject *parent = nullptr);
		~FancyTrayIcon () override;

		const IconInfo& GetInfo () const;

		void SetVisible (bool visible);

		void SetIcon (const Icon& icon);
		const Icon& GetIcon () const;

		void SetToolTip (Tooltip tooltip);
		const Tooltip& GetTooltip () const;

		void SetContextMenu (QMenu *menu);
		QMenu* GetContextMenu () const;
	private:
		void ReinitImpl ();
	signals:
		void activated ();
		void secondaryActivated ();
		void scrolled (int);
	};
}
