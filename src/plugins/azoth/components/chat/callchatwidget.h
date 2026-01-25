/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_callchatwidget.h"

namespace LC::Azoth
{
	class IMediaCall;

	class CallChatWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::CallChatWidget)

		Ui::CallChatWidget Ui_;
	public:
		explicit CallChatWidget (QObject*, QWidget* = nullptr);
	};
}
