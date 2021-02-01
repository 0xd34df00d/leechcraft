/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>

class QModelIndex;

template<typename>
class QList;

class IJobHolderRepresentationHandler
{
public:
	virtual ~IJobHolderRepresentationHandler () = default;

	virtual void HandleCurrentChanged (const QModelIndex&) {}
	virtual void HandleCurrentColumnChanged (const QModelIndex&) {}
	virtual void HandleCurrentRowChanged (const QModelIndex&) {}
	virtual void HandleSelectedRowsChanged (const QList<QModelIndex>&) {}

	virtual void HandleActivated (const QModelIndex&) {}
	virtual void HandleClicked (const QModelIndex&) {}
	virtual void HandleDoubleClicked (const QModelIndex&) {}
	virtual void HandleEntered (const QModelIndex&) {}
	virtual void HandlePressed (const QModelIndex&) {}
};

using IJobHolderRepresentationHandler_ptr = std::shared_ptr<IJobHolderRepresentationHandler>;
