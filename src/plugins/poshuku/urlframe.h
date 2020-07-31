/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_URLFRAME_H
#define PLUGINS_POSHUKU_URLFRAME_H
#include <QFrame>
#include "ui_urlframe.h"

namespace LC
{
namespace Poshuku
{
	class URLFrame : public QFrame
	{
		Q_OBJECT

		Ui::URLFrame Ui_;
	public:
		URLFrame (QWidget* = 0);

		QLineEdit* GetEdit () const;
		ProgressLineEdit* GetEditAsProgressLine () const;

		void SetFavicon (const QIcon&);
		void AddWidget (QWidget*);
		void RemoveWidget (QWidget*);
	private slots:
		void on_URLEdit__returnPressed ();
	signals:
		void load (const QString&);
	};
}
}

#endif
