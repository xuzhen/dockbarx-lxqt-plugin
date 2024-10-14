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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtLogging>
#else
#include <QDebug>
#endif

static const QRegularExpression spacesRe = QRegularExpression(QStringLiteral(u"\\s+"));

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

QString ThemeParser::getLinearGradient() {
    return bgLinearGradient;
}

QString ThemeParser::colorToString(const QColor &c) {
    return QStringLiteral(u"rgba(%1,%2,%3,%4)").arg(c.red()).arg(c.green()).arg(c.blue()).arg(c.alphaF());
}

void ThemeParser::parseBackgroundRule(const QString &qss) {
    const QStringList lines = qss.split(QChar(';'), Qt::SkipEmptyParts);
    for (const QString &l : lines) {
        QString line = l.trimmed().replace(spacesRe, QChar(' '));
        if (line.startsWith(QStringLiteral(u"background-color:"), Qt::CaseInsensitive) || line.startsWith(QStringLiteral(u"background:"), Qt::CaseInsensitive)) {
            QString value = line.mid(line.indexOf(QChar(':')) + 1).trimmed();
            if (parseHexNamedColor(value, bgColor)) {
                bgLinearGradient.clear();
            } else if (parsePaletteColor(value, bgColor)) {
                bgLinearGradient.clear();
            } else if (parseRgbaColor(value, bgColor)) {
                bgLinearGradient.clear();
            } else if (parseRgbColor(value, bgColor)) {
                bgLinearGradient.clear();
            } else if (parseLinearGradient(value, bgLinearGradient)) {
                bgColor = QColor();
            } else {
                qWarning() << "Unsupported background:" << value;
            }
        }
    }
}

bool ThemeParser::parseHexNamedColor(const QString &value, QColor &color) {
    QColor c(value);
    if (c.isValid()) {
        color = c;
        return true;
    }
    return false;
}

bool ThemeParser::parsePaletteColor(const QString &value, QColor &color) {
    static const QRegularExpression re = QRegularExpression(QStringLiteral(u"^palette\\((alternate-base|accent|base|bright-text|button|button-text|dark|highlight|highlighted-text|light|link|link-visited|mid|midlight|shadow|text|window|window-text)\\)$"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(value);
    if (match.hasMatch() == false) {
        return false;
    }
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
    color = qApp->palette().color(map.value(match.captured(1).toLower()));
    return true;
}

bool ThemeParser::parseRgbaColor(const QString &value, QColor &color) {
    static const QRegularExpression re = QRegularExpression(QStringLiteral(u"^rgba\\(([^\\)]+)\\)$"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(value);
    if (match.hasMatch() == false) {
        return false;
    }
    QStringList args;
    if (splitArguments(match.captured(1), args) == false) {
        return false;
    }
    if (args.size() != 4) {
        return false;
    }
    int r, g, b;
    double a;
    bool ok[4];
    r = args[0].toInt(&ok[0]);
    g = args[1].toInt(&ok[1]);
    b = args[2].toInt(&ok[2]);
    if (args[3].endsWith(QChar('%'))) {
        a = args[3].removeLast().toDouble(&ok[3]) / 100;
    } else {
        a = args[3].toDouble(&ok[3]);
    }
    if (ok[0] && ok[1] && ok[2] && ok[3]) {
        color.setRgb(r, g, b);
        color.setAlphaF(a);
        return true;
    } else {
        return false;
    }
}

bool ThemeParser::parseRgbColor(const QString &value, QColor &color) {
    static const QRegularExpression re = QRegularExpression(QStringLiteral(u"^rgb\\(([^\\)]+)\\)$"), QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(value);
    if (match.hasMatch() == false) {
        return false;
    }
    QStringList args;
    if (splitArguments(match.captured(1), args) == false) {
        return false;
    }
    if (args.size() != 3) {
        return false;
    }
    int r, g, b;
    bool ok[3];
    r = args[0].toInt(&ok[0]);
    g = args[1].toInt(&ok[1]);
    b = args[2].toInt(&ok[2]);
    if (ok[0] && ok[1] && ok[2]) {
        color.setRgb(r, g, b);
        return true;
    } else {
        return false;
    }
}

bool ThemeParser::parseLinearGradient(const QString &value, QString &linearGradient) {
    static const QRegularExpression re = QRegularExpression(QStringLiteral(u"^qlineargradient\\((.+)\\)$"), QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatch match = re.match(value);
    if (match.hasMatch() == false) {
        return false;
    }
    QStringList args;
    if (splitArguments(match.captured(1), args) == false) {
        return false;
    }
    int pos = 0;
    QJsonArray stops;
    QJsonObject json;
    for (const QString &p: args) {
        QStringList pair = p.split(QChar(':'));
        if (pair.size() != 2) {
            return false;
        }
        QString k = pair[0].trimmed().toLower();
        QString v = pair[1].trimmed();
        if (k == QStringLiteral(u"stop")) {
            QStringList stopArgs = v.replace(spacesRe, QChar(' ')).split(QChar(' '));
            QString stopPos = stopArgs.takeFirst().trimmed();
            QString stopColor = stopArgs.join(QChar(' ')).trimmed();
            bool ok;
            double d = stopPos.toDouble(&ok);
            if (!ok) {
                return false;
            }
            QColor tc;
            if (parseRgbaColor(stopColor, tc) || parseRgbColor(stopColor, tc) || parsePaletteColor(stopColor, tc)) {
                stopColor = colorToString(tc);
            }
            QJsonArray stopPair;
            stopPair.append(d);
            stopPair.append(stopColor);
            stops.append(stopPair);
        } else {
            bool ok = false;
            double dv;
            if (k == QStringLiteral(u"x1")) {
                dv = v.toDouble(&ok);
                pos |= 1;
            } else if (k == QStringLiteral(u"y1")) {
                dv = v.toDouble(&ok);
                pos |= 2;
            } else if (k == QStringLiteral(u"x2")) {
                dv = v.toDouble(&ok);
                pos |= 4;
            } else if (k == QStringLiteral(u"y2")) {
                dv = v.toDouble(&ok);
                pos |= 8;
            }
            if (!ok) {
                return false;
            }
            json.insert(k, dv);
        }
    }
    if (pos != 15 || stops.size() == 0) {
        return false;
    }
    json.insert(QStringLiteral(u"stops"), stops);
    linearGradient = QJsonDocument(json).toJson(QJsonDocument::Compact);
    return true;
}

bool ThemeParser::splitArguments(const QString &text, QStringList &args) {
    QChar comma(','), lp('('), rp(')');
    args.clear();
    int pi = 0;
    int depth = 0;
    for (int i = 0; i < text.length(); i++) {
        QChar c = text.at(i);
        if (c == comma) {
            if (depth == 0) {
                args.append(text.mid(pi, i - pi));
                pi = i + 1;
            }
        } else if (c == lp) {
            depth++;
        } else if (c == rp) {
            depth--;
            if (depth < 0) {
                return false;
            }
        }
    }
    if (depth != 0) {
        return false;
    }
    args.append(text.mid(pi));
    return true;
}