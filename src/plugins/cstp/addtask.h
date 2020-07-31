/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_CSTP_ADDTASK_H
#define PLUGINS_CSTP_ADDTASK_H
#include <QDialog>
#include <QUrl>
#include "ui_addtask.h"

namespace LC
{
namespace CSTP
{
	class AddTask : public QDialog
	{
		Q_OBJECT

		Ui::AddTask Ui_;
		bool UserModifiedFilename_;
	public:
		AddTask (QWidget* = 0);
		AddTask (const QUrl&, const QString&, QWidget* = 0);
		virtual ~AddTask ();

		struct Task
		{
			QUrl URL_;
			QString LocalPath_;
			QString Filename_;
			QString Comment_;

			Task (const QUrl&,
					const QString&,
					const QString&,
					const QString&);
		};

		Task GetTask () const;
	public slots:
		virtual void accept ();
	private slots:
		void on_URL__textEdited (const QString&);
		void on_LocalPath__textChanged ();
		void on_Filename__textEdited ();
		void on_BrowseButton__released ();
	private:
		void CheckOK ();
	};
}
}

#endif
