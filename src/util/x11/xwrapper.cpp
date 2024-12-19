/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xwrapper.h"
#include <limits>
#include <type_traits>
#include <bit>
#include <QString>
#include <QPixmap>
#include <QIcon>
#include <QApplication>
#include <QWidget>
#include <QDesktopWidget>
#include <QAbstractEventDispatcher>
#include <QtDebug>
#include <QScreen>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <xcb/xcb.h>
#include <util/sll/qtutil.h>

namespace LC::Util
{
	const int SourcePager = 2;

	const int StateRemove = 0;
	const int StateAdd = 1;

	XWrapper::XWrapper ()
	: Display_ (QX11Info::display ())
	, AppWin_ (QX11Info::appRootWindow ())
	{
		QAbstractEventDispatcher::instance ()->installNativeEventFilter (this);

		const uint32_t rootEvents [] =
		{
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |
				XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
				XCB_EVENT_MASK_PROPERTY_CHANGE
		};
		xcb_change_window_attributes (QX11Info::connection (),
				AppWin_, XCB_CW_EVENT_MASK, rootEvents);
	}

	XWrapper& XWrapper::Instance ()
	{
		static XWrapper w;
		return w;
	}

	Display* XWrapper::GetDisplay () const
	{
		return Display_;
	}

	Window XWrapper::GetRootWindow () const
	{
		return AppWin_;
	}

	bool XWrapper::nativeEventFilter (const QByteArray& eventType, void *msg, long int*)
	{
		if (eventType != "xcb_generic_event_t")
			return false;

		const auto ev = static_cast<xcb_generic_event_t*> (msg);
		if ((ev->response_type & ~0x80) == XCB_PROPERTY_NOTIFY)
			HandlePropNotify (static_cast<xcb_property_notify_event_t*> (msg));

		return false;
	}

	namespace
	{
		template<typename T>
		struct IsDoublePtr : std::false_type {};

		template<typename T>
		struct IsDoublePtr<T**> : std::true_type {};

		template<typename T>
		class Guarded
		{
			T *Data_ = nullptr;
		public:
			Guarded () = default;

			Guarded (const Guarded&) = delete;
			Guarded& operator= (const Guarded&) = delete;

			Guarded (Guarded&& other) noexcept
			: Data_ { other.Data_ }
			{
				other.Data_ = nullptr;
			}

			Guarded& operator= (Guarded&& other) noexcept
			{
				std::swap (Data_, other.Data_);
				return *this;
			}

			~Guarded () noexcept
			{
				if (Data_)
					XFree (Data_);
			}

			T** Get (bool clear = true) noexcept
			{
				if (clear && Data_)
					XFree (Data_);
				return &Data_;
			}

			template<typename U>
			U GetAs (bool clear = true) noexcept
			{
				if (clear && Data_)
					XFree (Data_);
				return IsDoublePtr<U>::value ?
						reinterpret_cast<U> (&Data_) :
						reinterpret_cast<U> (Data_);
			}

			T operator[] (size_t idx) const noexcept
			{
				return Data_ [idx];
			}

			T& operator[] (size_t idx) noexcept
			{
				return Data_ [idx];
			}

			explicit operator bool () const noexcept
			{
				return Data_ != nullptr;
			}

			bool operator! () const noexcept
			{
				return !Data_;
			}
		};
	}

	void XWrapper::Sync ()
	{
		XFlush (Display_);
		XSync (Display_, False);
	}

	QList<Window> XWrapper::GetWindows ()
	{
		ulong length = 0;
		Guarded<Window> data;

		QList<Window> result;
		if (GetRootWinProp (GetAtom ("_NET_CLIENT_LIST"), &length, data.GetAs<uchar**> ()))
			for (ulong i = 0; i < length; ++i)
				result << data [i];
		return result;
	}

