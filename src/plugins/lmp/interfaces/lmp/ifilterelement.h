/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "interfaces/lmp/ipath.h"

class QByteArray;

namespace LC
{
namespace LMP
{
	class IPath;
	class IFilterConfigurator;

	class IFilterElement
	{
	public:
		virtual ~IFilterElement () {}

		virtual QByteArray GetEffectId () const = 0;
		virtual QByteArray GetInstanceId () const = 0;

		virtual IFilterConfigurator* GetConfigurator () const = 0;

		void InsertInto (IPath *path)
		{
			path->InsertElement (GetElement ());
			PostAdd (path);
		}

		void RemoveFrom (IPath *path)
		{
			path->RemoveElement (GetElement ());
		}
	protected:
		virtual void PostAdd (IPath*)
		{
		}

		virtual GstElement* GetElement () const = 0;
	};
}
}
