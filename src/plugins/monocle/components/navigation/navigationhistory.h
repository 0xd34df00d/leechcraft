/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <QObject>
#include <QList>
#include <QMenu>

class QMenu;
class QAction;

namespace LC::Monocle
{
	struct ExternalNavigationAction;

	class NavigationHistory : public QObject
	{
		Q_OBJECT

		struct Actions
		{
			QAction Back_;
			QMenu BackMenu_;
			QAction Forward_;
			QMenu ForwardMenu_;

			explicit Actions ();

			Actions (const Actions&) = delete;
			Actions (Actions&&) = delete;
			Actions& operator= (const Actions&) = delete;
			Actions& operator= (Actions&&) = delete;
		};

		std::unique_ptr<Actions> Actions_;

		std::optional<QAction*> CurrentAction_;
	public:
		using PositionGetter = std::function<ExternalNavigationAction ()>;
	private:
		const PositionGetter PosGetter_;
	public:
		explicit NavigationHistory (PositionGetter, QObject* = nullptr);

		Actions& GetActions () const;

		void SaveCurrentPos ();
	private:
		QAction* MakeCurrentPositionAction ();
		void GoTo (QAction*, const ExternalNavigationAction&);
	signals:
		void navigationRequested (const ExternalNavigationAction&);
	};
}
