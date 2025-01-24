/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtGlobal>
#include <QQuickItem>

namespace LC
{
namespace Mellonetray
{
	class IconHandler : public QQuickItem
	{
		Q_OBJECT
		Q_PROPERTY (unsigned long wid READ GetWID WRITE SetWID NOTIFY widChanged)

		std::shared_ptr<QWindow> Proxy_;
		unsigned long WID_ = 0;
	public:
		IconHandler (QQuickItem* = 0);

		ulong GetWID () const;
		void SetWID (const ulong&);
	protected:
		void geometryChange (const QRectF&, const QRectF&) override;
	signals:
		void widChanged ();
	};
}
}
