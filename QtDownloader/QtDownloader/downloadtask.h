#ifndef DOWNLOADTASK_H
#define DOWNLOADTASK_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class DownloadTask : public QObject
{
	Q_OBJECT

public:
	enum State
	{
		Unvalid,
		Start,
		Pause,
		Finish,
		Queue,
		Cancel,
		Error,
	};

	DownloadTask(QObject *parent, QNetworkAccessManager *netMgr);
	~DownloadTask();

	void start();
	void start(const QString &url);
	void pause();
	void cancel();

	void setUrl(const QString &url);
	QString url() const;
	void setName(const QString &name);
	QString name() const;
	void setPath(const QString &path);
	QString path() const;
	QString fullName() const;
	void setSize(quint64 size);
	quint64 size() const;
	void setProgress(quint64 progress);
	quint64 progress() const;
	void setState(State state);
	State state() const;

private slots:
	void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void onFinished();
	void onError(QNetworkReply::NetworkError code);

private:
	static const QString INFO_FILE_EXT;
	static const QString TASK_FILE_EXT;

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
};

#endif // DOWNLOADTASK_H
