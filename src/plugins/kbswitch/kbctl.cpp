/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kbctl.h"
#include <algorithm>
#include <QtDebug>
#include <QTimer>
#include <QProcess>
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QGuiApplication>
#include <util/x11/xwrapper.h>
#include "xmlsettingsmanager.h"
#include "rulesstorage.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/XKBlib.h>

#include <xcb/xcb.h>

#define explicit xcb_authors_dont_care_about_cplusplus
#include <xcb/xkb.h>
#undef explicit

namespace LC
{
namespace KBSwitch
{
	KBCtl::KBCtl ()
	: Display_ { Util::XWrapper::Instance ().GetDisplay () }
	, Rules_ { new RulesStorage { Display_ } }
	{
		if (!InitDisplay ())
			return;

		QAbstractEventDispatcher::instance ()->installNativeEventFilter (this);

		const auto conn = Util::XWrapper::Instance ().GetConnection ();

		const uint32_t rootEvents [] =
		{
			XCB_EVENT_MASK_STRUCTURE_NOTIFY |
				XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
				XCB_EVENT_MASK_PROPERTY_CHANGE |
				XCB_EVENT_MASK_FOCUS_CHANGE |
				XCB_EVENT_MASK_KEYMAP_STATE |
				XCB_EVENT_MASK_LEAVE_WINDOW |
				XCB_EVENT_MASK_ENTER_WINDOW
		};
		xcb_change_window_attributes (conn,
				Window_, XCB_CW_EVENT_MASK, rootEvents);

		const uint16_t requiredMapParts = 0xffff;

		const uint16_t requiredEvents = XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY |
				XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
				XCB_XKB_EVENT_TYPE_STATE_NOTIFY;

		xcb_xkb_select_events (conn,
				XCB_XKB_ID_USE_CORE_KBD,
				requiredEvents,
				0,
				requiredEvents,
				requiredMapParts,
				requiredMapParts,
				nullptr);

		CheckExtWM ();

		if (!ExtWM_)
			SetupNonExtListeners ();

		UpdateGroupNames ();

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_KBSwitch");
		settings.beginGroup ("Groups");
		SetEnabledGroups (settings.value ("Groups").toStringList ());
		SetGroupVariants (settings.value ("Variants").toStringList ());
		settings.endGroup ();

		XmlSettingsManager::Instance ().RegisterObject ({
					"ManageSystemWide",
					"GlobalSwitchingPolicy",
					"KeyboardModel",
					"ManageKeyRepeat",
					"RepeatRate",
					"RepeatTimeout"
				},
				this, "scheduleApply");
		scheduleApply ();
	}

