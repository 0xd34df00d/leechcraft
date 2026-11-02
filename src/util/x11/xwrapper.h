/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <string>
#include <QList>
#include <QString>
#include <QHash>
#include <QObject>
#include <QAbstractNativeEventFilter>
#include <X11/Xdefs.h>
#include "x11config.h"
#include "winflags.h"

class QIcon;
class QWidget;
class QRect;
class QScreen;

using Display = struct _XDisplay;
using Window = unsigned long;

using XEvent = union _XEvent;

struct xcb_connection_t;

namespace LC::Util
{
	class UTIL_X11_API XWrapper : public QObject
								, public QAbstractNativeEventFilter
	{
		Q_OBJECT

		xcb_connection_t * const Conn_;
		Display * const Display_;
		const Window AppWin_;

		QHash<QByteArray, Atom> Atoms_;

		struct PrivateCtor { explicit PrivateCtor () = default; };
	public:
		XWrapper (xcb_connection_t*, Display*, PrivateCtor);

		enum class Layer
		{
			Top,
			Bottom,
			Normal
		};

		static std::optional<XWrapper&> Instance ();

		xcb_connection_t* GetConnection () const;
		Display* GetDisplay () const;
		Window GetRootWindow () const;

		bool nativeEventFilter (const QByteArray& eventType, void *message, qintptr *result) override;

		void Sync ();

		QList<Window> GetWindows ();
		QString GetWindowTitle (Window);
		QIcon GetWindowIcon (Window);
		WinStateFlags GetWindowState (Window);
		AllowedActionFlags GetWindowActions (Window);

		Window GetActiveApp ();

		bool IsLCWindow (Window);
		bool ShouldShow (Window);

		void Subscribe (Window);

		void SetStrut (QWidget*, Qt::ToolBarArea);
		void ClearStrut (QWidget*);
		void SetStrut (Window wid,
				ulong left, ulong right, ulong top, ulong bottom,
				ulong leftStartY, ulong leftEndY,
				ulong rightStartY, ulong rightEndY,
				ulong topStartX, ulong topEndX,
				ulong bottomStartX, ulong bottomEndX);

		void RaiseWindow (Window);
		void MinimizeWindow (Window);
		void MaximizeWindow (Window);
		void UnmaximizeWindow (Window);
		void ShadeWindow (Window);
		void UnshadeWindow (Window);
		void MoveWindowTo (Window, Layer);
		void CloseWindow (Window);

		void ResizeWindow (Window, int, int);

		int GetDesktopCount ();
		int GetCurrentDesktop ();
		void SetCurrentDesktop (int);
		QStringList GetDesktopNames ();
		QString GetDesktopName (int, const QString& = QString ());
		int GetWindowDesktop (Window);
		void MoveWindowToDesktop (Window, int);

		QRect GetAvailableGeometry (QScreen&);
		QRect GetAvailableGeometry (QWidget*);

		/** @brief Returns the atom denoting the given string.
		 *
		 * @param[in] str The name of the atom.
		 * @return An X11 Atom.
		 */
		Atom GetAtom (std::string_view str);

		/** @brief Returns the atom of an ICCCM manager selection on the
		 * default screen.
		 *
		 * @param[in] manager The selection name without the screen
		 * suffix, like `WM` for window managers, `_NET_WM_CM` for
		 * compositing managers or `_NET_SYSTEM_TRAY` for system trays.
		 * @return The atom of the selection, like the one of `WM_S0` for
		 * window managers on screen 0.
		 */
		Atom GetManagerSelection (std::string_view manager);

		/** @brief Returns the owner of the given selection.
		 *
		 * @param[in] selection The atom of the selection.
		 * @return The window owning the selection, or None if nobody
		 * holds it.
		 */
		Window GetSelectionOwner (Atom selection) const;
	private:
		template<typename T>
		void HandlePropNotify (T);

		template<typename Flag>
		QFlags<Flag> GetFlagsList (Window wid, Atom property, const QHash<Atom, Flag>& atom2flag) const;

		Window GetActiveWindow ();

		bool GetWinProp (Window, Atom, ulong*, uchar**, Atom = static_cast<Atom> (0)) const;
		bool GetRootWinProp (Atom, ulong*, uchar**, Atom = static_cast<Atom> (0)) const;
		QList<Atom> GetWindowType (Window);

		bool SendMessage (Window, Atom, ulong, ulong = 0, ulong = 0, ulong = 0, ulong = 0);
	private slots:
		void initialize ();
	signals:
		void windowListChanged ();
		void activeWindowChanged ();
		void desktopChanged ();

		void windowNameChanged (ulong);
		void windowIconChanged (ulong);
		void windowDesktopChanged (ulong);
		void windowStateChanged (ulong);
		void windowActionsChanged (ulong);
	};
}
