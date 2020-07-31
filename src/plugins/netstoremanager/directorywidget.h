/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_directorywidget.h"

namespace LC
{
namespace NetStoreManager
{
	class AccountsManager;

	class DirectoryWidget : public QWidget
	{
		Q_OBJECT

		Ui::SelectDirectoryWidget Ui_;
		QString Path_;
		QByteArray AccountID_;
		AccountsManager *AM_;
	public:
		enum class Type
		{
			Local,
			Remote
		};
	private:
		Type Type_;

	public:

		DirectoryWidget (Type t, const QByteArray& accId, AccountsManager *am = 0,
				QWidget *parent = 0);

		void SetPath (const QString& path, bool byHand = false);
		QString GetPath () const;

	private slots:
		void on_OpenDir__released ();
		void on_DirPath__editingFinished ();

	signals:
		void finished (QWidget *widget);
	};
}
}
