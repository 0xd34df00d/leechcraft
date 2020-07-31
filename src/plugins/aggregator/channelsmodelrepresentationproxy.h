/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIdentityProxyModel>

class QToolBar;
class QWidget;
class QMenu;

namespace LC
{
namespace Aggregator
{
	class ChannelsModelRepresentationProxy : public QIdentityProxyModel
	{
		QToolBar *Toolbar_ = nullptr;
		QWidget *TabWidget_ = nullptr;
		QMenu *Menu_ = nullptr;
	public:
		using QIdentityProxyModel::QIdentityProxyModel;

		QVariant data (const QModelIndex&, int) const override;

		void SetWidgets (QToolBar*, QWidget*);
		void SetMenu (QMenu*);
	};
}
}
