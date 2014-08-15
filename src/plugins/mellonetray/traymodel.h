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

#include <QAbstractItemModel>
#include <QIcon>

#if QT_VERSION < 0x050000
#include <QAbstractEventDispatcher>
#else
#include <QAbstractNativeEventFilter>
#endif

#include <util/models/rolenamesmixin.h>

namespace LeechCraft
{
namespace Mellonetray
{
	class TrayModel : public Util::RoleNamesMixin<QAbstractItemModel>
#if QT_VERSION >= 0x050000
					, public QAbstractNativeEventFilter
#endif
	{
		Q_OBJECT

		bool IsValid_ = false;

		ulong TrayWinID_ = 0;
		int DamageEvent_ = 0;

		struct TrayItem
		{
			ulong WID_;
		};
		QList<TrayItem> Items_;

		enum Role
		{
			ItemID = Qt::UserRole + 1
		};

#if QT_VERSION < 0x050000
		const QAbstractEventDispatcher::EventFilter PrevFilter_;
#endif

		TrayModel ();

		TrayModel (const TrayModel&) = delete;
		TrayModel (TrayModel&&) = delete;

		TrayModel& operator= (const TrayModel&) = delete;
		TrayModel& operator= (TrayModel&&) = delete;
	public:
		static TrayModel& Instance ();
		void Release ();

		bool IsValid () const;

		int columnCount (const QModelIndex& parent = QModelIndex()) const;
		int rowCount (const QModelIndex& parent = QModelIndex()) const;
		QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const;
		QModelIndex parent (const QModelIndex& child) const;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;

#if QT_VERSION < 0x050000
		bool Filter (XEvent*);
#else
		bool nativeEventFilter (const QByteArray&, void*, long int*) override;
#endif
	private:
		template<typename T>
		void HandleClientMsg (T);

		void Add (ulong);
		void Remove (ulong);
		void Update (ulong);

		QList<TrayItem>::iterator FindItem (ulong);
	signals:
		void updateRequired (ulong);
	};
}
}
