/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QGraphicsEffect>
#include "effects.h"

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	class EffectProcessor : public QGraphicsEffect
	{
		QList<Effect_t> Effects_;
	public:
		EffectProcessor (QWidget*);

		void SetEffects (QList<Effect_t>);
	protected:
		void draw (QPainter*) override;
	};
}
}
}
