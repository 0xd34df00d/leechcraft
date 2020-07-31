/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWizardPage>

class QLabel;

namespace LC
{
namespace BitTorrent
{
	class IntroPage : public QWizardPage
	{
		Q_OBJECT

		QLabel *Label_;
	public:
		IntroPage (QWidget *parent = 0);
	};
}
}
