#ifndef PYAPPLETKEEPER_H
#define PYAPPLETKEEPER_H

#include <QProcess>

class PyAppletKeeper
{
public:
    explicit PyAppletKeeper();
    ~PyAppletKeeper();
    void setStartupArguments(const QString &orient, int size);
    void start();
    void stop();

private:
    QProcess proc;
    QStringList args;
};

#endif // PYAPPLETKEEPER_H
