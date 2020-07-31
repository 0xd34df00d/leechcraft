/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_INBANDACCOUNTREGFIRSTPAGE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_INBANDACCOUNTREGFIRSTPAGE_H
#include <QWizardPage>
#include "ui_inbandaccountregfirstpage.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class InBandAccountRegFirstPage : public QWizardPage
	{
		Q_OBJECT

		Ui::InBandAccountRegFirstPage Ui_;
	public:
		InBandAccountRegFirstPage (QWidget* = 0);
		
		QString GetServerName () const;
		
		bool isComplete () const;
	};
}
}
}

#endif
