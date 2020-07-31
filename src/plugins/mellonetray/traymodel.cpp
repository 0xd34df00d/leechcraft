/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "traymodel.h"
#include <QAbstractEventDispatcher>
#include <QtDebug>
#include <util/x11/xwrapper.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>
#include <xcb/xcb.h>
#include <xcb/damage.h>

namespace LC
{
namespace Mellonetray
{
	namespace
	{
		VisualID GetVisual ()
		{
			VisualID result = 0;
			auto disp = Util::XWrapper::Instance ().GetDisplay ();

			XVisualInfo init;
			init.screen = QX11Info::appScreen ();
			init.depth = 32;
			init.c_class = TrueColor;

			int nvi = 0;
			auto xvi = XGetVisualInfo (disp, VisualScreenMask | VisualDepthMask | VisualClassMask, &init, &nvi);
			if (!xvi)
				return result;

			for (int i = 0; i < nvi; ++i)
			{
				auto fmt = XRenderFindVisualFormat (disp, xvi [i].visual);
				if (fmt && fmt->type == PictTypeDirect && fmt->direct.alphaMask)
				{
					result = xvi [i].visualid;
					break;
				}
			}

			XFree (xvi);

			return result;
		}
	}

	TrayModel::TrayModel ()
	{
		QAbstractEventDispatcher::instance ()->installNativeEventFilter (this);

		QHash<int, QByteArray> roleNames;
		roleNames [Role::ItemID] = "itemID";
		setRoleNames (roleNames);

		auto& w = Util::XWrapper::Instance ();
		const auto disp = w.GetDisplay ();
		const auto rootWin = w.GetRootWindow ();

		const auto atom = w.GetAtom (QString ("_NET_SYSTEM_TRAY_S%1").arg (DefaultScreen (disp)));

		if (XGetSelectionOwner (disp, atom) != None)
		{
			qWarning () << Q_FUNC_INFO
					<< "another system tray is active";
			return;
		}

		TrayWinID_ = XCreateSimpleWindow (disp, rootWin, -1, -1, 1, 1, 0, 0, 0);
		XSetSelectionOwner (disp, atom, TrayWinID_, CurrentTime);
		if (XGetSelectionOwner (disp, atom) != TrayWinID_)
		{
			qWarning () << Q_FUNC_INFO
					<< "call to XSetSelectionOwner failed";
			return;
		}

		int orientation = 0;
		XChangeProperty (disp,
				TrayWinID_,
				w.GetAtom ("_NET_SYSTEM_TRAY_ORIENTATION"),
				XA_CARDINAL,
				32,
				PropModeReplace,
				reinterpret_cast<uchar*> (&orientation),
				1);

		if (auto visual = GetVisual ())
			XChangeProperty (disp,
					TrayWinID_,
					w.GetAtom ("_NET_SYSTEM_TRAY_VISUAL"),
					XA_VISUALID,
					32,
					PropModeReplace,
					reinterpret_cast<uchar*> (&visual),
					1);

		XClientMessageEvent ev;
		ev.type = ClientMessage;
		ev.window = rootWin;
		ev.message_type = w.GetAtom ("MANAGER");
		ev.format = 32;
		ev.data.l [0] = CurrentTime;
		ev.data.l [1] = atom;
		ev.data.l [2] = TrayWinID_;
		ev.data.l [3] = 0;
		ev.data.l [4] = 0;
		XSendEvent (disp, rootWin, False, StructureNotifyMask, reinterpret_cast<XEvent*> (&ev));

		int damageErr = 0;
		XDamageQueryExtension (disp, &DamageEvent_, &damageErr);

		IsValid_ = true;
	}

	TrayModel& TrayModel::Instance ()
	{
		static TrayModel m;
		return m;
	}

	void TrayModel::Release ()
	{
		if (TrayWinID_)
			XDestroyWindow (Util::XWrapper::Instance ().GetDisplay (), TrayWinID_);
	}

	bool TrayModel::IsValid () const
	{
		return IsValid_;
	}

	int TrayModel::columnCount (const QModelIndex&) const
	{
		return 1;
	}

	int TrayModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Items_.size ();
	}

	QModelIndex TrayModel::index (int row, int column, const QModelIndex& parent) const
	{
		return hasIndex (row, column, parent) ?
				createIndex (row, column) :
				QModelIndex {};
	}

	QModelIndex TrayModel::parent (const QModelIndex&) const
	{
		return {};
	}

	QVariant TrayModel::data (const QModelIndex& index, int role) const
	{
		const auto row = index.row ();
		const auto& item = Items_.at (row);

		switch (role)
		{
		case Qt::DisplayRole:
		case Role::ItemID:
			return static_cast<qulonglong> (item.WID_);
		}

		return {};
	}

	template<>
	void TrayModel::HandleClientMsg<xcb_client_message_event_t*> (xcb_client_message_event_t *ev)
	{
		if (ev->type != Util::XWrapper::Instance ().GetAtom ("_NET_SYSTEM_TRAY_OPCODE"))
			return;

		switch (ev->data.data32 [1])
		{
		case 0:
			if (auto id = ev->data.data32 [2])
				Add (id);
		default:
			break;
		}
	}

	void TrayModel::Add (ulong wid)
	{
		if (FindItem (wid) != Items_.end ())
			return;

		beginInsertRows ({}, Items_.size (), Items_.size ());
		Items_.append ({ wid });
		endInsertRows ();
	}

	void TrayModel::Remove (ulong wid)
	{
		const auto pos = FindItem (wid);
		if (pos == Items_.end ())
			return;

		const auto dist = std::distance (Items_.begin (), pos);
		beginRemoveRows ({}, dist, dist);
		Items_.erase (pos);
		endRemoveRows ();
	}

	void TrayModel::Update (ulong wid)
	{
		emit updateRequired (wid);
	}

	bool TrayModel::nativeEventFilter (const QByteArray& eventType, void *msg, long int*)
	{
		if (eventType != "xcb_generic_event_t")
			return false;

		const auto ev = static_cast<xcb_generic_event_t*> (msg);

		switch (ev->response_type & ~0x80)
		{
		case XCB_CLIENT_MESSAGE:
			HandleClientMsg (static_cast<xcb_client_message_event_t*> (msg));
			break;
		case XCB_DESTROY_NOTIFY:
			Remove (static_cast<xcb_destroy_notify_event_t*> (msg)->window);
			break;
		default:
			if (ev->response_type == XCB_DAMAGE_NOTIFY + DamageEvent_)
			{
				auto dmg = static_cast<xcb_damage_notify_event_t*> (msg);
				Update (dmg->drawable);
			}
			break;
		}

		return false;
	}

	auto TrayModel::FindItem (ulong wid) -> QList<TrayItem>::iterator
	{
		return std::find_if (Items_.begin (), Items_.end (),
				[&wid] (const TrayItem& item) { return item.WID_ == wid; });
	}
}
}
