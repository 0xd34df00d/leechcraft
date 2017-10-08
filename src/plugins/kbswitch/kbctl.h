/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QHash>
#include <QAbstractNativeEventFilter>

typedef struct _XDisplay Display;
typedef union  _XEvent XEvent;

namespace LeechCraft
{
namespace KBSwitch
{
	class RulesStorage;

	class KBCtl : public QObject
				, public QAbstractNativeEventFilter
	{
		Q_OBJECT

		Display *Display_ = 0;
		int XkbEventType_ = 0;

		ulong Window_ = 0;
		ulong NetActiveWinAtom_ = 0;

		bool ExtWM_ = false;

		QStringList Groups_;
		QStringList Variants_;

		QStringList Options_;

		QHash<ulong, uchar> Win2Group_;

		RulesStorage *Rules_;

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
		SwitchPolicy Policy_;
	public:
		KBCtl (const KBCtl&) = delete;
		KBCtl& operator= (const KBCtl&) = delete;

		static KBCtl& Instance ();
		void Release ();

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
	public slots:
		void scheduleApply ();
		void apply ();
	signals:
		void groupChanged (int);
	};
}
}
