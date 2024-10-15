#include "lxqtpanel.h"
#include <lxqt/ilxqtpanel.h>

LXQtPanel::LXQtPanel(ILXQtPanel *panel) : panel(panel) {
}

int LXQtPanel::size() const {
    QRect geo = panel->globalGeometry();
    if (panel->isHorizontal()) {
        return geo.height();
    } else {
        return geo.width();
    }
}

QString LXQtPanel::orient() const {
    ILXQtPanel::Position pos = panel->position();
    switch (pos) {
    case ILXQtPanel::PositionBottom:
        return QStringLiteral(u"down");
    case ILXQtPanel::PositionTop:
        return QStringLiteral(u"up");
    case ILXQtPanel::PositionLeft:
        return QStringLiteral(u"left");
    case ILXQtPanel::PositionRight:
        return QStringLiteral(u"right");
    }
    return QString();
}

int LXQtPanel::iconSize() const {
    return std::min(panel->iconSize(), size());
}

bool LXQtPanel::isHorizontal() const {
    return panel->isHorizontal();
}