	bool KBCtl::InitDisplay ()
	{
		const auto conn = Util::XWrapper::Instance ().GetConnection ();
		const auto reply = xcb_get_extension_data (conn, &xcb_xkb_id);

		if (!reply || !reply->present)
		{
			qWarning () << Q_FUNC_INFO
					<< "XKB extension not present";
			return false;
		}

		XkbEventType_ = reply->first_event;

		xcb_xkb_use_extension (conn, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

		Window_ = DefaultRootWindow (Display_);
		NetActiveWinAtom_ = Util::XWrapper::Instance ().GetAtom ("_NET_ACTIVE_WINDOW");

		Available_ = true;
		return true;
	}

	KBCtl& KBCtl::Instance ()
	{
		static KBCtl ctl;
		return ctl;
	}

	void KBCtl::SetSwitchPolicy (SwitchPolicy policy)
	{
		Policy_ = policy;
	}

	int KBCtl::GetCurrentGroup () const
	{
		XkbStateRec state;
		XkbGetState (Display_, XkbUseCoreKbd, &state);
		return state.group;
	}

	const QStringList& KBCtl::GetEnabledGroups () const
	{
		return Groups_;
	}

	void KBCtl::SetEnabledGroups (QStringList groups)
	{
		if (groups.isEmpty ())
			return;

		if (groups.contains ("us") && groups.at (0) != "us")
		{
			groups.removeAll ("us");
			groups.prepend ("us");
		}

		if (Groups_ == groups)
			return;

		Groups_ = groups;
		scheduleApply ();
	}

	QString KBCtl::GetGroupVariant (int groupIdx) const
	{
		return Variants_.value (groupIdx);
	}

	void KBCtl::SetGroupVariants (const QStringList& variants)
	{
		if (variants.isEmpty ())
			return;

		Variants_ = variants;
		scheduleApply ();
	}

	void KBCtl::EnableNextGroup ()
	{
		const int count = GetEnabledGroups ().count ();
		EnableGroup ((GetCurrentGroup () + 1) % count);
	}

	void KBCtl::EnableGroup (int group)
	{
		XkbLockGroup (Display_, XkbUseCoreKbd, group);

		/* What an utter crap X11 is actually. The group doesn't get
		 * updated by the line above until we make another request to
		 * the X server, which this line basically does.
		 *
		 * Dunno why I'm writing this as I don't write comments for code
		 * at all. Probably for easy grepping by "crap" or "X!1".
		 */
		GetCurrentGroup ();
	}

	int KBCtl::GetMaxEnabledGroups () const
	{
		return XkbNumKbdGroups;
	}

	QString KBCtl::GetLayoutName (int group) const
	{
		return Groups_.value (group);
	}

	QString KBCtl::GetLayoutDesc (int group) const
	{
		return Rules_->GetLayoutsN2D () [GetLayoutName (group)];
	}

	void KBCtl::SetOptions (const QStringList& opts)
	{
		if (Options_ == opts)
			return;

		Options_ = opts;
		Options_.sort ();
		scheduleApply ();
	}

	const RulesStorage* KBCtl::GetRulesStorage () const
	{
		return Rules_;
	}

	bool KBCtl::nativeEventFilter (const QByteArray& eventType, void *msg, qintptr*)
	{
		if (!Available_)
			return false;

		if (eventType != "xcb_generic_event_t")
			return false;

		const auto ev = static_cast<xcb_generic_event_t*> (msg);

		if ((ev->response_type & ~0x80) == XkbEventType_)
			HandleXkbEvent (msg);

		switch (ev->response_type & ~0x80)
		{
		case XCB_FOCUS_IN:
		case XCB_FOCUS_OUT:
		case XCB_PROPERTY_NOTIFY:
			SetWindowLayout (Util::XWrapper::Instance ().GetActiveApp ());
			break;
		case XCB_CREATE_NOTIFY:
			AssignWindow (static_cast<xcb_create_notify_event_t*> (msg)->window);
			break;
		case XCB_DESTROY_NOTIFY:
			Win2Group_.remove (static_cast<xcb_destroy_notify_event_t*> (msg)->window);
			break;
		}

		return false;
	}

	void KBCtl::HandleXkbEvent (void *msg)
	{
		const auto ev = static_cast<xcb_generic_event_t*> (msg);
		switch (ev->pad0)
		{
		case XCB_XKB_STATE_NOTIFY:
		{
			const auto stateEv = static_cast<xcb_xkb_state_notify_event_t*> (msg);
			if (stateEv->group == stateEv->lockedGroup)
				Win2Group_ [Util::XWrapper::Instance ().GetActiveApp ()] = stateEv->group;
			emit groupChanged (stateEv->group);
			break;
		}
		case XCB_XKB_NEW_KEYBOARD_NOTIFY:
			Win2Group_.clear ();
			UpdateGroupNames ();
			break;
		}
	}

	void KBCtl::SetWindowLayout (ulong window)
	{
		if (Policy_ != SwitchPolicy::PerWindow)
			return;

		if (window == None)
			return;

		if (!Win2Group_.contains (window))
			return;

		const auto group = Win2Group_ [window];
		XkbLockGroup (Display_, XkbUseCoreKbd, group);

		/* See comments in SetGroup() for details of X11 crappiness.
		 */
		GetCurrentGroup ();
	}

	void KBCtl::CheckExtWM ()
	{
		if (!Available_)
			return;

		Atom type;
		int format;
		uchar *prop = nullptr;
		ulong count, after;
		const auto ret = XGetWindowProperty (Display_, Window_, NetActiveWinAtom_,
				0, sizeof (Window), 0, XA_WINDOW,
				&type, &format, &count, &after, &prop);
		if (ret == Success && prop)
		{
			XFree (prop);
			ExtWM_ = true;
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "extended window manager hints support wasn't detected, this won't work";
	}

	void KBCtl::SetupNonExtListeners ()
	{
		if (!Available_)
			return;

		uint count = 0;
		Window d1, d2;
		Window *windows = nullptr;

		if (!XQueryTree (Display_, Window_, &d1, &d2, &windows, &count))
			return;

		for (uint i = 0; i < count; ++i)
			AssignWindow (windows [i]);

		if (windows)
			XFree (windows);
	}

	void KBCtl::UpdateGroupNames ()
	{
		auto desc = XkbAllocKeyboard ();
		XkbGetControls (Display_, XkbAllControlsMask, desc);
		XkbGetNames (Display_, XkbSymbolsNameMask | XkbGroupNamesMask, desc);

		if (!desc->names)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot get names";
			return;
		}

		Groups_.clear ();

		const auto group = desc->names->groups;
		size_t groupCount = 0;
		for (; groupCount < XkbNumKbdGroups && group [groupCount]; ++groupCount) ;

		auto result = std::make_unique<char*[]> (groupCount);
		XGetAtomNames (Display_, group, groupCount, result.get ());

		const auto& layoutsD2N = Rules_->GetLayoutsD2N ();
		const auto& varredLayouts = Rules_->GetVariantsD2Layouts ();
		for (size_t i = 0; i < groupCount; ++i)
		{
			const QString str (result [i]);
			XFree (result [i]);

			if (!layoutsD2N [str].isEmpty ())
			{
				const auto& grp = layoutsD2N [str];
				Groups_ << grp;
				Variants_ << QString ();
			}
			else if (!varredLayouts [str].first.isEmpty ())
			{
				const auto& grp = varredLayouts [str];
				Groups_ << grp.first;
				Variants_ << grp.second;
			}
			else
			{
				qWarning () << Q_FUNC_INFO
						<< str
						<< "not present anywhere";
				qWarning () << varredLayouts.contains (str);
			}
		}

		XkbFreeNames (desc, XkbSymbolsNameMask | XkbGroupNamesMask, True);
	}

	void KBCtl::AssignWindow (ulong window)
	{
		if (ExtWM_)
			return;

		XWindowAttributes wa;
		if (!XGetWindowAttributes (Display_, window, &wa))
			return;

		const auto windowEvents = EnterWindowMask |
				FocusChangeMask |
				PropertyChangeMask |
				StructureNotifyMask;
		XSelectInput (Display_, window, windowEvents);
	}

	void KBCtl::scheduleApply ()
	{
		if (ApplyScheduled_)
			return;

		ApplyScheduled_ = true;
		QTimer::singleShot (100,
				this,
				SLOT (apply ()));
	}

	void KBCtl::ApplyKeyRepeat ()
	{
		if (!XmlSettingsManager::Instance ().property ("ManageKeyRepeat").toBool ())
			return;

		XkbChangeEnabledControls (Display_, XkbUseCoreKbd, XkbRepeatKeysMask, XkbRepeatKeysMask);

		auto timeout = XmlSettingsManager::Instance ().property ("RepeatTimeout").toUInt ();
		auto rate = XmlSettingsManager::Instance ().property ("RepeatRate").toUInt ();
		XkbSetAutoRepeatRate (Display_, XkbUseCoreKbd, timeout, 1000 / rate);

		// X11 is crap, XkbSetAutoRepeatRate() doesn't work next time if we don't call this.
		XkbGetAutoRepeatRate (Display_, XkbUseCoreKbd, &timeout, &rate);
	}

	void KBCtl::ApplyGlobalSwitchingPolicy ()
	{
		const auto& prop = XmlSettingsManager::Instance ().property ("GlobalSwitchingPolicy").toString ();
		if (prop == "global")
			Policy_ = SwitchPolicy::Global;
		else if (prop == "perWindow")
			Policy_ = SwitchPolicy::PerWindow;
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown global switching policy"
					<< prop;
	}

	void KBCtl::apply ()
	{
		ApplyScheduled_ = false;

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_KBSwitch");
		settings.beginGroup ("Groups");
		settings.remove ("");
		settings.setValue ("Groups", Groups_);
		settings.setValue ("Variants", Variants_);
		settings.endGroup ();

		ApplyGlobalSwitchingPolicy ();

		if (!XmlSettingsManager::Instance ()
				.property ("ManageSystemWide").toBool ())
			return;

		auto kbModel = XmlSettingsManager::Instance ()
				.property ("KeyboardModel").toString ();
		const auto& kbCode = Rules_->GetKBModelCode (kbModel);

		QStringList args
		{
			"-layout",
			Groups_.join (","),
			"-model",
			kbCode,
			"-option"
		};

		if (!Options_.isEmpty ())
			args << "-option"
					<< Options_.join (",");

		if (std::any_of (Variants_.begin (), Variants_.end (),
				[] (const QString& str) { return !str.isEmpty (); }))
			args << "-variant"
					<< Variants_.join (",");

		qDebug () << Q_FUNC_INFO << args;
		QProcess::startDetached ("setxkbmap", args);

		ApplyKeyRepeat ();
	}
}
}