	QString XWrapper::GetWindowTitle (Window wid)
	{
		QString name;

		ulong length = 0;
		Guarded<uchar> data;

		auto utf8Str = GetAtom ("UTF8_STRING");

		if (GetWinProp (wid, GetAtom ("_NET_WM_VISIBLE_NAME"), &length, data.Get (), utf8Str))
			name = QString::fromUtf8 (data.GetAs<char*> (false));

		if (name.isEmpty () && GetWinProp (wid, GetAtom ("_NET_WM_NAME"), &length, data.Get (), utf8Str))
			name = QString::fromUtf8 (data.GetAs<char*> (false));

		if (name.isEmpty () && GetWinProp (wid, GetAtom ("XA_WM_NAME"), &length, data.Get (), XA_STRING))
			name = QString::fromUtf8 (data.GetAs<char*> (false));

		if (name.isEmpty ())
		{
			XFetchName (Display_, wid, data.GetAs<char**> ());
			name = QString (data.GetAs<char*> (false));
		}

		if (name.isEmpty ())
		{
			XTextProperty prop;
			if (XGetWMName (Display_, wid, &prop))
			{
				name = QString::fromUtf8 (reinterpret_cast<char*> (prop.value));
				XFree (prop.value);
			}
		}

		return name;
	}

	QIcon XWrapper::GetWindowIcon (Window wid)
	{
		int fmt = 0;
		ulong type, count, extra;
		Guarded<ulong> data;

		XGetWindowProperty (Display_, wid, GetAtom ("_NET_WM_ICON"),
				0, std::numeric_limits<long>::max (), False, AnyPropertyType,
				&type, &fmt, &count, &extra,
				data.GetAs<uchar**> ());

		if (!data)
			return {};

		QIcon icon;

		auto cur = *data.Get (false);
		auto end = cur + count;
		while (cur < end)
		{
			QImage img (cur [0], cur [1], QImage::Format_ARGB32);
			cur += 2;
			const auto bytesCount = img.sizeInBytes ();
			for (int i = 0; i < bytesCount / 4; ++i, ++cur)
				reinterpret_cast<uint*> (img.bits ()) [i] = *cur;

			icon.addPixmap (QPixmap::fromImage (img));
		}

		return icon;
	}

	template<typename Flag>
	QFlags<Flag> XWrapper::GetFlagsList (Window wid, Atom property, const QHash<Atom, Flag>& atom2flag) const
	{
		QFlags<Flag> result;

		ulong length = 0;
		ulong *data = 0;
		if (!GetWinProp (wid, property, &length, reinterpret_cast<uchar**> (&data), XA_ATOM))
			return result;

		for (ulong i = 0; i < length; ++i)
			result |= atom2flag.value (data [i], static_cast<Flag> (0));

		XFree (data);

		return result;
	}

	WinStateFlags XWrapper::GetWindowState (Window wid)
	{
		auto get = [this] (const QByteArray& atom) { return GetAtom (AsStringView ("_NET_WM_STATE_" + atom)); };
		static const QHash<Atom, WinStateFlag> atom2flag
		{
			{ get ("MODAL"), WinStateFlag::Modal },
			{ get ("STICKY"), WinStateFlag::Sticky },
			{ get ("MAXIMIZED_VERT"), WinStateFlag::MaximizedVert },
			{ get ("MAXIMIZED_HORZ"), WinStateFlag::MaximizedHorz },
			{ get ("SHADED"), WinStateFlag::Shaded },
			{ get ("SKIP_TASKBAR"), WinStateFlag::SkipTaskbar },
			{ get ("SKIP_PAGER"), WinStateFlag::SkipPager },
			{ get ("HIDDEN"), WinStateFlag::Hidden },
			{ get ("FULLSCREEN"), WinStateFlag::Fullscreen },
			{ get ("ABOVE"), WinStateFlag::OnTop },
			{ get ("BELOW"), WinStateFlag::OnBottom },
			{ get ("DEMANDS_ATTENTION"), WinStateFlag::Attention },
		};

		return GetFlagsList (wid, GetAtom ("_NET_WM_STATE"), atom2flag);
	}

