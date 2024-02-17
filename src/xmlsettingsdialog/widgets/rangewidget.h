/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>

class QSpinBox;

namespace LC
{
	class RangeWidget : public QWidget
	{
		Q_OBJECT

		QSpinBox *Lower_, *Higher_;
	public:
		RangeWidget (QWidget *parent = 0);
		void SetMinimum (int);
		void SetMaximum (int);
		void SetLower (int);
		void SetHigher (int);
		void SetRange (const QVariant&);
		QVariant GetRange () const;
	private slots:
		void lowerChanged (int);
		void upperChanged (int);
	signals:
		void changed ();
	};
}
