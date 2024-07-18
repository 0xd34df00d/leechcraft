/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <optional>
#include <QList>
#include "interfaces/monocle/ilink.h"

class QMenu;
class QAction;

namespace LC::Monocle
{
	class NavigationHistory : public QObject
	{
		Q_OBJECT

		QMenu * const BackwardMenu_;
		QMenu * const ForwardMenu_;

		std::optional<QAction*> CurrentAction_;
	public:
		explicit NavigationHistory (QObject* = nullptr);

		QMenu* GetBackwardMenu () const;
		QMenu* GetForwardMenu () const;

		void GoBack () const;
		void GoForward () const;

		void SaveCurrentPos (const ExternalNavigationAction&);
	private:
		QAction* MakeCurrentPositionAction (const ExternalNavigationAction&);
		void GoTo (QAction*, const ExternalNavigationAction&);
	signals:
		void backwardHistoryAvailabilityChanged (bool);
		void forwardHistoryAvailabilityChanged (bool);

		void navigationRequested (const ExternalNavigationAction&);
	};
}