	AllowedActionFlags XWrapper::GetWindowActions (Window wid)
	{
		auto get = [this] (const QByteArray& atom) { return GetAtom (AsStringView ("_NET_WM_ACTION_" + atom)); };
		static const QHash<Atom, AllowedActionFlag> atom2flag
		{
			{ get ("MOVE"), AllowedActionFlag::Move },
			{ get ("RESIZE"), AllowedActionFlag::Resize },
			{ get ("MINIMIZE"), AllowedActionFlag::Minimize },
			{ get ("SHADE"), AllowedActionFlag::Shade },
			{ get ("STICK"), AllowedActionFlag::Stick },
			{ get ("MAXIMIZE_HORZ"), AllowedActionFlag::MaximizeHorz },
			{ get ("MAXIMIZE_VERT"), AllowedActionFlag::MaximizeVert },
			{ get ("FULLSCREEN"), AllowedActionFlag::ShowFullscreen },
			{ get ("CHANGE_DESKTOP"), AllowedActionFlag::ChangeDesktop },
			{ get ("CLOSE"), AllowedActionFlag::Close },
			{ get ("ABOVE"), AllowedActionFlag::MoveToTop },
			{ get ("BELOW"), AllowedActionFlag::MoveToBottom },
		};

		return GetFlagsList (wid, GetAtom ("_NET_WM_ALLOWED_ACTIONS"), atom2flag);
	}

	Window XWrapper::GetActiveApp ()
	{
		auto win = GetActiveWindow ();
		if (!win)
			return 0;

		Window transient = None;
		if (!ShouldShow (win) && XGetTransientForHint (Display_, win, &transient))
			return transient;

		return win;
	}

	bool XWrapper::IsLCWindow (Window wid)
	{
		ulong length = 0;
		Guarded<uchar> data;
		return GetWinProp (wid, GetAtom ("WM_CLASS"), &length, data.Get ()) &&
				QString (data.GetAs<char*> (false)).startsWith ("leechcraft"_ql);
	}

	bool XWrapper::ShouldShow (Window wid)
	{
		static const QList<Atom> ignoreAtoms
		{
			GetAtom ("_NET_WM_WINDOW_TYPE_DESKTOP"),
			GetAtom ("_NET_WM_WINDOW_TYPE_DOCK"),
			GetAtom ("_NET_WM_WINDOW_TYPE_TOOLBAR"),
			GetAtom ("_NET_WM_WINDOW_TYPE_UTILITY"),
			GetAtom ("_NET_WM_WINDOW_TYPE_MENU"),
			GetAtom ("_NET_WM_WINDOW_TYPE_SPLASH"),
			GetAtom ("_NET_WM_WINDOW_TYPE_POPUP_MENU")
		};

		for (const auto& type : GetWindowType (wid))
			if (ignoreAtoms.contains (type))
				return false;

		if (GetWindowState (wid) & WinStateFlag::SkipTaskbar)
			return false;

		Window transient = None;
		if (!XGetTransientForHint (Display_, wid, &transient))
			return true;

		if (transient == 0 || transient == wid || transient == AppWin_)
			return true;

		return !GetWindowType (transient).contains (GetAtom ("_NET_WM_WINDOW_TYPE_NORMAL"));
	}

	void XWrapper::Subscribe (Window wid)
	{
		if (IsLCWindow (wid))
			return;

		XSelectInput (Display_, wid, PropertyChangeMask);
	}

	void XWrapper::SetStrut (QWidget *widget, Qt::ToolBarArea area)
	{
		const auto wid = widget->effectiveWinId ();

		const auto& winGeom = widget->geometry ();

		switch (area)
		{
		case Qt::BottomToolBarArea:
			SetStrut (wid,
					0, 0, 0, winGeom.height (),
					0, 0,
					0, 0,
					0, 0,
					winGeom.left (), winGeom.right ());
			break;
		case Qt::TopToolBarArea:
			SetStrut (wid,
					0, 0, winGeom.height (), 0,
					0, 0,
					0, 0,
					winGeom.left (), winGeom.right (),
					0, 0);
			break;
		case Qt::LeftToolBarArea:
			SetStrut (wid,
					winGeom.width (), 0, 0, 0,
					winGeom.top (), winGeom.bottom (),
					0, 0,
					0, 0,
					0, 0);
			break;
		case Qt::RightToolBarArea:
			SetStrut (wid,
					0, winGeom.width (), 0, 0,
					0, 0,
					winGeom.top (), winGeom.bottom (),
					0, 0,
					0, 0);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "incorrect area passed"
					<< area;
			break;
		}
	}

