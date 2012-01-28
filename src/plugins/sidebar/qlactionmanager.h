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

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/iactionsexporter.h>
#include <interfaces/core/icoreproxy.h>

namespace LeechCraft
{
namespace Sidebar
{
	class SBWidget;
	class ShowConfigDialog;

	class QLActionManager : public QObject
	{
		Q_OBJECT

		ICoreProxy_ptr Proxy_;
		SBWidget *Bar_;
		std::shared_ptr<ShowConfigDialog> CfgDialog_;
	public:
		QLActionManager (SBWidget*, ICoreProxy_ptr, QObject* = 0);

		void AddToQuickLaunch (const QList<QAction*>&);
		void AddToLCTray (const QList<QAction*>&);
	private:
		void AddLabeled (const QList<QAction*>&, const QString&);
	public slots:
		void handleGotActions (const QList<QAction*>&, LeechCraft::ActionsEmbedPlace);

		void handleShowActions (const QList<QAction*>&);
		void handleHideActions (const QList<QAction*>&);
	};
}
}
