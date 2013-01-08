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

#include "quarkproxy.h"
#include <QGraphicsObject>
#include <QtDebug>
#include <interfaces/iquarkcomponentprovider.h>
#include <util/util.h>
#include "viewmanager.h"
#include "sbview.h"
#include "quarkunhidelistview.h"
#include "quarkorderview.h"
#include "declarativewindow.h"

namespace LeechCraft
{
namespace SB2
{
	QuarkProxy::QuarkProxy (ViewManager *mgr, ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Manager_ (mgr)
	, Proxy_ (proxy)
	{
	}

	const QString& QuarkProxy::GetExtHoveredQuarkClass () const
	{
		return ExtHoveredQuarkClass_;
	}

	QPoint QuarkProxy::mapToGlobal (double x, double y)
	{
		return Manager_->GetView ()->mapToGlobal (QPoint (x, y));
	}

	void QuarkProxy::showSettings (const QUrl& url)
	{
		Manager_->ShowSettings (url);
	}

	void QuarkProxy::removeQuark (const QUrl& url)
	{
		Manager_->RemoveQuark (url);
	}

	QVariant QuarkProxy::openWindow (const QUrl& url, const QString& str, const QVariant& var)
	{
		const auto& newUrl = url.resolved (str);

		auto varMap = var.toMap ();

		const auto& existing = varMap.take ("existing").toString ();

		if ((existing == "toggle" || existing.isEmpty ()) &&
				URL2LastOpened_.value (newUrl))
		{
			URL2LastOpened_.take (newUrl)->deleteLater ();
			return QVariant ();
		}

		int x = varMap.take ("x").toInt ();
		int y = varMap.take ("y").toInt ();

		auto window = new DeclarativeWindow (newUrl, varMap, Proxy_);
		window->move (Util::FitRectScreen ({ x, y }, window->size ()));
		window->show ();

		URL2LastOpened_ [newUrl] = window;

		return QVariant::fromValue<QObject*> (window->rootObject ());
	}

	void QuarkProxy::quarkAddRequested (int x, int y)
	{
		auto toAdd = Manager_->FindAllQuarks ();
		for (const auto& existing : Manager_->GetAddedQuarks ())
		{
			const auto pos = std::find_if (toAdd.begin (), toAdd.end (),
					[&existing] (decltype (toAdd.at (0)) item) { return item.Url_ == existing; });
			if (pos == toAdd.end ())
				continue;

			toAdd.erase (pos);
		}

		if (toAdd.isEmpty ())
			return;

		auto unhide = new QuarkUnhideListView (toAdd, Manager_, Proxy_, Manager_->GetView ());
		unhide->move (x, y);
		unhide->show ();
	}

	void QuarkProxy::quarkOrderRequested (int x, int y)
	{
		auto view = new QuarkOrderView (Manager_, Proxy_);
		view->move (Util::FitRectScreen ({ x, y }, view->size ()));
		view->show ();

		connect (view,
				SIGNAL (quarkClassHovered (QString)),
				this,
				SLOT (handleExtHoveredQuarkClass (QString)));
	}

	void QuarkProxy::handleExtHoveredQuarkClass (const QString& qClass)
	{
		if (ExtHoveredQuarkClass_ == qClass)
			return;

		ExtHoveredQuarkClass_ = qClass;
		emit extHoveredQuarkClassChanged ();
	}
}
}
