/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_microblogstab.h"

namespace LC
{
namespace Azoth
{
	class IAccount;

	class MicroblogsTab : public QWidget
						, public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		static QObject* S_ParentMultiTabs_;
		static TabClassInfo S_TC_;

		Ui::MicroblogsTab Ui_;

		IAccount *Account_;
	public:
		static void SetTabData (QObject*, const TabClassInfo&);

		explicit MicroblogsTab (IAccount*);

		TabClassInfo GetTabClassInfo () const override;
		QObject* ParentMultiTabs () override;
		void Remove () override;
		QToolBar* GetToolBar () const override;
	signals:
		void removeTab () override;
	};
}
}
