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
#ifndef THEMEPARSER_H
#define THEMEPARSER_H

#include <QString>
#include <QColor>

class ThemeParser {
public:
    ThemeParser(const QString &qss);

    QColor getBackgroundColor();
    QString getLinearGradient();

    static QString colorToString(const QColor &c);

private:
    void parseBackgroundRule(const QString &qss);
    bool parseHexNamedColor(const QString &value, QColor &color);
    bool parsePaletteColor(const QString &value, QColor &color);
    bool parseRgbaColor(const QString &value, QColor &color);
    bool parseRgbColor(const QString &value, QColor &color);
    bool parseLinearGradient(const QString &value, QString &linearGradient);

    bool splitArguments(const QString &text, QStringList &args);

    QColor bgColor;
    QString bgLinearGradient;
};

#endif // THEMEPARSER_H
