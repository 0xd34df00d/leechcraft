/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/iauthwidget.h>
#include "ui_deliciousauthwidget.h"

namespace LC
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace Delicious
{
	class DeliciousAuthWidget : public QWidget
								, public IAuthWidget
	{
		Q_OBJECT
		Q_INTERFACES (LC::Poshuku::OnlineBookmarks::IAuthWidget)

		Ui::AuthWidget Ui_;
	public:
		DeliciousAuthWidget (QWidget* = 0);
		QVariantMap GetIdentifyingData () const;
		void SetIdentifyingData (const QVariantMap&);
	};
}
}
}
}
