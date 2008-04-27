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
#include "fsirc.h"
#include <QDialog>
#include <QDebug>
#include <QScrollBar>
#include <QRegExp>
#include <QStringList>
#include "config.h"

fsirc::fsirc(QWidget *parent) : QDialog(parent)
{
	fSettings settings;
	setupUi(this);
	irc=new ircLayer;
	mArg.setPattern("(\\S+)(?: (\\S+))+");
	initCompleters();
	initConnections();
	pickAction();
}

void fsirc::fsEcho(QString message, QString style)
{
	QScrollBar *scrollbar = fsChatView->verticalScrollBar();
	int offset = -1;
	if(scrollbar->sliderPosition() < scrollbar->maximum())
		offset = scrollbar->sliderPosition();
	fsChatView->moveCursor(QTextCursor::End);
	fsChatView->insertHtml("<div style='color:"+style+"'> "+message+"</div> <br />");
	if(offset < 0) offset = scrollbar->maximum();
	scrollbar->setValue(offset);
//	qDebug() << fsChatView->toHtml();
}

void fsirc::initConnections()
{
	connect(fsActionCombo,SIGNAL(activated(int)),fsActionEdit,SLOT(setFocus()));
	connect(fsActionCombo,SIGNAL(activated(int)),this,SLOT(pickAction()));
	connect(fsActionCombo,SIGNAL(activated(int)),fsActionEdit,SLOT(selectAll()));
	connect(fsActionEdit, SIGNAL(returnPressed()), this, SLOT(takeAction()));
	connect(fsCmdEdit, SIGNAL(returnPressed()), this, SLOT(sayHere()));
	connect(irc, SIGNAL(gotMsg(QHash<QString, QString>)), this, SLOT(gotMsg(QHash<QString, QString>)));
	connect(irc, SIGNAL(gotNotice(QHash<QString, QString>)), this, SLOT(gotNotice(QHash<QString, QString>)));
	connect(irc, SIGNAL(gotInfo(QString)), this, SLOT(gotInfo(QString)));
	connect(irc, SIGNAL(gotAction(QHash<QString, QString>)), this, SLOT(gotAction(QHash<QString, QString>)));
	connect(irc, SIGNAL(gotNames(QStringList)), this, SLOT(gotNames(QStringList)));
	connect(irc, SIGNAL(gotTopic(QStringList)), this, SLOT(gotTopic(QStringList)));
	connect(irc, SIGNAL(gotNick(QHash<QString, QString>)), this, SLOT(gotNick(QHash<QString, QString>)));
	connect(irc, SIGNAL(gotJoin(QHash<QString, QString>)), this, SLOT(gotJoin(QHash<QString, QString>)));
	connect(irc, SIGNAL(gotPart(QHash<QString, QString>)), this, SLOT(gotPart(QHash<QString, QString>)));
	connect(irc, SIGNAL(gotQuit(QHash<QString, QString>)), this, SLOT(gotQuit(QHash<QString, QString>)));
	connect(irc, SIGNAL(gotKick(QHash<QString, QString>)), this, SLOT(gotKick(QHash<QString, QString>)));
	connect(irc, SIGNAL(gotMode(QHash<QString, QString>)), this, SLOT(gotMode(QHash<QString, QString>)));
}

void fsirc::initCompleters()
{
	fSettings settings;
	settings.beginGroup("history");
	// Encodings
	if(actionCompleters[ACT_ENCODING])
	{
		QStringList eList;
		QList<QByteArray> bList=QTextCodec::availableCodecs();
		QList<QByteArray>::const_iterator iter;
		for(iter = bList.constBegin(); iter != bList.constEnd(); ++iter)
			eList.append(*iter);
		actionCompleters[ACT_ENCODING]= new QCompleter(eList, this);
		actionCompleters[ACT_ENCODING]->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
		actionCompleters[ACT_ENCODING]->setCaseSensitivity(Qt::CaseInsensitive);
	}
	// Recent IRC URIs
	if(actionCompleters[ACT_URI])
		 delete actionCompleters[ACT_URI];
	actionCompleters[ACT_URI]= new QCompleter(settings.toStringList("irc-uris","uri"), this);
	actionCompleters[ACT_URI]->setCaseSensitivity(Qt::CaseInsensitive);
	// » nicks
	if(actionCompleters[ACT_NICK])
		delete actionCompleters[ACT_NICK];
	actionCompleters[ACT_NICK]= new QCompleter(settings.toStringList("irc-nicks","nick"), this);
	actionCompleters[ACT_NICK]->setCaseSensitivity(Qt::CaseInsensitive);
	actionCompleters[ACT_NICK]->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
	settings.endGroup();
}

void fsirc::pickAction()
{
	QString pick;
	switch(fsActionCombo->currentIndex())
	{
		case ACT_URI: //IRC URI, paste current one
		pick=irc->getIrcUri();
		break;
		case ACT_NICK: // nick, current one
		pick=irc->nick();
		break;
		case ACT_CHANNEL: // channel, »
		pick=irc->channel();
		break;
		case ACT_ENCODING: // encoding, »
		pick=irc->encoding();
		break;
		case ACT_QUIT: // quit. ehm..
		pick="";
		break;
	}
	fsActionEdit->setCompleter(actionCompleters.value(fsActionCombo->currentIndex()));
	fsActionEdit->setText(pick);
}

