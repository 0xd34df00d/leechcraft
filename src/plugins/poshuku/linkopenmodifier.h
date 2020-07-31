/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/poshuku/ilinkopenmodifier.h"

namespace LC
{
	namespace Poshuku
{
	class LinkOpenModifier : public QObject
						   , public ILinkOpenModifier
	{
		Qt::MouseButtons MouseButtons_ = Qt::NoButton;
		Qt::KeyboardModifiers Modifiers_ = Qt::NoModifier;
	public:
		void InstallOn (QWidget*) override;

		OpenBehaviourSuggestion GetOpenBehaviourSuggestion () const override;
		void ResetSuggestionState () override;
	};
}
}
