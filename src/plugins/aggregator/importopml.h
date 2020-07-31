/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_importopml.h"

namespace LC
{
namespace Aggregator
{
	class ImportOPML : public QDialog
	{
		Q_OBJECT

		Ui::ImportOPML Ui_;
	public:
		ImportOPML (const QString& = QString (), QWidget* = 0);

		QString GetFilename () const;
		QString GetTags () const;
		QSet<QString> GetSelectedUrls () const;
	private slots:
		void on_File__textEdited (const QString&);
		void on_Browse__released ();
	private:
		void HandleFile (const QString&);
		void Reset ();
	};
}
}
