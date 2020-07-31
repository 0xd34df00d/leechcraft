/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>
#include <QQuickWidget>

#include <interfaces/core/icoreproxy.h>

class QSortFilterProxyModel;

namespace LC
{
namespace Vrooby
{
	class DevBackend;
	class FlatMountableItems;
	class FilterModel;

	class TrayView : public QQuickWidget
	{
		Q_OBJECT

		ICoreProxy_ptr CoreProxy_;
		FlatMountableItems *Flattened_;
		FilterModel *Filtered_;

		DevBackend *Backend_;
	public:
		TrayView (ICoreProxy_ptr);

		void SetBackend (DevBackend*);
		bool HasItems () const;
	private slots:
		void toggleHide (const QString&);
		void toggleShowHidden ();
	signals:
		void hasItemsChanged ();
	};
}
}
