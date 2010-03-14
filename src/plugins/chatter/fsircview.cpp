/***************************************************************************
 *   Copyright (C) 2008 by Voker57   *
 *   voker57@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QtCore/QRegExp>
#include <QtCore/QHash>
#include <QtGui/QScrollBar>
#include <QtGui/QCompleter>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
//#include <QtCore/QDebug>

#include "fsircview.h"
#include "irc.h"
#include "fsettings.h"
#include "config.h"
#include "fsirc.h"
#include "connectiondialog.h"

FsIrcView::FsIrcView(QWidget * parent) : QWidget(parent)
{
	setupUi(this);
	fsChatView->setFocusProxy(cmdEdit);
	m_irc=new IrcLayer(0, FS_IRC_URI);
	m_linkRegexp = new QRegExp("([a-zA-Z\\+\\-\\.]+://(?:["+QRegExp::escape("-_.!~*'();/?:@&=+$,%#")+"]|\\w)+)");
	m_chanRegexp = new QRegExp("(\\s)(#(?:\\w|[\\.\\-\\[\\]\\(\\)@\"'`\\^\\$<>&~=#\\*])+)");
	initCompleters();
	initConnections();

	actionConnect = new QAction (tr ("Connect to room"), this);
	menuButton->addAction (actionConnect);
	connect (actionConnect, SIGNAL (triggered ()), this, SLOT (setConnection()));

	actionChangeNick = new QAction (tr ("Change nick"), this);
	menuButton->addAction (actionChangeNick);
	connect (actionChangeNick, SIGNAL (triggered ()), this, SLOT (changeNick ()));

	// TODO: make those customizable.. or not?
	m_msgColors["plain"]="#FFFFFF";
	m_msgColors["action"]="#59FF00";
	m_msgColors["event"]="#6075FF";
	m_msgColors["private"]="#FF1C1F";
	m_msgColors["notice"]="#5CE7FF";
	m_msgColors["error"]="#FF0000";
	m_msgColors["badevent"]="#FF1C1F";
	m_msgColors["raw"]="#C0C000";
	m_msgColors["link"]="#88FF76";
	m_msgColors["nicklink"]="#6FFF4B";
	m_msgColors["chanlink"]="#439A2D";
}

FsIrcView::~FsIrcView()
{
	delete m_irc;
	delete m_linkRegexp;
	delete m_chanRegexp;
}

void FsIrcView::fsEcho(QString message, QString style)
{
	// Escaping HTML in message
	message = Qt::escape(message);
	// Highlighting links
	message.replace(*m_linkRegexp,QString("<a href='\\1' style='color:%1'>\\1</a>").arg(m_msgColors["link"]));

	// Workaround to save the links
	message.replace(QRegExp("&amp;(?![a-z]+;)"),"&");
	// FIXME hack here!
	// Highlighting Private/Notice sources
	if(style==m_msgColors["notice"] || style==m_msgColors["private"])
		message.replace(
			QRegExp("^(\\S+): (\\S+):"),
			QString("\\1: <a href='irc://%1:%2/\\2' style='color:%3'>\\2</a>:").arg(m_irc->server(), m_irc->port(), m_msgColors["nicklink"])
		);
	// Highlighting each channel user's name
	foreach(QString user, m_irc->users())
	{
		message.replace(
			QRegExp(QString("([%2]|\\s|^|$)%1(?=[%2]|\\s|^|$)").arg(QRegExp::escape(user), QRegExp::escape(",\"';:.%!\\$#()"))),
			QString("\\1<a href='irc://%1:%2/%3' style='color:%4'>%3</a>").arg(m_irc->server(), m_irc->port(), user, m_msgColors["nicklink"])
		);
	}
	// Highlighting channel references
	message.replace(*m_chanRegexp,QString("\\1<a href='irc://%1:%2/\\2' style='color:%3'>\\2</a>").arg(m_irc->server(), m_irc->port(), m_msgColors["chanlink"]));
	// Phew.
	fsOut(QString("<span style='color:%1'>%2</span> <br />").arg(style,message));
}

void FsIrcView::initConnections()
{
	connect(cmdEdit, SIGNAL(returnPressed()), this, SLOT(sayHere()));
	connect(fsChatView, SIGNAL(anchorClicked(QUrl)), this, SIGNAL(anchorClicked(QUrl)));
}

void FsIrcView::initCompleters()
{
	fSettings settings;
	settings.beginGroup("history");
	// Encodings
	if (m_actionCompleters[ACT_ENCODING])
		delete m_actionCompleters[ACT_ENCODING];
	QStringList eList;
	QList<QByteArray> bList=QTextCodec::availableCodecs();
	QList<QByteArray>::const_iterator iter;
	for (iter = bList.constBegin(); iter != bList.constEnd(); ++iter)
		eList.append(*iter);
	m_actionCompleters[ACT_ENCODING]= new QCompleter(eList, this);
	m_actionCompleters[ACT_ENCODING]->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
	m_actionCompleters[ACT_ENCODING]->setCaseSensitivity(Qt::CaseInsensitive);
	// Recent irc URIs
	if (m_actionCompleters[ACT_URI])
		delete m_actionCompleters[ACT_URI];
	m_actionCompleters[ACT_URI]= new QCompleter(settings.toStringList("irc-uris","uri"), this);
	m_actionCompleters[ACT_URI]->setCaseSensitivity(Qt::CaseInsensitive);
	// » nicks
	if (m_actionCompleters[ACT_NICK])
		delete m_actionCompleters[ACT_NICK];
	m_actionCompleters[ACT_NICK]= new QCompleter(settings.toStringList("irc-nicks","nick"), this);
	m_actionCompleters[ACT_NICK]->setCaseSensitivity(Qt::CaseInsensitive);
	m_actionCompleters[ACT_NICK]->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
	settings.endGroup();
}

void FsIrcView::gotChannelMsg(QHash<QString, QString> data)
{
	// Channel message
	if (data["text"].contains(
				QRegExp(
					QString("\\b%1\\b").arg(QRegExp::escape(m_irc->nick()))
				)
			)
		)
	{
		fsEcho(data["nick"]+": "+data["text"],m_msgColors["private"]);
		emit gotHlite();
	}
	else
		fsEcho(data["nick"]+": "+data["text"]);
}

void FsIrcView::gotPrivMsg(QHash<QString, QString> data)
{
	// Private message
	qDebug() << "Receiving PrivMsg" << ircUri() << hasFocus() << isVisible() << isHidden();
	if(fsirc::findTab(ircUri().replace(QRegExp("/[^/]+$"),"/"+data["nick"]))<0 && fsirc::findTab(ircUri().replace(QRegExp("/[^/]+$"),"/"+data["target"]))<0 && !isHidden())
		fsEcho(tr("Private: ")+data["nick"]+": "+data["text"], m_msgColors["private"]);
}

void FsIrcView::gotNotice(QHash<QString, QString> data)
{
	fsEcho(tr("Notice: ")+data["nick"]+": "+data["text"], m_msgColors["notice"]);
}

void FsIrcView::gotAction(QHash<QString, QString> data)
{
	fsEcho("* "+data["nick"]+" "+data["text"], m_msgColors["action"]);
}

void FsIrcView::sayHere()
{
	QString msg=cmdEdit->text();
	if (!msg.isEmpty())
	{
		cmdEdit->clear();
		if (!msg.startsWith(FS_COM_CHAR)) // a shout
		{
			m_irc->say(msg);
			fsEcho(m_irc->nick()+": "+msg);
		}
		else // a command
		{
			msg.remove(0,1);
			QRegExp command("^([a-zA-z]+) (.+)$");
			if (command.exactMatch(msg))
			{
				fsExec(command.cap(1),command.cap(2));
			}
			else fsExec(msg);
		}
	}
}

void FsIrcView::gotInfo(QString message)
{
	// Various info
	fsEcho(message,m_msgColors["notice"]);
}

void FsIrcView::gotError(QString message)
{
	// Bad news from m_irc layer
	fsEcho(message,m_msgColors["error"]);
}

void FsIrcView::gotNames(QStringList data)
{
	QString output = tr("Names for %1: %2").arg(m_irc->target(),data.join(", "));
	fsEcho(output, m_msgColors["event"]);
}

void FsIrcView::gotTopic(QStringList data)
{
	fsEcho(tr("Topic for ") + data[1] + ": " + data[2], m_msgColors["event"]);
}

void FsIrcView::gotNick(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" is now known as ") + data["target"], m_msgColors["event"]);
	// Record nick to history
	if (m_irc->nick()==data["target"])
	{
		qDebug() << "nick logged";
		nickToHistory(data["target"]);
	}
}

void FsIrcView::nickToHistory(QString nick)
{
	fSettings settings;
	settings.setValue("nickname", nick);
	settings.setValue("servers/"+m_irc->server()+"/nickname", nick);
	settings.beginGroup("history");
	if (settings.appendValue(nick,"irc-nicks","nick"))
		initCompleters();
}

void FsIrcView::gotJoin(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has joined ") + data["target"], m_msgColors["event"]);
	if (data["nick"]==m_irc->nick())
	{
		fSettings settings;
		settings.beginGroup("encodings");
		if (settings.contains(m_irc->getIrcUri()))
			m_irc->setEncoding(settings.value(m_irc->getIrcUri()).toString());
		settings.endGroup();
	}
}

void FsIrcView::gotPart(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has left ") + data["target"] + ": "+data["text"], m_msgColors["event"]);
}

void FsIrcView::gotQuit(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has quit IRC: ") + data["text"], m_msgColors["event"]);
}

void FsIrcView::gotKick(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has kicked ") + data["subject"] + tr(" out of ") + data["target"] + ": " + data["text"], m_msgColors["badevent"]);
}

void FsIrcView::gotMode(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has set mode ") + data["text"] + " " + data["subject"] + tr(" on ") + data["target"], m_msgColors["event"]);
}

void FsIrcView::fsExec(QString cmd, QString arg)
{
	fSettings settings;
	cmd.toLower();
	QStringList args = arg.split(QRegExp("\\s+"));
	if (cmd=="encoding")
	{
		if (m_irc->setEncoding(arg)==1)
		{
			// Good one, gotta save this
			settings.beginGroup("encodings");
			settings.setValue(m_irc->getIrcUri(), arg);
			settings.endGroup();
		}
		return;
	}
	if (cmd=="me")
	{
		QString sarg = arg;
		arg.prepend("ACTION ");
		arg.prepend('\x01');
		arg.append('\x01');
		m_irc->ircMsg(arg, m_irc->channel());
		fsEcho("* "+m_irc->nick()+" "+sarg, m_msgColors["action"]);
		return;
	}
	if (cmd=="join" || cmd=="j")
	{
		m_irc->ircJoin(arg);
		return;
	}
	if (cmd=="nick")
	{
		m_irc->setNickChanged(true);
		m_irc->ircSetNick(arg);
		if (!m_irc->connected())
			nickToHistory(arg);
		return;
	}
	if (cmd=="mode")
	{
		m_irc->ircMode(arg);
		return;
	}
	if ((cmd=="nickserv")||(cmd=="ns"))
	{
		m_irc->ircNs(arg);
		return;
	}
	if ((cmd=="chanserv")||(cmd=="cs"))
	{
		m_irc->ircCs(arg);
		return;
	}
	if (cmd=="quit")
	{
		m_irc->ircQuit(arg);
		return;
	}
	if ((cmd=="memoserv")||(cmd=="ms"))
	{
		m_irc->ircMs(arg);
		return;
	}
	// More-than-one parameters
	if(cmd=="msg")
	{
		m_irc->ircMsg(args[1],args[0]);
		return;
	}
	else
	if(cmd=="kick")
	{
		m_irc->ircKick(args[0], args[1]);
		return;
	}
	if(cmd=="topic")
	{
		m_irc->setTopic(arg);
		return;
	}
	//fsEcho(tr("Unknown command: ") + cmd, "red");
	{
		m_irc->ircThrow(cmd+" "+arg);
		fsEcho(tr("RAW -> ")+cmd+" "+arg,m_msgColors["raw"]);
	}
}

void FsIrcView::fsQuit()
{
	m_irc->ircQuit(FS_QUIT_MSG);
}

void FsIrcView::openIrc(QString uri)
{
	if (!IrcLayer::isIrcUri(uri)) return;
	if(m_irc) delete m_irc;
	m_irc=new IrcLayer(this, uri);
	m_irc->ircConnect();
	emit uriChanged();
	connect(m_irc, SIGNAL(gotChannelMsg(QHash<QString, QString>)), this, SLOT(gotChannelMsg(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotPrivMsg(QHash<QString, QString>)), this, SLOT(gotPrivMsg(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotNotice(QHash<QString, QString>)), this, SLOT(gotNotice(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotInfo(QString)), this, SLOT(gotInfo(QString)));
	connect(m_irc, SIGNAL(gotError(QString)), this, SLOT(gotError(QString)));
	connect(m_irc, SIGNAL(gotAction(QHash<QString, QString>)), this, SLOT(gotAction(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotPrivAction(QHash<QString, QString>)), this, SLOT(gotPrivAction(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotNames(QStringList)), this, SLOT(gotNames(QStringList)));
	connect(m_irc, SIGNAL(gotTopic(QStringList)), this, SLOT(gotTopic(QStringList)));
	connect(m_irc, SIGNAL(gotTopic(QHash<QString, QString>)), this, SLOT(gotTopic(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotNick(QHash<QString, QString>)), this, SLOT(gotNick(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotJoin(QHash<QString, QString>)), this, SLOT(gotJoin(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotPart(QHash<QString, QString>)), this, SLOT(gotPart(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotQuit(QHash<QString, QString>)), this, SLOT(gotQuit(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotKick(QHash<QString, QString>)), this, SLOT(gotKick(QHash<QString, QString>)));
	connect(m_irc, SIGNAL(gotMode(QHash<QString, QString>)), this, SLOT(gotMode(QHash<QString, QString>)));
	// retranslation of events
	connect(m_irc, SIGNAL(gotPrivMsg(QHash<QString, QString>)), this, SIGNAL(gotHlite()));
	connect(m_irc, SIGNAL(gotChannelMsg(QHash<QString, QString>)), this, SIGNAL(gotSomeMsg()));
	connect(m_irc, SIGNAL(gotNotice(QHash<QString, QString>)), this, SIGNAL(gotSomeMsg()));
	connect(m_irc, SIGNAL(gotError(QString)), this, SIGNAL(gotHlite()));
	connect(m_irc, SIGNAL(gotInfo(QString)), this, SIGNAL(gotSomeMsg()));
	connect(m_irc, SIGNAL(gotAction(QHash<QString, QString>)), this, SIGNAL(gotSomeMsg()));
	connect(m_irc, SIGNAL(gotKick(QHash<QString, QString>)), this, SIGNAL(gotSomeMsg()));
}

void FsIrcView::fsOut(QString message)
{
	QScrollBar *scrollbar = fsChatView->verticalScrollBar();
	int offset = -1;
	if (scrollbar->sliderPosition() < scrollbar->maximum())
		offset = scrollbar->sliderPosition();
	fsChatView->moveCursor(QTextCursor::End);
	int pos = 0;
	while ((pos = m_linkRegexp->indexIn(message,pos)) != -1)
	{
		emit gotLink(m_linkRegexp->cap(1));
		pos+=m_linkRegexp->matchedLength();
	}
	fsChatView->insertHtml(message);
	if (offset < 0) offset = scrollbar->maximum();
	scrollbar->setValue(offset);
}

QString FsIrcView::ircUri() const
{
	if(m_irc->connected()) return m_irc->getIrcUri(); else return QString();
}

void FsIrcView::proposeUri(QString uri)
{
}

void FsIrcView::clearView()
{
	fsChatView->clear();
}

void FsIrcView::gotPrivAction(QHash< QString, QString > data)
{
		// Private message
	qDebug() << "Receiving PrivMsg" << ircUri() << hasFocus() << isVisible() << isHidden();
	if(fsirc::findTab(ircUri().replace(QRegExp("/[^/]+$"),"/"+data["nick"]))<0 && fsirc::findTab(ircUri().replace(QRegExp("/[^/]+$"),"/"+data["target"]))<0 && !isHidden())
		fsEcho(tr("Private: * %1 %2").arg(data["nick"],data["text"]), m_msgColors["private"]);
}

void FsIrcView::gotTopic(QHash< QString, QString > data)
{
	fsEcho(tr("%1 sets topic to %2").arg(data["nick"],data["text"]), m_msgColors["event"]);
}

void FsIrcView::changeNick()
{
	fSettings settings;

	QStringList items;
	items << settings.value("nickname").toString();

	bool ok = false;
	const QString& nick = QInputDialog::getItem (this, tr ("IRC URI"), tr ("IRC URI"),
								 items, -1, true, &ok);

	if (!ok)
		return;

	if (nick.isEmpty ()) {
		QMessageBox::critical (this, "", tr ("Nick is empty"));
		return;
	}

	fsExec ("nick", nick);
}

void FsIrcView::setConnection()
{
	ConnectionDialog d (this);
	if (d.exec ()) {
		fsExec("nick", d.nick());
		fsExec("encoding", d.encoding());
		openIrc("irc://" + d.server() + "/" + d.room());
	}
}
