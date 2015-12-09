#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include "qdownloader_global.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

class QDOWNLOADER_EXPORT DownloadTask : public QObject
{
	Q_OBJECT

public:
	enum State
	{
		Unvalid,
		Init,
		Start,
		Pause,
		Finish,
		Queue,
		Cancel,
		Error,
	};

	DownloadTask(QObject *parent);
	~DownloadTask();

	void init(const QString &savePath, const QString &url, QNetworkAccessManager *netMgr);
	void load(const QString &path, const QString &infFile, QNetworkAccessManager *netMgr);
	void start();
	void pause();
	void cancel();

	void setUrl(const QString &url);
	QString url() const;
	void setName(const QString &name);
	QString name() const;
	void setPath(const QString &path);
	QString path() const;
	QString fullName() const;
	void setSize(qint64 size);
	qint64 size() const;
	void setProgress(qint64 progress);
	qint64 progress() const;
	void setState(State state);
	State state() const;

signals:
	void inited();
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal, qint64 bytesPerSecond);
	void finished();
	void error(QNetworkReply::NetworkError code);

private slots:
	void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void onFinished();
	void onError(QNetworkReply::NetworkError code);

private:
	static const QString INFO_FILE_EXT;
	static const QString TASK_FILE_EXT;

	class SpeedTest
	{
	public:
		SpeedTest();

		void reset();
		qint64 bytesPerSecond(qint64 currentMSecsSinceEpoch);

		qint64 currentMSecsSinceEpoch_;
		qint64 lastMSecsSinceEpoch_;
		qint64 downloadBytes_;
	};

	void splitName();
	QString infoFileFullName();
	QString taskFileFullName();

	QString url_;
	QString name_;
	QString path_;
	qint64 size_;
	qint64 progress_;
	State state_;
	QNetworkAccessManager *networkMgr_;
	SpeedTest speedTest_;
	QFile infoFile_;
	QFile taskFile_;
};

#endif // DOWNLOADTASK_H