	void XWrapper::ClearStrut (QWidget *w)
	{
		const auto wid = w->effectiveWinId ();
		XDeleteProperty (Display_, wid, GetAtom ("_NET_WM_STRUT"));
		XDeleteProperty (Display_, wid, GetAtom ("_NET_WM_STRUT_PARTIAL"));
	}

	void XWrapper::SetStrut (Window wid,
			ulong left, ulong right, ulong top, ulong bottom,
			ulong leftStartY, ulong leftEndY,
			ulong rightStartY, ulong rightEndY,
			ulong topStartX, ulong topEndX,
			ulong bottomStartX, ulong bottomEndX)
	{
		ulong struts [12] =
		{
			left,
			right,
			top,
			bottom,

			leftStartY,
			leftEndY,
			rightStartY,
			rightEndY,

			topStartX,
			topEndX,
			bottomStartX,
			bottomEndX
		};

		XChangeProperty (Display_, wid, GetAtom ("_NET_WM_STRUT_PARTIAL"),
				XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<uchar*> (struts), 12);

		XChangeProperty (Display_, wid, GetAtom ("_NET_WM_STRUT"),
				XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<uchar*> (struts), 4);
	}

	void XWrapper::RaiseWindow (Window wid)
	{
		SendMessage (wid, GetAtom ("_NET_ACTIVE_WINDOW"), SourcePager);
	}

	void XWrapper::MinimizeWindow (Window wid)
	{
		SendMessage (wid, GetAtom ("WM_CHANGE_STATE"), IconicState);
	}

	void XWrapper::MaximizeWindow (Window wid)
	{
		SendMessage (wid, GetAtom ("_NET_WM_STATE"), StateAdd,
				GetAtom ("_NET_WM_STATE_MAXIMIZED_VERT"),
				GetAtom ("_NET_WM_STATE_MAXIMIZED_HORZ"),
				SourcePager);
	}

	void XWrapper::UnmaximizeWindow (Window wid)
	{
		SendMessage (wid, GetAtom ("_NET_WM_STATE"), StateRemove,
				GetAtom ("_NET_WM_STATE_MAXIMIZED_VERT"),
				GetAtom ("_NET_WM_STATE_MAXIMIZED_HORZ"),
				SourcePager);
	}

	void XWrapper::ResizeWindow (Window wid, int width, int height)
	{
		XResizeWindow (Display_, wid, width, height);
	}

	void XWrapper::ShadeWindow (Window wid)
	{
		SendMessage (wid, GetAtom ("_NET_WM_STATE"),
				StateAdd, GetAtom ("_NET_WM_STATE_SHADED"), 0, SourcePager);
	}

	void XWrapper::UnshadeWindow (Window wid)
	{
		SendMessage (wid, GetAtom ("_NET_WM_STATE"),
				StateRemove, GetAtom ("_NET_WM_STATE_SHADED"), 0, SourcePager);
	}

	void XWrapper::MoveWindowTo (Window wid, Layer layer)
	{
		const auto top = layer == Layer::Top ? StateAdd : StateRemove;
		const auto bottom = layer == Layer::Bottom ? StateAdd : StateRemove;

		SendMessage (wid, GetAtom ("_NET_WM_STATE"), top,
				GetAtom ("_NET_WM_STATE_ABOVE"), 0, SourcePager);

		SendMessage (wid, GetAtom ("_NET_WM_STATE"), bottom,
				GetAtom ("_NET_WM_STATE_BELOW"), 0, SourcePager);
	}

	void XWrapper::CloseWindow (Window wid)
	{
		SendMessage (wid, GetAtom ("_NET_CLOSE_WINDOW"), 0, SourcePager);
	}

