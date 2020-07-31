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
#include "interfaces/monocle/idocument.h"

class QMenu;
class QAction;

namespace LC
{
namespace Monocle
{
	class NavigationHistory : public QObject
	{
		Q_OBJECT
	public:
		struct Entry
		{
			QString Document_;
			IDocument::Position Position_;
		};

		using EntryGetter_f = std::function<Entry ()>;
	private:
		const EntryGetter_f EntryGetter_;

		QMenu * const BackwardMenu_;
		QMenu * const ForwardMenu_;

		std::optional<QAction*> CurrentAction_;
	public:
		NavigationHistory (const EntryGetter_f&, QObject* = nullptr);

		QMenu* GetBackwardMenu () const;
		QMenu* GetForwardMenu () const;

		void GoBack () const;
		void GoForward () const;

		void HandleSearchNavigationRequested ();
	private:
		void GoSingleAction (QMenu*) const;

		void AppendHistoryEntry ();
		QAction* MakeCurrentPositionAction ();
		void GoTo (QAction*, const Entry&);
	public slots:
		void handleDocumentNavigationRequested ();
	signals:
		void entryNavigationRequested (const Entry&);

		void backwardHistoryAvailabilityChanged (bool);
		void forwardHistoryAvailabilityChanged (bool);
	};
}
}
