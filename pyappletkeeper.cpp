#include "pyappletkeeper.h"
#include "config.h"

PyAppletKeeper::PyAppletKeeper() {
    proc.setProgram(QStringLiteral(u"%1/lxqt-panel-plugin.py").arg(DOCKBARX_PATH));
    proc.setWorkingDirectory(DOCKBARX_PATH);
}

PyAppletKeeper::~PyAppletKeeper() {
    stop();
}

void PyAppletKeeper::setStartupArguments(const QString &orient, int size) {
    args.clear();
    args << QStringLiteral(u"-o") << orient;
    args << QStringLiteral(u"-s") << QString::number(size);
}

void PyAppletKeeper::start() {
    QObject::connect(&proc, static_cast<void(QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), &proc, [this](){
        proc.setArguments(args);
        proc.start();
    });
    proc.setArguments(args);
    proc.start();
}

void PyAppletKeeper::stop() {
    proc.disconnect();
    proc.kill();
    proc.waitForFinished();
}
