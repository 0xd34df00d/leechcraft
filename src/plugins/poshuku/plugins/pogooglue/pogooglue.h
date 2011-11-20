/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_POGOOGLUE_POGOOGLUE_H
#define PLUGINS_POSHUKU_PLUGINS_POGOOGLUE_POGOOGLUE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/poshukutypes.h>
#include <interfaces/core/ihookproxy.h>

class QGraphicsWebView;
class QGraphicsSceneContextMenuEvent;
class QWebHitTestResult;

namespace LeechCraft
{
namespace Poshuku
{
namespace Pogooglue
{
	class Plugin : public QObject
					, public IInfo
					, public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		QString SelectedText_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QSet<QByteArray> GetPluginClasses () const;

	private slots:
		void handleGoogleIt ();
	public slots:
		void hookWebViewContextMenu (LeechCraft::IHookProxy_ptr,
				QGraphicsWebView*,
				QGraphicsSceneContextMenuEvent*,
				const QWebHitTestResult&, QMenu*,
				WebViewCtxMenuStage);
	signals:
		void gotEntity (const LeechCraft::Entity&);
	};
}
}
}

#endif // PLUGINS_POSHUKU_PLUGINS_POGOOGLUE_POGOOGLUE_H
