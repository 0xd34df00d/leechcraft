/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QMap>
#include "ui_changer.h"

namespace LC
{
namespace Poshuku
{
namespace Fua
{
	class Changer : public QDialog
	{
		Q_OBJECT

		Ui::Changer Ui_;

		const QList<QPair<QString, QString>> IDs_;
		const QMap<QString, QString> BackLookup_;
	public:
		Changer (const QList<QPair<QString, QString>>&,
				const QMap<QString, QString>&,
				const QString& = QString (),
				const QString& = QString (),
				QWidget* = 0);
		QString GetDomain () const;
		QString GetID () const;
	private slots:
		void on_Domain__textChanged ();
		void on_IDString__textChanged ();
		void on_Agent__currentIndexChanged (int);
	private:
		void SetEnabled ();
	};
}
}
}