	template<typename T>
	void XWrapper::HandlePropNotify (T ev)
	{
		if (ev->state == XCB_PROPERTY_DELETE)
			return;

		const auto wid = ev->window;

		if (wid == AppWin_)
		{
			if (ev->atom == GetAtom ("_NET_CLIENT_LIST"))
				emit windowListChanged ();
			else if (ev->atom == GetAtom ("_NET_ACTIVE_WINDOW"))
				emit activeWindowChanged ();
			else if (ev->atom == GetAtom ("_NET_CURRENT_DESKTOP"))
				emit desktopChanged ();
		}
		else
		{
			if (ev->atom == GetAtom ("_NET_WM_VISIBLE_NAME") ||
					ev->atom == GetAtom ("WM_NAME"))
				emit windowNameChanged (wid);
			else if (ev->atom == GetAtom ("_NET_WM_ICON"))
				emit windowIconChanged (wid);
			else if (ev->atom == GetAtom ("_NET_WM_DESKTOP"))
				emit windowDesktopChanged (wid);
			else if (ev->atom == GetAtom ("_NET_WM_STATE"))
				emit windowStateChanged (wid);
			else if (ev->atom == GetAtom ("_NET_WM_ALLOWED_ACTIONS"))
				emit windowActionsChanged (wid);
		}
	}

	Window XWrapper::GetActiveWindow ()
	{
		ulong length = 0;
		Guarded<ulong> data;

		if (!GetRootWinProp (GetAtom ("_NET_ACTIVE_WINDOW"), &length, data.GetAs<uchar**> (), XA_WINDOW))
			return 0;

		if (!length)
			return 0;

		return data [0];
	}

	int XWrapper::GetDesktopCount ()
	{
		ulong length = 0;
		Guarded<ulong> data;

		if (GetRootWinProp (GetAtom ("_NET_NUMBER_OF_DESKTOPS"), &length, data.GetAs<uchar**> (), XA_CARDINAL))
			return length > 0 ? data [0] : -1;

		return -1;
	}

	int XWrapper::GetCurrentDesktop ()
	{
		ulong length = 0;
		Guarded<ulong> data;

		if (GetRootWinProp (GetAtom ("_NET_CURRENT_DESKTOP"), &length, data.GetAs<uchar**> (), XA_CARDINAL))
			return length > 0 ? data [0] : -1;

		return -1;
	}

	void XWrapper::SetCurrentDesktop (int desktop)
	{
		SendMessage (AppWin_, GetAtom ("_NET_CURRENT_DESKTOP"), desktop);
	}

	QStringList XWrapper::GetDesktopNames ()
	{
		ulong length = 0;
		Guarded<uchar> data;

		if (!GetRootWinProp (GetAtom ("_NET_DESKTOP_NAMES"),
				&length, data.GetAs<uchar**> (), GetAtom ("UTF8_STRING")))
			return {};

		if (!data)
			return {};

		QStringList result;
		for (char *pos = data.GetAs<char*> (false), *end = data.GetAs<char*> (false) + length; pos < end; )
		{
			const auto& str = QString::fromUtf8 (pos);
			result << str;
			pos += str.toUtf8 ().size () + 1;
		}
		return result;
	}

	QString XWrapper::GetDesktopName (int desktop, const QString& def)
	{
		return GetDesktopNames ().value (desktop, def);
	}

	int XWrapper::GetWindowDesktop (Window wid)
	{
		ulong length = 0;
		Guarded<ulong> data;
		if (GetWinProp (wid, GetAtom ("_NET_WM_DESKTOP"), &length, data.GetAs<uchar**> (), XA_CARDINAL) && length)
			return data [0];

		if (GetWinProp (wid, GetAtom ("_WIN_WORKSPACE"), &length, data.GetAs<uchar**> (), XA_CARDINAL) && length)
			return data [0];

		return -1;
	}

	void XWrapper::MoveWindowToDesktop (Window wid, int num)
	{
		unsigned long data = num;
		XChangeProperty (QX11Info::display (),
					wid,
					GetAtom ("_NET_WM_DESKTOP"),
					XA_CARDINAL,
					32,
					PropModeReplace,
					reinterpret_cast<unsigned char*> (&data),
					1);
	}

