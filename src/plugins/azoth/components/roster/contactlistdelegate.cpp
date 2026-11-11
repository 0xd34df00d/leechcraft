/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "contactlistdelegate.h"
#include <algorithm>
#include <QPainter>
#include <QPainterStateGuard>
#include <QPixmap>
#include <QApplication>
#include <QAbstractProxyModel>
#include <QTreeView>
#include <util/sll/qobjectrefcast.h>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sys/resourceloader.h>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/media/audiostructs.h>
#include "avatarsmanager.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/isupportgeolocation.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iextselfinfoaccount.h"
#include "interfaces/azoth/ihavecontacttune.h"
#include "interfaces/azoth/ihavecontactmood.h"
#include "interfaces/azoth/ihavecontactactivity.h"
#include "interfaces/azoth/moodinfo.h"
#include "interfaces/azoth/activityinfo.h"
#include "components/util/gui.h"
#include "core.h"
#include "xmlsettingsmanager.h"
#include "resourcesmanager.h"
#include "roles.h"

namespace LC
{
namespace Azoth
{
	const int CPadding = 2;

	ContactListDelegate::ContactListDelegate (AvatarsManager& avatars, QTreeView& parent)
	: QStyledItemDelegate { &parent }
	, Avatars_ { avatars }
	, View_ { parent }
	{
		XmlSettingsManager::Instance ().RegisterObject ("ShowAvatars", this,
				[this] (bool show) { ShowAvatars_ = show; });
		XmlSettingsManager::Instance ().RegisterObject ("ShowClientIcons", this,
				[this] (bool show) { ShowClientIcons_ = show; });
		XmlSettingsManager::Instance ().RegisterObject ("ActivityIcons", this,
				[this] (const QString& iconset)
				{
					ActivityIconCache_.clear ();
					ActivityIconset_ = iconset;
				});
		XmlSettingsManager::Instance ().RegisterObject ("MoodIcons", this,
				[this] (const QString& iconset)
				{
					MoodIconCache_.clear ();
					MoodIconset_ = iconset;
				});
		XmlSettingsManager::Instance ().RegisterObject ("SystemIcons", this,
				[this] (const QString& iconset)
				{
					SystemIconCache_.clear ();
					SystemIconset_ = iconset;
				});
		XmlSettingsManager::Instance ().RegisterObject ("ShowStatuses", this,
				[this] (bool show) { ShowStatuses_ = show; });
		XmlSettingsManager::Instance ().RegisterObject ("HighlightGroups", this,
				[this] (bool highlight) { HighlightGroups_ = highlight; });
		XmlSettingsManager::Instance ().RegisterObject ("RosterContactHeight", this,
				[this] (int height)
				{
					ContactHeight_ = height <= 0 ? 24 : height;
					View_.setIconSize ({ ContactHeight_, ContactHeight_ });
				});

		XmlSettingsManager::Instance ().RegisterObject ({
					"ShowAvatars",
					"ShowClientIcons",
					"ActivityIcons",
					"AffIcons",
					"ClientIcons",
					"MoodIcons",
					"SystemIcons",
					"ShowStatuses",
					"HighlightGroups",
				},
				this,
				[this] { View_.viewport ()->update (); });

		Core::Instance ().RegisterHookable (this);
	}

	void ContactListDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& sopt, const QModelIndex& index) const
	{
		QPainterStateGuard guard { painter };
		switch (index.data (CLREntryType).value<CLEntryType> ())
		{
		case CLETAccount:
			DrawAccount (painter, sopt, index);
			break;
		case CLETCategory:
			DrawCategory (painter, sopt, index);
			break;
		case CLETContact:
			DrawContact (painter, sopt, index);
			break;
		}
	}

	QSize ContactListDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		QSize size = QStyledItemDelegate::sizeHint (option, index);

		switch (index.data (CLREntryType).value<CLEntryType> ())
		{
		case CLETContact:
			if (size.height () < ContactHeight_)
				size.setHeight (ContactHeight_);
			break;
		case CLETAccount:
			size.setHeight (size.height () * 1.1);
			break;
		case CLETCategory:
			const int textHeight = option.fontMetrics.height ();
			size.setHeight (qMax (textHeight + CPadding * 2, size.height ()));
			break;
		}

