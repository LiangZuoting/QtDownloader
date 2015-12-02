#include "taskitem.h"
#include <QPainter>

TaskItemData::TaskItemData(QObject *parent /* = nullptr */)
	: QObject(parent)
	, progress(0)
	, size(0)
	, bytesPerSecond(0)
{}

TaskItemData::~TaskItemData()
{}

TaskItem::TaskItem(QObject *parent)
	: QStyledItemDelegate(parent)
{

}

TaskItem::~TaskItem()
{
}

void TaskItem::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	TaskItemData *data = reinterpret_cast<TaskItemData*>(index.data(Qt::DisplayRole).toInt());
	painter->fillRect(option.rect, Qt::red);
	painter->drawText(option.rect, "speed : " + QString::number(data->bytesPerSecond));
}