	QRect XWrapper::GetAvailableGeometry (QScreen& screen)
	{
		auto available = screen.geometry ();
		const auto deskGeom = screen.virtualGeometry ();

		for (const auto wid : GetWindows ())
		{
			ulong length = 0;
			Guarded<ulong> struts;
			const auto status = GetWinProp (wid, GetAtom ("_NET_WM_STRUT_PARTIAL"),
					&length, struts.GetAs<uchar**> (), XA_CARDINAL);
			if (!status || length != 12)
				continue;

			const QRect left
			{
				static_cast<int> (deskGeom.x ()),
				static_cast<int> (deskGeom.y () + struts [4]),
				static_cast<int> (struts [0]),
				static_cast<int> (struts [5] - struts [4])
			};
			if (available.intersects (left))
				available.setX (left.width ());

			const QRect right
			{
				static_cast<int> (deskGeom.x () + deskGeom.width () - struts [1]),
				static_cast<int> (deskGeom.y () + struts [6]),
				static_cast<int> (struts [1]),
				static_cast<int> (struts [7] - struts [6])
			};
			if (available.intersects (right))
				available.setWidth (right.x () - available.x ());

			const QRect top
			{
				static_cast<int> (deskGeom.x () + struts [8]),
				static_cast<int> (deskGeom.y ()),
				static_cast<int> (struts [9] - struts [8]),
				static_cast<int> (struts [2])
			};
			if (available.intersects (top))
				available.setY (top.height ());

			const QRect bottom
			{
				static_cast<int> (deskGeom.x () + struts [10]),
				static_cast<int> (deskGeom.y () + deskGeom.height () - struts [3]),
				static_cast<int> (struts [11] - struts [10]),
				static_cast<int> (struts [3])
			};
			if (available.intersects (bottom))
				available.setHeight (bottom.y () - available.y ());
		}

		return available;
	}

	QRect XWrapper::GetAvailableGeometry (QWidget *widget)
	{
		return GetAvailableGeometry (*widget->screen ());
	}

	Atom XWrapper::GetAtom (std::string_view name)
	{
		const auto pos = Atoms_.find (AsByteArray (name));
		if (pos != Atoms_.end ())
			return *pos;

		auto atom = XInternAtom (Display_, name.data (), false);
		Atoms_ [ToByteArray (name)] = atom;
		return atom;
	}

	bool XWrapper::GetWinProp (Window win, Atom property,
			ulong *length, unsigned char **result, Atom req) const
	{
		int fmt = 0;
		ulong type = 0, rest = 0;
		return XGetWindowProperty (Display_, win,
				property, 0, 1024, false, req, &type,
				&fmt, length, &rest, result) == Success;
	}

	bool XWrapper::GetRootWinProp (Atom property,
			ulong *length, uchar **result, Atom req) const
	{
		return GetWinProp (AppWin_, property, length, result, req);
	}

	QList<Atom> XWrapper::GetWindowType (Window wid)
	{
		QList<Atom> result;

		ulong length = 0;
		ulong *data = nullptr;

		if (!GetWinProp (wid, GetAtom ("_NET_WM_WINDOW_TYPE"),
				&length, reinterpret_cast<uchar**> (&data)))
			return result;

		for (ulong i = 0; i < length; ++i)
			result << data [i];

		XFree (data);
		return result;
	}

	bool XWrapper::SendMessage (Window wid, Atom atom, ulong d0, ulong d1, ulong d2, ulong d3, ulong d4)
	{
		XEvent msg;
		msg.xclient.window = wid;
		msg.xclient.type = ClientMessage;
		msg.xclient.message_type = atom;
		msg.xclient.send_event = true;
		msg.xclient.display = Display_;
		msg.xclient.format = 32;
		msg.xclient.data.l [0] = d0;
		msg.xclient.data.l [1] = d1;
		msg.xclient.data.l [2] = d2;
		msg.xclient.data.l [3] = d3;
		msg.xclient.data.l [4] = d4;

		auto flags = SubstructureRedirectMask | SubstructureNotifyMask;
		return XSendEvent (Display_, AppWin_, false, flags, &msg) == Success;
	}

	void XWrapper::initialize ()
	{
	}
}
