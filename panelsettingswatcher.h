#ifndef PANELSETTINGSWATCHER_H
#define PANELSETTINGSWATCHER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>

class PanelSettingsWatcher : public QObject
{
    Q_OBJECT
public:
    explicit PanelSettingsWatcher(const QString &file, QObject *parent = nullptr);
    ~PanelSettingsWatcher();
    void stop();

public slots:
    void start();

signals:
    void modified();

private slots:
    void checkFile();

private:
    void startTimer();
    volatile bool run = false;
    int fd;
    QString filePath;
    QTimer *timer = nullptr;
    QDateTime mtime;
};

#endif // PANELSETTINGSWATCHER_H
