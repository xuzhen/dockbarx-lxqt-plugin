/*
 Copyright (C) 2024 Xu Zhen

 This file is part of DockbarX LXQt panel plugin.

 DockbarX LXQt panel plugin is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or (at your
 option) any later version.

 DockbarX LXQt panel plugin is distributed in the hope that it will be
 useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this file. If not, see <http://www.gnu.org/licenses/>.
*/

#include "themeparser.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QColor>
#include <QPalette>
#include <QHash>
#include <lxqt/LXQt/lxqtapplication.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtLogging>
#else
#include <QDebug>
#endif

static const QRegularExpression spacesRe = QRegularExpression(QStringLiteral(u"\\s+"));

static const QRegularExpression paletteRe = QRegularExpression(QStringLiteral(u"^palette\\((alternate-base|accent|base|bright-text|button|button-text|dark|highlight|highlighted-text|light|link|link-visited|mid|midlight|shadow|text|window|window-text)\\)$"));
static const QRegularExpression rgbaRe = QRegularExpression(QStringLiteral(u"^rgba\\(([^\\)]+)\\)$"));
static const QRegularExpression rgbRe = QRegularExpression(QStringLiteral(u"^rgb\\(([^\\)]+)\\)$"));

ThemeParser::ThemeParser(const QString &qss) {
    static const QRegularExpression bgRe("LXQtPanel\\s+#BackgroundWidget\\s*{([^}]*)}");
    QRegularExpressionMatch match = bgRe.match(qss);
    if (match.hasMatch()) {
        parseBackgroundRule(match.captured(1));
    }
}

QColor ThemeParser::getBackgroundColor() {
    return bgColor;
}

void ThemeParser::parseBackgroundRule(const QString &qss) {
    const QStringList lines = qss.split(QChar(';'), Qt::SkipEmptyParts);
    for (const QString &l : lines) {
        QString line = l.trimmed().replace(spacesRe, QChar(' '));
        if (line.startsWith(QStringLiteral(u"background-color:")) || line.startsWith(QStringLiteral(u"background:"))) {
            QString value = line.mid(line.indexOf(QChar(':')) + 1).trimmed();
            QColor c(value);
            QRegularExpressionMatch match;
            if (c.isValid()) {
                bgColor = c;
            } else if ((match = paletteRe.match(value)).hasMatch()) {
                bgColor = getPaletteColor(match.captured(1));
            } else if ((match = rgbaRe.match(value)).hasMatch()) {
                bgColor = getRgbaColor(match.captured(1));
            } else if ((match = rgbRe.match(value)).hasMatch()) {
                bgColor = getRgbColor(match.captured(1));
            } else {
                qWarning() << "Unsupported background:" << value;
                bgColor = QColor(0, 0, 0, 0);
            }
        }
    }
}

QColor ThemeParser::getPaletteColor(const QString &role) {
    static QHash<QString, QPalette::ColorRole> map;
    if (map.isEmpty()) {
        map.insert(QStringLiteral(u"alternate-base"),   QPalette::AlternateBase);
        map.insert(QStringLiteral(u"accent"),           QPalette::Accent);
        map.insert(QStringLiteral(u"base"),             QPalette::Base);
        map.insert(QStringLiteral(u"bright-text"),      QPalette::BrightText);
        map.insert(QStringLiteral(u"button"),           QPalette::Button);
        map.insert(QStringLiteral(u"button-text"),      QPalette::ButtonText);
        map.insert(QStringLiteral(u"dark"),             QPalette::Dark);
        map.insert(QStringLiteral(u"highlight"),        QPalette::Highlight);
        map.insert(QStringLiteral(u"highlighted-text"), QPalette::HighlightedText);
        map.insert(QStringLiteral(u"light"),            QPalette::Light);
        map.insert(QStringLiteral(u"link"),             QPalette::Link);
        map.insert(QStringLiteral(u"link-visited"),     QPalette::LinkVisited);
        map.insert(QStringLiteral(u"mid"),              QPalette::Mid);
        map.insert(QStringLiteral(u"midlight"),         QPalette::Midlight);
        map.insert(QStringLiteral(u"shadow"),           QPalette::Shadow);
        map.insert(QStringLiteral(u"text"),             QPalette::Text);
        map.insert(QStringLiteral(u"window"),           QPalette::Window);
        map.insert(QStringLiteral(u"window-text"),      QPalette::WindowText);
    }
    return lxqtApp->palette().color(map.value(role));
}

QColor ThemeParser::getRgbaColor(const QString &rgba) {
    QStringList parts = rgba.split(QChar(','));
    if (parts.size() != 4) {
        return QColor();
    }
    int r, g, b;
    double a;
    bool ok[4];
    r = parts[0].toInt(&ok[0]);
    g = parts[1].toInt(&ok[1]);
    b = parts[2].toInt(&ok[2]);
    if (parts[3].endsWith(QChar('%'))) {
        a = parts[3].removeLast().toDouble(&ok[3]) / 100;
    } else {
        a = parts[3].toDouble(&ok[3]);
    }
    if (ok[0] && ok[1] && ok[2] && ok[3]) {
        QColor c(r, g, b);
        c.setAlphaF(a);
        return c;
    } else {
        return QColor();
    }
}

QColor ThemeParser::getRgbColor(const QString &rgb) {
    QStringList parts = rgb.split(QChar(','));
    if (parts.size() != 3) {
        return QColor();
    }
    int r, g, b;
    bool ok[3];
    r = parts[0].toInt(&ok[0]);
    g = parts[1].toInt(&ok[1]);
    b = parts[2].toInt(&ok[2]);
    if (ok[0] && ok[1] && ok[2]) {
        return QColor(r, g, b);
    } else {
        return QColor();
    }
}