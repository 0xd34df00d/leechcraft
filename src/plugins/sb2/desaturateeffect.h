/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsEffect>

namespace LC::SB2
{
	class DesaturateEffect : public QGraphicsEffect
	{
		Q_OBJECT

		Q_PROPERTY (qreal strength READ GetStrength WRITE SetStrength NOTIFY strengthChanged)

		float Strength_ = 0;
	public:
		using QGraphicsEffect::QGraphicsEffect;

		qreal GetStrength () const;
		void SetStrength (qreal);
	protected:
		void draw (QPainter*) final;
	signals:
		void strengthChanged ();
	};
}
