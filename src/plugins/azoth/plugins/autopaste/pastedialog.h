/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <QPointer>
#include "pasteservicefactory.h"
#include "pasteservicebase.h"
#include "ui_pastedialog.h"

namespace LC::Azoth::Autopaste
{
	class PasteDialog : public QDialog
	{
		Q_OBJECT

		Ui::PasteDialog Ui_;
	public:
		enum Choice
		{
			Yes,
			No,
			Cancel
		};
	private:
		Choice Choice_ = Choice::Cancel;
	public:
		explicit PasteDialog (QWidget* = nullptr);

		Choice GetChoice () const;

		PasteServiceFactory::Creator_f GetCreator () const;
		QString GetCreatorName () const;
		void SetCreatorName (const QString&);

		Highlight GetHighlight () const;
		void SetHighlight (Highlight);
	private slots:
		void on_ButtonBox__clicked (QAbstractButton*);
	};
}
