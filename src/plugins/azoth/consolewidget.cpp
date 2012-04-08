/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "consolewidget.h"
#include <QDomDocument>
#include <QtDebug>
#include "interfaces/azoth/iaccount.h"

namespace LeechCraft
{
namespace Azoth
{
	ConsoleWidget::ConsoleWidget (QObject *obj, QWidget *parent)
	: QWidget (parent)
	, ParentMultiTabs_ (0)
	, AsAccount_ (qobject_cast<IAccount*> (obj))
	, AsConsole_ (qobject_cast<IHaveConsole*> (obj))
	, Format_ (AsConsole_->GetPacketFormat ())
	{
		if (!AsAccount_ || !AsConsole_)
			qWarning () << Q_FUNC_INFO
					<< "oh shi~,"
					<< obj
					<< "doesn't implement IAccount or IHaveConsole,"
					<< "we definitely gonna segfault soon";

		Ui_.setupUi (this);

		TabClassInfo temp =
		{
			"ConsoleTab",
			tr ("IM console"),
			tr ("Protocol console, for example, XML console for a XMPP "
				"client protocol"),
			QIcon (":/plugins/azoth/resources/images/sdtab.svg"),
			0,
			TFEmpty
		};
		TabClass_ = temp;

		connect (obj,
				SIGNAL (gotConsolePacket (QByteArray, int, QString)),
				this,
				SLOT (handleConsolePacket (QByteArray, int, QString)));

		AsConsole_->SetConsoleEnabled (true);
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
		AsConsole_->SetConsoleEnabled (false);
		emit removeTab (this);
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

	void ConsoleWidget::handleConsolePacket (QByteArray data, int direction, const QString& entryId)
	{
		const QString& filter = Ui_.EntryIDFilter_->text ();
		if (!filter.isEmpty () && !entryId.contains (filter, Qt::CaseInsensitive))
			return;

		const QString& color = direction == IHaveConsole::PDOut ?
				"#56ED56" :			// rather green
				"#ED55ED";			// violet or something

		QString html = QString::fromUtf8 ("—————— [%1] ——————")
				.arg (QTime::currentTime ().toString ("HH:mm:ss.zzz"));
		html += "<br /><font color=\"" + color + "\">";
		switch (Format_)
		{
		case IHaveConsole::PFBinary:
			html += "(base64) ";
			html += data.toBase64 ();
			break;
		case IHaveConsole::PFXML:
		{
			QDomDocument doc;
			if (doc.setContent (data))
				data = doc.toByteArray (2);
		}
		case IHaveConsole::PFPlainText:
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

	void ConsoleWidget::on_ClearButton__released ()
	{
		Ui_.PacketsBrowser_->clear ();
	}

	void ConsoleWidget::on_EnabledBox__toggled (bool enable)
	{
		AsConsole_->SetConsoleEnabled (enable);
	}
}
}
