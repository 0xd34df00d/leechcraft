/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_PLUGINS_PINTAB_PINTAB_H
#define PLUGINS_POSHUKU_PLUGINS_PINTAB_PINTAB_H
#include <QObject>
#include <QAction>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QGraphicsWebView>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/core/ihookproxy.h>

Q_DECLARE_METATYPE(QList<QObject*>)

namespace LeechCraft
{
namespace Poshuku
{
namespace PinTab
{
	//TODO:
	//- hide close button on pinned tabs
	//- move pinned tabs to left
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		ICoreProxy_ptr CoreProxy_;
		QMap<const QObject*, QAction*> ActionsMap_;
		QMap<QObject*, QString> Pinned_;
		QStringList PinnedUrls_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QSet<QByteArray> GetPluginClasses () const;
	private:
		void ChangeTabTitle (QObject *widget, const QString& title);
		void SetPinned (QObject *widget, QAction *action, bool pinned);
		QGraphicsWebView* GetWebView (QObject *browserWidget);
		void SavePinned ();
	private slots:
		void handlePinTabTriggered ();
		void handlePinnedTitleChanged (const QString& title);
	public slots:
		void hookMoreMenuFillEnd (LeechCraft::IHookProxy_ptr proxy,
			QMenu *menu,
			QGraphicsWebView *webView,
			QObject *browserWidget);
		void hookTabBarContextMenuActions (LeechCraft::IHookProxy_ptr proxy,
			const QObject *browserWidget) const;
		void hookTabRemoveRequested (LeechCraft::IHookProxy_ptr proxy,
			QObject *browserWidget);
		void hookTabAdded (LeechCraft::IHookProxy_ptr,
			QObject *browserWidget,
			QGraphicsWebView *view,
			const QUrl& url);


	};
}
}
}

#endif
