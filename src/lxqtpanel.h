#ifndef LXQTPANEL_H
#define LXQTPANEL_H

#include <QString>

class ILXQtPanel;

class LXQtPanel
{
public:
    LXQtPanel(ILXQtPanel *panel);

    int size() const;
    QString orient() const;

    int iconSize() const;
    bool isHorizontal() const;

private:
    ILXQtPanel *panel;
};

#endif // LXQTPANEL_H
