/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/imwproxy.h"

namespace LC
{
	class MainWindow;

	class MWProxy : public QObject
				  , public IMWProxy
	{
		Q_OBJECT
		Q_INTERFACES (IMWProxy)

		MainWindow *Win_;
	public:
		explicit MWProxy (MainWindow*, QObject* = nullptr);

		void AddDockWidget (QDockWidget*, const DockWidgetParams&) override;
		void AssociateDockWidget (QDockWidget*, QWidget*) override;
		void SetDockWidgetVisibility (QDockWidget*, bool) override;
		void ToggleViewActionVisiblity (QDockWidget*, bool) override;
		void SetViewActionShortcut (QDockWidget*, const QKeySequence&) override;

		void ToggleVisibility () override;
		void ShowMain () override;

		QMenu* GetMainMenu () override;
		void HideMainMenu () override;
	};
}
