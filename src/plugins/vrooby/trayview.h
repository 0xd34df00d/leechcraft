/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QQuickWidget>

namespace LC::Vrooby
{
	class DevBackend;
	class FlatMountableItems;
	class TrayProxyModel;

	class TrayView : public QQuickWidget
	{
		Q_OBJECT

		TrayProxyModel *TrayModel_;
		DevBackend *Backend_ = nullptr;
	public:
		explicit TrayView ();

		void SetBackend (DevBackend*);
		bool HasItems () const;
	private slots:
		void toggleHide (const QString&);
		void toggleShowHidden ();
	signals:
		void hasItemsChanged ();
	};
}
