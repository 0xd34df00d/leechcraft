/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "consolewidget.h"
#include <QDomDocument>
#include <QtDebug>
#include "interfaces/azoth/iaccount.h"

namespace LC
{
namespace Azoth
{
	ConsoleWidget::ConsoleWidget (QObject *obj, QWidget *parent)
	: QWidget (parent)
	, AsObject_ (obj)
	, AsAccount_ (qobject_cast<IAccount*> (obj))
	, AsConsole_ (qobject_cast<IHaveConsole*> (obj))
	, Format_ (AsConsole_->GetPacketFormat ())
	{
		Ui_.setupUi (this);

		TabClassInfo temp =
		{
			"ConsoleTab",
			tr ("IM console"),
			tr ("Protocol console, for example, XML console for a XMPP "
				"client protocol"),
			QIcon ("lcicons:/plugins/azoth/resources/images/sdtab.svg"),
			0,
			TFEmpty
		};
		TabClass_ = temp;

		connect (obj,
				SIGNAL (gotConsolePacket (QByteArray, IHaveConsole::PacketDirection, QString)),
				this,
				SLOT (handleConsolePacket (QByteArray, IHaveConsole::PacketDirection, QString)));

		AsConsole_->SetConsoleEnabled (true);

		connect (Ui_.ClearButton_,
				&QPushButton::released,
				Ui_.PacketsBrowser_,
				&QTextBrowser::clear);
		connect (Ui_.EnabledBox_,
				&QCheckBox::toggled,
				this,
				[this] (bool enable) { AsConsole_->SetConsoleEnabled (enable); });
	}

	TabClassInfo ConsoleWidget::GetTabClassInfo () const
	{
		return TabClass_;
	}

	QObject* ConsoleWidget::ParentMultiTabs ()
	{
		return ParentMultiTabs_;
	}

	void ConsoleWidget::Remove ()
	{
		if (AsObject_)
			AsConsole_->SetConsoleEnabled (false);
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* ConsoleWidget::GetToolBar () const
	{
		return 0;
	}

	void ConsoleWidget::SetParentMultiTabs (QObject *obj)
	{
		ParentMultiTabs_ = obj;
	}

	QString ConsoleWidget::GetTitle () const
	{
		return tr ("%1: console").arg (AsAccount_->GetAccountName ());
	}

	void ConsoleWidget::handleConsolePacket (QByteArray data,
			IHaveConsole::PacketDirection direction, const QString& entryId)
	{
		const QString& filter = Ui_.EntryIDFilter_->text ();
		if (!filter.isEmpty () && !entryId.contains (filter, Qt::CaseInsensitive))
			return;

		const QString& color = direction == IHaveConsole::PacketDirection::Out ?
				"#56ED56" :			// rather green
				"#ED55ED";			// violet or something

		QString html = (direction == IHaveConsole::PacketDirection::Out ?
					QString::fromUtf8 ("→→→→→→ [%1] →→→→→→") :
					QString::fromUtf8 ("←←←←←← [%1] ←←←←←←"))
				.arg (QTime::currentTime ().toString ("HH:mm:ss.zzz"));
		html += "<br /><font color=\"" + color + "\">";
		switch (Format_)
		{
		case IHaveConsole::PacketFormat::Binary:
			html += "(base64) ";
			html += data.toBase64 ();
			break;
		case IHaveConsole::PacketFormat::XML:
		{
			QDomDocument doc;
			data.prepend ("<root>");
			data.append ("</root>");
			if (doc.setContent (data))
				data = doc.toByteArray (2);

			const auto markerSize = QString ("<root>\n").size ();
			data.chop (markerSize + 1);
			data = data.mid (markerSize);
			[[fallthrough]];
		}
		case IHaveConsole::PacketFormat::PlainText:
			html += QString::fromUtf8 (data
					.replace ('<', "&lt;")
					.replace ('\n', "<br/>")
					.replace (' ', "&nbsp;")
					.constData ());
			break;
		}
		html += "</font><br />";

		Ui_.PacketsBrowser_->append (html);
	}
}
}
