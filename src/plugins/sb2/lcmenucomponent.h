/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/icoreproxy.h>

namespace LC::SB2
{
	class LCMenuComponent : public QObject
	{
		Q_OBJECT

		IMWProxy* Proxy_;
		const QuarkComponent_ptr Component_;
	public:
		explicit LCMenuComponent (IMWProxy*, QObject* = nullptr);

		QuarkComponent_ptr GetComponent () const;
	public slots:
		void execMenu ();
	};
}
