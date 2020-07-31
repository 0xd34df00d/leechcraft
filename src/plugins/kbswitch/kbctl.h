/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QAbstractNativeEventFilter>

typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;

namespace LC
{
namespace KBSwitch
{
	class RulesStorage;

	class KBCtl : public QObject
				, public QAbstractNativeEventFilter
	{
		Q_OBJECT

		Display * const Display_;
		int XkbEventType_ = 0;

		ulong Window_ = 0;
		ulong NetActiveWinAtom_ = 0;

		bool ExtWM_ = false;

		QStringList Groups_;
		QStringList Variants_;

		QStringList Options_;

		QHash<ulong, uchar> Win2Group_;

		RulesStorage * const Rules_;

		bool ApplyScheduled_ = false;

		bool Available_ = false;

		KBCtl ();
	public:
		enum class SwitchPolicy
		{
			Global,
			PerWindow
		};
	private:
		SwitchPolicy Policy_ { SwitchPolicy::Global };
	public:
		KBCtl (const KBCtl&) = delete;
		KBCtl& operator= (const KBCtl&) = delete;

		static KBCtl& Instance ();

		void SetSwitchPolicy (SwitchPolicy);

		int GetCurrentGroup () const;

		const QStringList& GetEnabledGroups () const;
		void SetEnabledGroups (QStringList);
		QString GetGroupVariant (int) const;
		void SetGroupVariants (const QStringList&);
		void EnableNextGroup ();
		void EnableGroup (int);

		int GetMaxEnabledGroups () const;

		QString GetLayoutName (int group) const;
		QString GetLayoutDesc (int group) const;

		void SetOptions (const QStringList&);

		const RulesStorage* GetRulesStorage () const;

		bool nativeEventFilter (const QByteArray& eventType, void *message, long *result) override;
	private:
		void HandleXkbEvent (void*);

		void SetWindowLayout (ulong);

		bool InitDisplay ();
		void CheckExtWM ();
		void SetupNonExtListeners ();

		void UpdateGroupNames ();

		void AssignWindow (ulong);

		void ApplyKeyRepeat ();
		void ApplyGlobalSwitchingPolicy ();
	public slots:
		void scheduleApply ();
		void apply ();
	signals:
		void groupChanged (int);
	};
}
}
