#ifndef PLUGINLISTTABLEWIDGETICON_H
#define PLUGINLISTTABLEWIDGETICON_H
#include <QTableWidgetItem>

class QIcon;
class StatusBarMessage_;

class PluginListTableWidgetIcon : public QTableWidgetItem
{
    QString *Text_;
    QIcon *Icon_;
    QString *WhatsThisMessage_;
    QString *TooltipMessage_;
    QString *StatusbarMessage_;
public:
    PluginListTableWidgetIcon ();
    virtual ~PluginListTableWidgetIcon ();
    
    virtual QVariant data (int) const;
    virtual void setData (int, const QVariant&);
};

#endif

