/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QMetaType>

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	struct InvertEffect
	{
		int Threshold_ = 127;
	};

	bool operator== (const InvertEffect&, const InvertEffect&);
	bool operator!= (const InvertEffect&, const InvertEffect&);

	struct LightnessEffect
	{
		double Factor_ = 2;
	};

	bool operator== (const LightnessEffect&, const LightnessEffect&);
	bool operator!= (const LightnessEffect&, const LightnessEffect&);

	struct ColorTempEffect
	{
		int Temperature_ = 6500;
	};

	bool operator== (const ColorTempEffect&, const ColorTempEffect&);
	bool operator!= (const ColorTempEffect&, const ColorTempEffect&);

	using Effect_t = std::variant<InvertEffect, LightnessEffect, ColorTempEffect>;
}
}
}

Q_DECLARE_METATYPE (LC::Poshuku::DCAC::Effect_t)
