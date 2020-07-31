/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_imagedialog.h"

namespace LC
{
namespace LHTR
{
	class ImageDialog : public QDialog
	{
		Q_OBJECT

		Ui::ImageDialog Ui_;
	public:
		ImageDialog (QWidget* = 0);

		QString GetPath () const;
		QString GetAlt () const;
		int GetWidth () const;
		int GetHeight () const;

		QString GetFloat () const;
	private slots:
		void on_TypeEmbed__toggled (bool);
		void on_Browse__released ();
	};
}
}
