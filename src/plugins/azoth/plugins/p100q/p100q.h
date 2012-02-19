/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2011 Minh Ngo
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

#ifndef PLUGINS_AZOTH_PLUGINS_P100Q_P100Q_H
#define PLUGINS_AZOTH_PLUGINS_P100Q_P100Q_H
#include <QObject>
#include <QRegExp>
#include <QUrl>
#include <QWebView>
#include <QFile>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

class QTranslator;

namespace LeechCraft
{
namespace Azoth
{
namespace p100q
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		QRegExp UserRX_;
		QRegExp PostAuthorRX_;
		QRegExp PostRX_;
		QRegExp PostByUserRX_;
		QRegExp CommentRX_;
		QRegExp TagRX_;
		QRegExp ImgRX_;
		QRegExp PstoCommentRX_;

		Util::XmlSettingsDialog_ptr XmlSettingsDialog_;

		QMap<QObject*, QObject*> Entry2Tab_;
		QMap<QObject*, QString> LastPostInTab_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QStringList Provides () const;
		QStringList Needs () const;
		QStringList Uses () const;
		void SetProvider (QObject*, const QString&);

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private:
		QString FormatBody (QString);
	public slots:
		void hookChatTabCreated (LeechCraft::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				QWebView *webView);
		void hookFormatBodyEnd (LeechCraft::IHookProxy_ptr proxy,
				QObject *message);
	private slots:
		void handleShortcutActivated ();
		void handleChatDestroyed ();
	};
}
}
}

#endif