void fsirc::takeAction()
{
	fSettings settings;
	settings.beginGroup("history");
	QHash<QString, QString> ircUri;
	switch(fsActionCombo->currentIndex())
	{
		case ACT_URI: // Using IRC URI
		ircUri=irc->chewIrcUri(fsActionEdit->text());
		if(ircUri.isEmpty())
			fsEcho("Incorrect irc:// URI");
		else
		{
			// Use server nickname, otherwise global one
			settings.endGroup();
			QVariant valNick=settings.value("servers/"+ircUri["server"]+"/Nickname");
			if(valNick.isValid())
				irc->ircSetNick(valNick.toString());
			else
			{
				valNick=settings.value("Nickname");
				if(valNick.isValid())
					irc->ircSetNick(valNick.toString());
			}
			settings.beginGroup("history");
			// Use URI
			irc->ircUseUri(fsActionEdit->text());
			fsActionEdit->setText(irc->getIrcUri());
			// Add URI to recent list, if it's not already there
			if (settings.appendValue(fsActionEdit->text(),"irc-uris","uri"))
				// Update completers
				initCompleters();
		}
		break;
		case ACT_NICK: // Changing nick
			irc->ircSetNick(fsActionEdit->text());
		break;
		case ACT_CHANNEL: // Joining (changing) channel
			irc->ircJoin(fsActionEdit->text());
		break;
		case ACT_ENCODING: // Changing encoding
			irc->setEncoding(fsActionEdit->text());
		break;
		case ACT_QUIT: // Goodbye cruel world
			irc->ircQuit(fsActionEdit->text());
		break;
	}
	settings.endGroup();
}

void fsirc::gotMsg(QHash<QString, QString> data)
{
	// Mostly channel messages
	if(data["target"]==irc->channel())
		fsEcho(data["nick"]+": "+data["text"]); else
	fsEcho(tr("Private> ")+data["nick"]+": "+data["text"], "red");
}

void fsirc::gotNotice(QHash<QString, QString> data)
{
	fsEcho(tr("Notice> ")+data["nick"]+": "+data["text"], "#030A70");
}

void fsirc::gotAction(QHash<QString, QString> data)
{
	fsEcho("* "+data["nick"]+" "+data["text"], "#2DBF10");
}

void fsirc::sayHere()
{
	QString msg=fsCmdEdit->text();
	if(!msg.isEmpty())
	{
		fsCmdEdit->clear();
		if(!msg.startsWith(FS_COM_CHAR)) // a shout
		{
			irc->ircMsg(msg,irc->channel());
			fsEcho(irc->nick()+": "+msg);
		}else // a command
		{
			msg.remove(0,1);
			QRegExp command("^([a-zA-z]+) (.+)$");
			if(command.exactMatch(msg))
			{
				fsExec(command.cap(1),command.cap(2));
			}
		}
	}
}

void fsirc::gotInfo(QString message)
{
	// Various info
	fsEcho(message,"green");
}

void fsirc::gotNames(QStringList data)
{
	QString output = tr("Names for ") + data[1] + ":";
	for(int i = 2; i < data.count(); i++)
		output.append(" " + data[i]);
	fsEcho(output, "blue");
}

void fsirc::gotTopic(QStringList data)
{
	fsEcho(tr("Topic for ") + data[1] + ": " + data[2], "blue");
}

void fsirc::gotNick(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" is now known as ") + data["target"], "blue");
	// Record nick to history
	if(irc->nick()==data["target"])
	{
		fSettings settings;
		settings.setValue("Nickname", data["target"]);
		settings.setValue("servers/"+irc->server()+"/Nickname", data["target"]);
		settings.beginGroup("history");
		if(settings.appendValue(data["target"],"irc-nicks","nick"))
			initCompleters();
	}
}

void fsirc::gotJoin(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has joined ") + data["target"], "blue");
}

void fsirc::gotPart(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has left ") + data["target"] + ": "+data["text"], "blue");
}

void fsirc::gotQuit(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has quit IRC ")+data["target"] + ": " + data["text"], "blue");
}

void fsirc::gotKick(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has kicked ") + data["subject"] + tr(" out of ") + data["target"] + ": " + data["text"], "red");
}

void fsirc::gotMode(QHash<QString, QString> data)
{
	fsEcho(data["nick"] + tr(" has set mode ") + data["text"] + " " + data["subject"] + tr(" on ") + data["target"], "blue");
}

void fsirc::fsExec(QString cmd, QString arg)
{
	cmd.toLower();
	if(cmd=="raw")
	{
		irc->ircThrow(arg);
		fsEcho("[RAW] "+arg, "brown");
	} else
	if(cmd=="me")
	{
		QString sarg = arg;
		arg.prepend("ACTION ");
		arg.prepend('\x01');
		arg.append('\x01');
		irc->ircMsg(arg, irc->channel());
		fsEcho("* "+irc->nick()+" "+sarg, "#2DBF10");
	} else
	if(cmd=="join")
	{
		irc->ircJoin(arg);
	} else
	if(cmd=="nick")
	{
		irc->ircSetNick(arg);
	} else
	if(cmd=="mode")
	{
		irc->ircMode(arg);
	} else
	if((cmd=="nickserv")||(cmd=="ns"))
	{
		irc->ircNs(arg);
	} else
	if((cmd=="chanserv")||(cmd=="cs"))
	{
		irc->ircCs(arg);
	} else
	if(cmd=="quit")
	{
		irc->ircQuit(arg);
	} else
	if((cmd=="memoserv")||(cmd=="ms"))
	{
		irc->ircMs(arg);
	} else
        if(mArg.exactMatch(arg))
	{
	// More-than-one parameters
		if(cmd=="msg")
		{
			irc->ircMsg(mArg.cap(2), mArg.cap(1));
		} else
		if(cmd=="kick")
		{
			irc->ircKick(mArg.cap(1), mArg.cap(2));
		}
	} else
	fsEcho(tr("Unknown command: ") + cmd, "red");
}

void fsirc::fsQuit()
{
	irc->ircQuit("Fsirc "+QString(FS_VERSION)+" by Voker57. "+tr("STOP DRINKING COGNAC ON MORNINGS!"));
}
