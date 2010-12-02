/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_CHATTAB_H
#define PLUGINS_AZOTH_CHATTAB_H
#include <QWidget>
#include <QPointer>
#include <QPersistentModelIndex>
#include <QDateTime>
#include <interfaces/iinfo.h>
#include <interfaces/imultitabs.h>
#include "ui_chattab.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
			{
				class ICLEntry;
				class IMUCEntry;
				class IMessage;
			}

			class ChatTab : public QWidget
						  , public IMultiTabsWidget
			{
				Q_OBJECT
				Q_INTERFACES (IMultiTabsWidget)

				static QObject *S_ParentMultiTabs_;

				Ui::ChatTab Ui_;
				QByteArray EntryID_;
				QString Variant_;
				QRegExp LinkRegexp_;
				QColor BgColor_;
				QList<QColor> NickColors_;
				QList<QString> MsgHistory_;
				int CurrentHistoryPosition_;
			public:
				static void SetParentMultiTabs (QObject*);

				ChatTab (const QByteArray&, const QString&, QWidget* = 0);

				QList<QAction*> GetTabBarContextMenuActions () const;
				QObject* ParentMultiTabs () const;
				void NewTabRequested ();
				QToolBar* GetToolBar () const;
				void Remove ();
			private slots:
				void on_MsgEdit__returnPressed ();
				void on_MsgEdit__textChanged (const QString&);
				void on_SubjectButton__released ();
				void handleEntryMessage (QObject*);
				void handleViewLinkClicked (const QUrl&);
				void scrollToEnd ();
				void handleHistoryUp ();
				void handleHistoryDown ();
			private:
				template<typename T>
				T* GetEntry () const;

				void CheckMUC ();
				void HandleMUC ();

				/** Appends the message to the message view area.
				 */
				void AppendMessage (Plugins::IMessage*);

				QString FormatDate (QDateTime, Plugins::IMessage*);
				QString FormatNickname (QString, Plugins::IMessage*);
				QString FormatBody (QString, Plugins::IMessage*);

				void GenerateColors ();
			signals:
				void needToClose (ChatTab*);

				// Hooks
				void hookFormatDateTime (LeechCraft::IHookProxy_ptr proxy,
						QObject *chatTab,
						QDateTime dateTime,
						QObject *message);
				void hookFormatNickname (LeechCraft::IHookProxy_ptr proxy,
						QObject *chatTab,
						QString string,
						QObject *message);
				void hookFormatBodyBegin (LeechCraft::IHookProxy_ptr proxy,
						QObject *chatTab,
						QString string,
						QObject *message);
				void hookFormatBodyEnd (LeechCraft::IHookProxy_ptr proxy,
						QObject *chatTab,
						QString string,
						QObject *message);
				void hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy,
						QObject *chatTab,
						int type,
						QString variant,
						QString text);
				void hookMessageCreated (LeechCraft::IHookProxy_ptr proxy,
						QObject *chatTab,
						QObject *message);
			};

			typedef QPointer<ChatTab> ChatTab_ptr;
		}
	}
}

#endif
