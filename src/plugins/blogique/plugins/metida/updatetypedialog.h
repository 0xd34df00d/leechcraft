/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_updatetypedialog.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class UpdateTypeDialog : public QDialog
	{
		Q_OBJECT

		Ui::UpdateTypeDialog Ui_;

	public:
		enum LoadType
		{
			LoadLastEvents,
			LoadChangesEvents
		};

	private:
		LoadType LT_;

	public:
		UpdateTypeDialog (LoadType lt, QWidget *parent = 0);

		int GetCount () const;
		QDateTime GetDateTime () const;

	public slots:
		void accept ();
	};
}
}
}
