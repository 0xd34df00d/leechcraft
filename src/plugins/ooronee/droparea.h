/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>
#include <QQuickItem>

namespace LC
{
namespace Ooronee
{
	class DropArea : public QQuickItem
	{
		Q_OBJECT
		Q_PROPERTY (bool acceptingDrops READ GetAcceptingDrops WRITE SetAcceptingDrops NOTIFY acceptingDropsChanged)
	public:
		DropArea (QQuickItem* = 0);

		bool GetAcceptingDrops () const;
		void SetAcceptingDrops (bool);
	protected:
		void dragEnterEvent (QDragEnterEvent*) override;
		void dragLeaveEvent (QDragLeaveEvent*) override;
		void dropEvent (QDropEvent*) override;
	signals:
		void acceptingDropsChanged (bool);

		void dragEntered (const QVariant& data);
		void dragLeft ();

		void dataDropped (const QVariant& data);
	};
}
}
