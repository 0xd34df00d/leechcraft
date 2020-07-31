/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include "ui_firefoxprofileselectpage.h"
#include "entitygeneratingpage.h"

class QSqlDatabase;
class QSqlQuery;

namespace LC
{
struct Entity;

namespace NewLife
{
namespace Importers
{
	class FirefoxProfileSelectPage : public EntityGeneratingPage
	{
		Q_OBJECT

		Ui::FirefoxProfileSelectPage Ui_;
		std::shared_ptr<QSqlDatabase> DB_;
	public:
		FirefoxProfileSelectPage (const ICoreProxy_ptr&, QWidget* = nullptr);
		virtual ~FirefoxProfileSelectPage ();

		virtual int nextId () const;
		virtual void initializePage ();
		QString GetProfileDirectory (const QString&) const;
		void GetProfileList (const QString&);
		QList<QVariant> GetHistory ();
		QList<QVariant> GetBookmarks ();
		QString GetImportOpmlFile ();
		QSqlQuery GetQuery (const QString&);
		bool IsFirefoxRunning ();
	private slots:
		void checkImportDataAvailable (int);
		void handleAccepted ();
	};
}
}
}
