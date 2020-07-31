/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "sourceviewer.h"
#include <QGuiApplication>
#include <QScreen>
#include <QShortcut>
#include <util/gui/findnotification.h>
#include "htmlhighlighter.h"
#include "core.h"

namespace LC
{
namespace Poshuku
{
	namespace
	{
		class SourceFinder : public Util::FindNotification
		{
			QTextEdit * const Edit_;
		public:
			SourceFinder (QTextEdit *edit, const ICoreProxy_ptr& proxy)
			: FindNotification { proxy, edit }
			, Edit_ { edit }
			{
			}
		protected:
			void handleNext (const QString& text, FindFlags flags)
			{
				QTextDocument::FindFlags tdFlags;
				if (flags & FindCaseSensitively)
					tdFlags |= QTextDocument::FindCaseSensitively;
				if (flags & FindBackwards)
					tdFlags |= QTextDocument::FindBackward;
				SetSuccessful (Edit_->find (text, tdFlags));
			}
		};
	}

	SourceViewer::SourceViewer (QWidget *parent)
	: QMainWindow { parent }
	{
		Ui_.setupUi (this);

		auto frect = frameGeometry ();
		frect.moveCenter (QGuiApplication::primaryScreen ()->availableGeometry ().center ());
		move (frect.topLeft ());
		new HtmlHighlighter { Ui_.HtmlEdit_ };

		const auto finder = new SourceFinder
		{
			Ui_.HtmlEdit_,
			Core::Instance ().GetProxy ()
		};
		finder->hide ();
	}

	void SourceViewer::SetHtml (const QString& html)
	{
		Ui_.HtmlEdit_->setPlainText (html);
	}
}
}
