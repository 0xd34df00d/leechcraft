/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QAction>
#include <QCoreApplication>

class QToolBar;

namespace LC::Monocle
{
	class IDocument;

	class DocumentActions;
	class Navigator;
	class RecentlyOpenedManager;

	class ToolbarActions : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::ToolbarActions)
	public:
		struct Deps
		{
			Navigator& Navigator_;
			RecentlyOpenedManager& RecentlyOpenedManager_;

			QWidget& DocTabWidget_;
		};
	private:
		Deps Deps_;
		QToolBar& Bar_;

		std::unique_ptr<DocumentActions> DocActions_;
	public:
		explicit ToolbarActions (QToolBar&, const Deps&);
		~ToolbarActions () override;

		void HandleDoc (IDocument&);
	private:
		QWidget* MakeOpenButton ();
		void RunOpenDialog ();
	};
}
