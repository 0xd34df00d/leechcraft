/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QStringList>
#include "ui_flashonclickwhitelist.h"

class QStandardItemModel;

namespace LC
{
namespace Poshuku
{
namespace FOC
{
	class FlashOnClickWhitelist : public QWidget
	{
		Q_OBJECT

		Ui::FlashOnClickWhitelist Ui_;
		QStandardItemModel *Model_;
	public:
		FlashOnClickWhitelist (QWidget* = nullptr);

		QStringList GetWhitelist () const;
		bool Matches (const QString&) const;
		void Add (const QString&);
	private slots:
		void on_Add__released ();
		void on_Modify__released ();
		void on_Remove__released ();

		void accept ();
		void reject ();
	private:
		void AddImpl (QString = {}, const QModelIndex& = {});

		void ReadSettings ();
		void SaveSettings ();
	};
}
}
}
