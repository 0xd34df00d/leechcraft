/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "entitygeneratingpage.h"
#include "ui_operaimportselectpage.h"

namespace LC
{
struct Entity;

namespace NewLife
{
namespace Importers
{
	class OperaImportSelectPage : public EntityGeneratingPage
	{
		Q_OBJECT

		Ui::OperaImportSelectPage Ui_;
	public:
		OperaImportSelectPage (const ICoreProxy_ptr&, QWidget* = nullptr);

		int nextId () const override;
		void initializePage () override;
	private:
		QList<QVariant> GetHistory ();
		QList<QVariant> GetBookmarks ();
		QString GetImportOpmlFile ();
	private slots:
		void checkImportDataAvailable (int);
		void handleAccepted ();
	};
}
}
}
