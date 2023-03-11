/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "termcontextmenubuilder.h"
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QGuiApplication>
#include <QMenu>
#include <qtermwidget.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>

namespace LC::Eleeminator
{
	namespace
	{
		struct ContextMenuRunner
		{
			QTermWidget& Term_;
			QPoint Point_;

			QMenu Menu_ {};

			QString Text_ = Term_.selectedText ();

			void operator () ()
			{
				AddUrlActions ();
				AddLocalFileActions ();
				AddTextActions ();

				Menu_.exec (Term_.mapToGlobal (Point_));
			}

			void AddUrlActions ()
			{
				const auto hotspot = Term_.getHotSpotAt (Point_);
				if (!hotspot)
					return;

				if (hotspot->type () != Filter::HotSpot::Link)
					return;

				const auto urlHotSpot = dynamic_cast<const Konsole::UrlFilter::HotSpot*> (hotspot);
				const auto& cap = urlHotSpot->capturedTexts ().value (0);
				if (cap.isEmpty ())
					return;

				const auto itm = GetProxyHolder ()->GetIconThemeManager ();
				Menu_.addAction (itm->GetIcon ("document-open-remote"_qs),
						tr ("Open URL"),
						&Term_,
						[cap]
						{
							const auto& url = QUrl::fromEncoded (cap.toUtf8 ());
							const auto& entity = Util::MakeEntity (url, {}, TaskParameter::FromUserInitiated);
							GetProxyHolder ()->GetEntityManager ()->HandleEntity (entity);
						});
				Menu_.addAction (tr ("Copy URL"),
						&Term_,
						[cap] { QGuiApplication::clipboard ()->setText (cap, QClipboard::Clipboard); });
				Menu_.addSeparator ();
			}

			void AddLocalFileActions ()
			{
				if (Text_.isEmpty ())
					return;

				const QDir workingDir { Term_.workingDirectory () };
				if (!workingDir.exists (Text_))
					return;

				const auto& localUrl = QUrl::fromLocalFile (workingDir.filePath (Text_));
				Menu_.addAction (tr ("Open file"),
						&Term_,
						[localUrl]
						{
							const auto& entity = Util::MakeEntity (localUrl, {}, OnlyHandle | FromUserInitiated);
							GetProxyHolder ()->GetEntityManager ()->HandleEntity (entity);
						});
				Menu_.addAction (tr ("Open file externally"),
						&Term_,
						[localUrl] { QDesktopServices::openUrl (localUrl); });
				Menu_.addSeparator ();

				new Util::StdDataFilterMenuCreator { localUrl, GetProxyHolder ()->GetEntityManager (), &Menu_ };
			}

			void AddTextActions ()
			{
				const auto itm = GetProxyHolder ()->GetIconThemeManager ();

				const auto copyAct = Menu_.addAction (itm->GetIcon ("edit-copy"_qs),
						tr ("Copy selected text"),
						&Term_,
						&QTermWidget::copyClipboard);
				copyAct->setEnabled (!Term_.selectedText ().isEmpty ());

				const auto pasteAct = Menu_.addAction (itm->GetIcon ("edit-paste"_qs),
						tr ("Paste from clipboard"),
						&Term_,
						&QTermWidget::pasteClipboard);
				pasteAct->setEnabled (!QGuiApplication::clipboard ()->text (QClipboard::Clipboard).isEmpty ());

				new Util::StdDataFilterMenuCreator { Text_, GetProxyHolder ()->GetEntityManager (), &Menu_ };
			}

			Q_DECLARE_TR_FUNCTIONS (LC::Eleeminator::TermTabContextMenu)
		};
	}

	void SetupContextMenu (QTermWidget& term)
	{
		term.setContextMenuPolicy (Qt::CustomContextMenu);
		QObject::connect (&term,
				&QTermWidget::customContextMenuRequested,
				[&term] (const QPoint& point) { ContextMenuRunner { term, point } (); });
	}
}
