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
#include <QPointer>

namespace LC
{
namespace SB2
{
	class SBView : public QQuickWidget
	{
		int Dim_ = 32;

		struct UnhoverItem
		{
			QPointer<QQuickItem> Item_;
			QPointF OldPos_;
		};
		QList<UnhoverItem> UnhoverItems_;
	public:
		SBView (QWidget* = 0);

		void SetDimensions (int);

		QSize minimumSizeHint () const override;
		QSize sizeHint () const override;
	protected:
		void enterEvent (QEvent*) override;
		void leaveEvent (QEvent*) override;
	};
}
}
