#include "downloadtask.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QFile>
#include <QFileInfo>

const QString DownloadTask::INFO_FILE_EXT("inf");
const QString DownloadTask::TASK_FILE_EXT("tsk");

DownloadTask::DownloadTask(QObject *parent, QNetworkAccessManager *netMgr)
	: QObject(parent)
	, size_(0)
	, progress_(0)
	, state_(Unvalid)
	, networkMgr_(netMgr)
{}

DownloadTask::~DownloadTask()
{}

void DownloadTask::start()
{
	if (url_.isEmpty())
	{
		qDebug() << __FUNCTION__ << " " << __LINE__ << "url is empty.";
		return;
	}
	if (name_.isEmpty())
	{
		splitName();
	}
	
	QNetworkReply *reply = networkMgr_->get(QNetworkRequest(url_));
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
	connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
}

void DownloadTask::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	QNetworkReply *reply = reinterpret_cast<QNetworkReply*>(sender());
	reply->deleteLater();
	if (reply->error() != QNetworkReply::NoError)
	{
		qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << reply->error();
		return;
	}
	QFileInfo fi(infoFileFullName());
	qint64 bytesWritten = 0;
	if (!fi.exists())
	{
		size_ = reply->header(QNetworkRequest::ContentLengthHeader).toLongLong();
		QFile taskFile(taskFileFullName());
		if (!taskFile.open(QFile::ReadWrite))
		{
			qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << taskFile.error();
			return;
		}
		if (!taskFile.resize(size_))
		{
			qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << taskFile.error();
			return;
		}
		QFile infoFile(infoFileFullName());
		if (!infoFile.open(QFile::ReadWrite))
		{
			qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << infoFile.error();
			return;
		}
		bytesWritten = infoFile.write((char*)&size_, sizeof(size_));
		if (bytesWritten != sizeof(size_))
		{
			qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << infoFile.error();
			return;
		}
	}
	QFile taskFile(taskFileFullName());
	if (!taskFile.open(QFile::ReadWrite))
	{
		qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << taskFile.error();
		return;
	}
	qint64 size = reply->size();
	bytesWritten = taskFile.write(reply->readAll());
	if (bytesWritten != size)
	{
		qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << taskFile.error();
		return;
	}
	progress_ += size;

	QFile infoFile(infoFileFullName());
	if (!infoFile.open(QFile::ReadWrite))
	{
		qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << infoFile.error();
		return;
	}
	if (!infoFile.seek(sizeof(size_)))
	{
		qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << infoFile.error();
		return;
	}
	bytesWritten = infoFile.write((char*)&progress_, sizeof(progress_));
	if (bytesWritten != sizeof(progress_))
	{
		qDebug() << __FUNCTIONW__ << " " << __LINE__ << " error : " << infoFile.error();
		return;
	}

	auto data = reply->readAll();
	//write files.
}

void DownloadTask::onFinished()
{

}

void DownloadTask::splitName()
{
	if (url_.isEmpty())
	{
		qDebug() << __FUNCTION__ << " " << __LINE__ << "url is empty.";
		return;
	}
	int index = url_.lastIndexOf('/');
	if (index == -1)
	{
		qDebug() << __FUNCTION__ << " " << __LINE__ << "url is wrong.";
		return;
	}
	name_ = url_.right(url_.size() - index -1);
}

QString DownloadTask::infoFileFullName()
{
	QString result = path_;
	if (!result.endsWith('\\'))
	{
		result += "\\";
	}
	result += name_ + "." + INFO_FILE_EXT;
	return result;
}

QString DownloadTask::taskFileFullName()
{
	QString result = path_;
	if (!result.endsWith('\\'))
	{
		result += "\\";
	}
	result += name_ + "." + TASK_FILE_EXT;
	return result;
}