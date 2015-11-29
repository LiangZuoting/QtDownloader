#include "downloadtask.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QFile>

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
	QString infoFile = infoFileFullName();
	QFile fileInfo(infoFile);
	if (fileInfo.open(QFile::ReadOnly)
		&& fileInfo.read((char*)&size_, sizeof(size_)) == sizeof(size_)
		&& fileInfo.read((char*)&progress_, sizeof(progress_) == sizeof(progress_)))
	{
		fileInfo.close();
	} 
	else
	{
		if (fileInfo.isOpen())
		{
			fileInfo.close();
		}
		fileInfo.open(QFile::ReadWrite | QFile::Truncate);
		fileInfo.close();
	}
	QNetworkRequest req(url_);
	QString range("bytes=%1-");
	range = range.arg(progress_);
	req.setRawHeader("Range", range.toUtf8());
	QNetworkReply *reply = networkMgr_->get(req);
	connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
	connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
}

void DownloadTask::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	QNetworkReply *reply = reinterpret_cast<QNetworkReply*>(sender());
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
	result += "." + INFO_FILE_EXT;
	return result;
}