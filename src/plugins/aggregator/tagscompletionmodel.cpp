#include <QLineEdit>
#include <QtDebug>
#include "tagscompletionmodel.h"

TagsCompletionModel::TagsCompletionModel (QObject *parent)
: QAbstractItemModel (parent)
{
}

int TagsCompletionModel::columnCount (const QModelIndex& parent) const
{
    return 1;
}

QVariant TagsCompletionModel::data (const QModelIndex& index, int role) const
{
    if (!index.isValid ())
        return QVariant ();

    if (role == Qt::EditRole || role == Qt::DisplayRole)
        return Tags_.at (index.row ());
    else
        return QVariant ();
}

Qt::ItemFlags TagsCompletionModel::flags (const QModelIndex& index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool TagsCompletionModel::hasChildren (const QModelIndex& index) const
{
    return !index.isValid ();
}

QVariant TagsCompletionModel::headerData (int, Qt::Orientation, int role) const
{
    return QVariant ();
}

QModelIndex TagsCompletionModel::index (int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex (row, column, parent))
        return QModelIndex ();

    return createIndex (row, column);
}

QModelIndex TagsCompletionModel::parent (const QModelIndex&) const
{
    return QModelIndex ();
}

int TagsCompletionModel::rowCount (const QModelIndex&) const
{
    return Tags_.size ();
}

void TagsCompletionModel::UpdateTags (const QStringList& newTags)
{
    for (int i = 0; i < newTags.size (); ++i)
        if (!Tags_.contains (newTags.at (i)))
            Tags_.append (newTags.at (i));
}

QStringList TagsCompletionModel::GetTags () const
{
    return Tags_;
}

void TagsCompletionModel::SetLineEdit (const QLineEdit* edit)
{
    LineEdit_ = edit;
}

