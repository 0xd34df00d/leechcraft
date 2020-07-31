/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <interfaces/core/icoreproxy.h>
#include "ui_vcarddialog.h"
#include "structures.h"

namespace LC
{
namespace Azoth
{
class IAvatarsManager;

namespace Murm
{
	class VkEntry;
	class GeoResolver;

	class VCardDialog : public QDialog
	{
		Q_OBJECT

		Ui::VCardDialog Ui_;

		const ICoreProxy_ptr Proxy_;

		const UserInfo Info_;
	public:
		VCardDialog (VkEntry*, IAvatarsManager*, GeoResolver*, ICoreProxy_ptr, QWidget* = 0);
	private slots:
		void on_OpenVKPage__released ();
	};
}
}
}
