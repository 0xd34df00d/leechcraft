/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "effects.h"

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	bool operator== (const InvertEffect& ef1, const InvertEffect& ef2)
	{
		return ef1.Threshold_ == ef2.Threshold_;
	}

	bool operator!= (const InvertEffect& ef1, const InvertEffect& ef2)
	{
		return !(ef1 == ef2);
	}

	bool operator== (const LightnessEffect& ef1, const LightnessEffect& ef2)
	{
		return ef1.Factor_ == ef2.Factor_;
	}

	bool operator!= (const LightnessEffect& ef1, const LightnessEffect& ef2)
	{
		return !(ef1 == ef2);
	}

	bool operator== (const ColorTempEffect& ef1, const ColorTempEffect& ef2)
	{
		return ef1.Temperature_ == ef2.Temperature_;
	}

	bool operator!= (const ColorTempEffect& ef1, const ColorTempEffect& ef2)
	{
		return ef1.Temperature_ != ef2.Temperature_;
	}
}
}
}
