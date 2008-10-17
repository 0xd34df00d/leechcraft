#ifndef TAGSCOMPLETIONMODEL_H
#define TAGSCOMPLETIONMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include "config.h"

class TagsCompletionModel : public QAbstractItemModel
{
    Q_OBJECT

    QStringList Tags_;
public:
    LEECHCRAFT_API TagsCompletionModel (QObject *parent = 0);

    LEECHCRAFT_API virtual int columnCount (const QModelIndex& parent = QModelIndex ()) const;
    LEECHCRAFT_API virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    LEECHCRAFT_API virtual Qt::ItemFlags flags (const QModelIndex&) const;
    LEECHCRAFT_API virtual bool hasChildren (const QModelIndex&) const;
    LEECHCRAFT_API virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    LEECHCRAFT_API virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex()) const;
    LEECHCRAFT_API virtual QModelIndex parent (const QModelIndex&) const;
    LEECHCRAFT_API virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

    LEECHCRAFT_API void UpdateTags (const QStringList&);
    LEECHCRAFT_API QStringList GetTags () const;
signals:
	LEECHCRAFT_API void tagsUpdated (const QStringList&);
};

#endif

