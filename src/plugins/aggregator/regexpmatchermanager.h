/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#ifndef PLUGINS_AGGREGATOR_REGEXPMATCHERMANAGER_H
#define PLUGINS_AGGREGATOR_REGEXPMATCHERMANAGER_H
#include <deque>
#include <stdexcept>
#include <QAbstractItemModel>
#include <QStringList>
#include <interfaces/structures.h>
#include "item.h"

namespace LeechCraft
{
namespace Aggregator
{
	struct Item;

	class RegexpMatcherManager : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		typedef std::pair<QString, QString> titlebody_t;
		class AlreadyExists : public std::runtime_error
		{
		public:
			explicit AlreadyExists (const std::string& str)
			: std::runtime_error (str)
			{
			}
		};

		class NotFound : public std::runtime_error
		{
		public:
			explicit NotFound (const std::string& str)
			: std::runtime_error (str)
			{
			}
		};

		class Malformed : public std::runtime_error
		{
		public:
			explicit Malformed (const std::string& str)
			: std::runtime_error (str)
			{
			}
		};

		struct RegexpItem
		{
			QString Title_;
			QString Body_;

			RegexpItem (const QString& = QString (),
					const QString& = QString ());
			bool operator== (const RegexpItem&) const;
			bool IsEqual (const QString&) const;
			QByteArray Serialize () const;
			void Deserialize (const QByteArray&);
		};
	private:
		QStringList ItemHeaders_;
		typedef std::deque<RegexpItem> items_t;
		items_t Items_;

		RegexpMatcherManager ();

		mutable bool SaveScheduled_;
	public:
		static RegexpMatcherManager& Instance ();
		virtual ~RegexpMatcherManager ();

		void Release ();
		void Add (const QString&, const QString&);
		void Remove (const QString&);
		void Remove (const QModelIndex&);
		void Modify (const QString&, const QString&);
		titlebody_t GetTitleBody (const QModelIndex&) const;
		void HandleItem (const Item_ptr&) const;

		virtual int columnCount (const QModelIndex& = QModelIndex ()) const;
		virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const;
		virtual Qt::ItemFlags flags (const QModelIndex&) const;
		virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const;
		virtual QModelIndex index (int, int, const QModelIndex& = QModelIndex()) const;
		virtual QModelIndex parent (const QModelIndex&) const;
		virtual int rowCount (const QModelIndex& = QModelIndex ()) const;
	private slots:
		void saveSettings () const;
	private:
		void RestoreSettings ();
		void ScheduleSave ();
	signals:
		void gotLink (const LeechCraft::Entity&) const;
	};
}
}

#endif
