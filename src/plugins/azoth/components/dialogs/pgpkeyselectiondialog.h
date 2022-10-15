/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PGPKEYSELECTIONDIALOG_H
#define PLUGINS_AZOTH_PGPKEYSELECTIONDIALOG_H
#include <QDialog>
#include <QtCrypto>
#include "ui_pgpkeyselectiondialog.h"

namespace LC
{
namespace Azoth
{
	class PGPKeySelectionDialog : public QDialog
	{
		Q_OBJECT

		Ui::PGPKeySelectionDialog Ui_;
		QList<QCA::PGPKey> Keys_;
	public:
		enum Type
		{
			TPublic,
			TPrivate
		};

		PGPKeySelectionDialog (const QString&, Type, const QCA::PGPKey&, QWidget* = 0);

		QCA::PGPKey GetSelectedKey () const;
	};
}
}

#endif
