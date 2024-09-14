/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAction>
#include <QCoreApplication>

class QToolBar;

namespace LC::Monocle
{
	class IDocument;

	class Navigator;
	class RecentlyOpenedManager;

	class DocumentActions : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::DocumentActions)
	public:
		struct Deps
		{
			Navigator& Navigator_;
			RecentlyOpenedManager& RecentlyOpenedManager_;

			QWidget& DocTabWidget_;
		};
	private:
		Deps Deps_;
	public:
		explicit DocumentActions (QToolBar&, const Deps&);

		void HandleDocument (const IDocument&);
	private:
		QWidget* MakeOpenButton ();
		void RunOpenDialog ();
	};
}
