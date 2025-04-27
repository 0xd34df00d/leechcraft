/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_importopmldialog.h"

namespace LC::Aggregator
{
	class ImportOPMLDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::ImportOPMLDialog)

		Ui::ImportOPMLDialog Ui_;

		QHash<QLatin1String, QLabel*> Fields_;
	public:
		explicit ImportOPMLDialog (const QString& = {}, QWidget* = nullptr);

		QString GetFilename () const;
		QString GetTags () const;
		QSet<QString> GetSelectedUrls () const;
	private:
		void HandleFilePathEdited (const QString&);
		void BrowseFile ();

		void HandleFile (const QString&);
		void Reset ();
	};
}
