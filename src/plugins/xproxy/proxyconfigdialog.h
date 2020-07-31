/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_proxyconfigdialog.h"

namespace LC
{
namespace XProxy
{
	struct Proxy;

	class ProxyConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::ProxyConfigDialog Ui_;
	public:
		ProxyConfigDialog (QWidget*);

		Proxy GetProxy () const;
		void SetProxy (const Proxy&);
	};
}
}