		return size;
	}

	namespace
	{
		Util::ContextTask<void> RepaintWhenReady (QAbstractItemView& view,
				QPersistentModelIndex index,
				Util::ContextTask<std::optional<QImage>> task)
		{
			co_await Util::AddContext { Util::CoroContext::Of (view) };
			if (co_await task)
				view.update (index);
		}

		QImage RequestAvatar (AvatarsManager& avatars, const GlobalStrongestId& id,
				QTreeView& view, const QModelIndex& index, int iconSize)
		{
			auto result = avatars.GetAvatarImageNow (id, AvatarsManager::Scaled { iconSize });
			const auto img = Util::Visit (result,
					[&] (const std::optional<QImage>& maybeImg) { return maybeImg.value_or ({}); },
					[&] (const Util::ContextTask<std::optional<QImage>>& task)
					{
						RepaintWhenReady (view, index, task);
						return QImage {};
					});
			return !img.isNull () ? img : ResourcesManager::Instance ().GetDefaultAvatar (iconSize);
		}
	}

	void ContactListDelegate::DrawAccount (QPainter *painter,
			QStyleOptionViewItem o, const QModelIndex& index) const
	{
		QStyle *style = o.widget ?
				o.widget->style () :
				QApplication::style ();

		style->drawPrimitive (QStyle::PE_PanelButtonBevel, &o, painter, o.widget);

		o.font.setBold (true);

		QStyledItemDelegate::paint (painter, o, index);

		const auto acc = index.data (CLRAccountObject).value<IAccount*> ();
		const auto extAcc = qobject_cast<IExtSelfInfoAccount*> (acc->GetQObject ());

		QIcon accIcon = extAcc ? extAcc->GetAccountIcon () : QIcon ();
		if (accIcon.isNull ())
			accIcon = qobject_cast<IProtocol*> (acc->GetParentProtocol ())->GetProtocolIcon ();

		const QRect& r = o.rect;
		const int iconSize = r.height () - 2 * CPadding;

		QImage avatarImg;
		if (extAcc && extAcc->GetSelfContact ())
		{
			const auto& id = qobject_ref_cast<ICLEntry> (extAcc->GetSelfContact ()).GetGlobalStrongestId ();
			avatarImg = RequestAvatar (Avatars_, id, View_, index, iconSize);
		}
		else
			avatarImg = ResourcesManager::Instance ().GetDefaultAvatar (iconSize);

		auto pxDraw = r.topRight () - QPoint (CPadding, 0);

		pxDraw.rx () -= avatarImg.width ();
		const QPoint delta { 0, (r.height () - avatarImg.height ()) / 2 };
		painter->drawPixmap (pxDraw + delta, QPixmap::fromImage (avatarImg));
		pxDraw.rx () -= CPadding;

		if (!accIcon.isNull ())
		{
			const int size = std::min (16, iconSize);
			const QPixmap& px = accIcon.pixmap (size, size);
			pxDraw.rx () -= px.width ();
			const QPoint delta { 0, (r.height () - px.height ()) / 2 };
			painter->drawPixmap (pxDraw + delta, px);
		}
	}

	namespace
	{
		std::pair<int, int> GetCounts (const QModelIndex& index)
		{
			const int visibleCount = index.model ()->rowCount (index);

			auto model = index.model ();
			auto sourceIndex = index;
			while (auto proxyModel = qobject_cast<const QAbstractProxyModel*> (model))
			{
				model = proxyModel->sourceModel ();
				sourceIndex = proxyModel->mapToSource (sourceIndex);
			}

			if (model->rowCount (sourceIndex) != visibleCount)
				return { visibleCount, model->rowCount (sourceIndex) };

			const auto numOnline = index.data (CLRNumOnline).toInt ();

			return { numOnline, visibleCount };
		}
	}

	void ContactListDelegate::DrawCategory (QPainter *painter,
			QStyleOptionViewItem o, const QModelIndex& index) const
	{
		const QRect& r = o.rect;

		auto style = o.widget ?
				o.widget->style () :
				QApplication::style ();

		if (HighlightGroups_)
			o.features |= QStyleOptionViewItem::Alternate;

		style->drawPrimitive (QStyle::PE_PanelItemViewRow, &o, painter, o.widget);

		if (o.state & QStyle::State_Selected)
			painter->setPen (o.palette.color (QPalette::HighlightedText));

		const int unread = index.data (CLRUnreadMsgCount).toInt ();
		if (unread)
		{
			const QString& text = QString (" %1 :: ").arg (unread);

			QFont unreadFont = o.font;
			unreadFont.setBold (true);

			int unreadSpace = CPadding + QFontMetrics { unreadFont }.horizontalAdvance (text);

			painter->setFont (unreadFont);
			painter->drawText (r.left () + CPadding, r.top (),
					unreadSpace, r.height (),
					Qt::AlignVCenter | Qt::AlignLeft,
					text);
			painter->setFont (o.font);

			o.rect.setLeft (unreadSpace + o.rect.left ());
		}

		const auto& groupName = index.data ().toString ();
		painter->drawText (r, Qt::AlignVCenter | Qt::AlignLeft, groupName);
		const int textWidth = o.fontMetrics.horizontalAdvance (groupName);
		const int rem = r.width () - textWidth;

		const auto& counts = GetCounts (index);

		const QString& str = QString (" %1/%2 ")
				.arg (counts.first)
				.arg (counts.second);

		if (rem >= o.fontMetrics.horizontalAdvance (str))
		{

			QFont font = painter->font ();
			font.setItalic (true);
			painter->setFont (font);
			const QRect numRect (r.left () + textWidth - 1, r.top (),
					rem - 1, r.height ());
			painter->drawText (numRect, Qt::AlignVCenter | Qt::AlignRight, str);
		}
	}

	void ContactListDelegate::DrawContact (QPainter *painter,
			QStyleOptionViewItem option, const QModelIndex& index) const
	{
		option.rect.setLeft (0);

		QObject *entryObj = index.data (CLREntryObject).value<QObject*> ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

		const bool isMUC = entry->GetEntryType () == ICLEntry::EntryType::MUC;

		QStyle *style = option.widget ?
				option.widget->style () :
				QApplication::style ();

		const QRect& r = option.rect;
		const int sHeight = r.height ();
		const int iconSize = sHeight - 2 * CPadding;
		const int clientIconSize = (iconSize > 16) ? 16 : iconSize;

		const QIcon& stateIcon = index.data (Qt::DecorationRole).value<QIcon> ();
		QString name = index.data (Qt::DisplayRole).value<QString> ();
		const QString status = entry->GetStatus ().StatusString_.replace ('\n', ' ');
		const QImage& avatarImg = ShowAvatars_ ?
				RequestAvatar (Avatars_, entry->GetGlobalStrongestId (), View_, index, iconSize) :
				QImage {};
		const int unreadNum = index.data (CLRUnreadMsgCount).toInt ();
		const QString& unreadStr = unreadNum ?
				QString (" %1 :: ").arg (unreadNum) :
				QString ();
		if (ShowStatuses_ && !status.isEmpty ())
			name += " (" + status + ")";

		const bool selected = option.state & QStyle::State_Selected;
		const QColor fgColor = selected ?
				option.palette.color (QPalette::HighlightedText) :
				option.palette.color (QPalette::Text);

		QFont unreadFont;
		int unreadSpace = 0;
		if (unreadNum)
		{
			unreadFont = option.font;
			unreadFont.setBold (true);

			unreadSpace = CPadding + QFontMetrics { unreadFont }.horizontalAdvance (unreadStr);
		}

		const int textShift = 2 * CPadding + iconSize + unreadSpace;

		const QStringList& vars = entry->Variants ();

		const auto& clientIcons = GetContactIcons (index, entry, vars);
		const int clientsIconsWidth = clientIcons.isEmpty () ?
				0 :
				clientIcons.size () * (clientIconSize + CPadding);
		/* text for width is total width minus shift of the text from
		 * the left (textShift) minus space for avatar (if present) with
		 * paddings minus space for client icons and paddings between
		 * them: there are N-1 paddings inbetween if there are N icons.
		 */
		const int textWidth = r.width () - textShift -
				(isMUC || !ShowAvatars_ ? 0 : (iconSize + 2 * CPadding)) -
				clientsIconsWidth;

		if (selected ||
				(option.state & QStyle::State_MouseOver))
			style->drawPrimitive (QStyle::PE_PanelItemViewItem,
					&option, painter, option.widget);

		painter->setPen (fgColor);

		painter->translate (option.rect.topLeft ());
		if (unreadNum)
		{
			painter->setFont (unreadFont);
			painter->drawText (textShift - unreadSpace, CPadding,
					textWidth, r.height () - 2 * CPadding,
					Qt::AlignVCenter | Qt::AlignLeft,
					unreadStr);
			painter->setFont (option.font);
		}

		painter->drawText (textShift, CPadding,
				textWidth, r.height () - 2 * CPadding,
				Qt::AlignVCenter | Qt::AlignLeft,
				option.fontMetrics.elidedText (name, Qt::ElideRight, textWidth));

		stateIcon.paint (painter, CPadding, CPadding, iconSize, iconSize);

		if (!avatarImg.isNull ())
			painter->drawPixmap (QPoint (textShift + textWidth + clientsIconsWidth + CPadding, CPadding),
					QPixmap::fromImage (avatarImg));

		int currentShift = textShift + textWidth + CPadding;

		for (const auto& icon : clientIcons)
		{
			icon.paint (painter, currentShift, (sHeight - clientIconSize) / 2, clientIconSize, clientIconSize);
			currentShift += clientIconSize + CPadding;
		}
	}

	namespace
	{
		QIcon LoadIcon (QHash<QString, QIcon>& cache,
				ResourcesManager::LoaderType loaderType,
				const QString& iconset,
				const QString& iconName)
		{
			const auto& fullName = iconset + '/' + iconName;
			auto pos = cache.find (fullName);
			if (pos == cache.end ())
			{
				const auto loader = ResourcesManager::Instance ().GetResourceLoader (loaderType);
				pos = cache.insert (fullName, QIcon { loader->GetIconPath (fullName) });
			}
			return *pos;
		}
	}

	QList<QIcon> ContactListDelegate::GetContactIcons (const QModelIndex& index,
			ICLEntry *entry, const QStringList& vars) const
	{
		QList<QIcon> clientIcons;
		clientIcons.reserve (vars.size () + 4);

		if (entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
		{
			const auto& aff = index.data (CLRAffiliation).toByteArray ();
			clientIcons.prepend (ResourcesManager::Instance ().GetAffIcon (aff));
		}

		if (!vars.isEmpty ())
		{
			if (const auto ihca = qobject_cast<IHaveContactActivity*> (entry->GetQObject ()))
			{
				QString iconName;
				for (const auto& var : vars)
				{
					const auto& info = ihca->GetUserActivity (var);
					iconName = GetActivityIconName (info.General_, info.Specific_);
					if (!iconName.isEmpty ())
						break;
				}

				if (!iconName.isEmpty ())
					clientIcons << LoadIcon (ActivityIconCache_, ResourcesManager::RLTActivityIconLoader, ActivityIconset_, iconName);
			}
			if (const auto ihcm = qobject_cast<IHaveContactMood*> (entry->GetQObject ()))
			{
				QString iconName;
				for (const auto& var : vars)
				{
					iconName = ihcm->GetUserMood (var).Mood_;
					if (!iconName.isEmpty ())
						break;
				}
				if (!iconName.isEmpty ())
					clientIcons << LoadIcon (MoodIconCache_, ResourcesManager::RLTMoodIconLoader, MoodIconset_, iconName);
			}
			if (const auto ihct = qobject_cast<IHaveContactTune*> (entry->GetQObject ()))
			{
				if (std::ranges::any_of (vars,
						[ihct] (const QString& var)
						{
							const auto& info = ihct->GetUserTune (var);
							return !info.Artist_.isEmpty () || !info.Album_.isEmpty ();
						}))
					clientIcons << LoadIcon (SystemIconCache_, ResourcesManager::RLTSystemIconLoader, SystemIconset_, "notification_roster_tune"_qs);
			}

			if (auto geoloc = qobject_cast<ISupportGeolocation*> (entry->GetParentAccount ()->GetQObject ()))
			{
				if (const auto& info = geoloc->GetUserGeolocationInfo (entry->GetQObject (), vars.value (0));
					!info.isEmpty ())
					clientIcons << LoadIcon (SystemIconCache_, ResourcesManager::RLTSystemIconLoader, SystemIconset_, "geolocation"_qs);
			}
		}

		const bool isMUC = entry->GetEntryType () == ICLEntry::EntryType::MUC;
		if (!isMUC && ShowClientIcons_)
		{
			const auto& iconsMap = ResourcesManager::Instance ().GetClientIconForEntry (entry);
			for (int i = 0; i < std::min (static_cast<int> (vars.size ()), 4); ++i)
				clientIcons << iconsMap [vars.at (i)];
		}

		emit hookCollectContactIcons (IHookProxy_ptr { new Util::DefaultHookProxy },
				entry->GetQObject (), clientIcons);

		clientIcons.removeIf (std::mem_fn (&QIcon::isNull));

		return clientIcons;
	}
}
}
