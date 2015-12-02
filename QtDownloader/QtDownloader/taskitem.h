#ifndef TASKITEM_H
#define TASKITEM_H

#include <QStyledItemDelegate>

class TaskItemData : public QObject
{
	Q_OBJECT
public:
	TaskItemData(QObject *parent = nullptr);
	~TaskItemData();

	QString name;
	qint64 progress;
	qint64 size;
	qint64 bytesPerSecond;
};

class TaskItem : public QStyledItemDelegate
{
	Q_OBJECT

public:
	TaskItem(QObject *parent);
	~TaskItem();

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

};

#endif // TASKITEM_H
