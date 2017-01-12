/***************************************************************************
 *   Copyright (C) 2016 by Jeremy Whiting <jpwhiting@kde.org>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License.        *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef PROJECT_H
#define PROJECT_H

#include "nametable.h"
#include "tileset.h"
#include "palette.h"

class QSettings;

/**
 * @brief The Project class encapsulates a workspace project
 *
 * A project includes two palettes (sprites and background), a tileset
 * and a number of nametables with filenames.
 * A project is saved to disk in a human readable format so it can be easily
 * edited by hand if needed. The format is an ini file so we can use QSettings
 * to easily parse it and create the gui components required
 */
class Project: public QObject
{
    Q_OBJECT
public:
    Project();
    ~Project();

    void load(QString &filename);
    void saveAs(QString &filename);

    void setBackgroundPaletteFilename(QString &filename);
    void setSpritesPaletteFilename(QString &filename);
    void setTileSetFilename(QString &filename);
    void setNameTables(QStringList &filenames);

signals:
    void loadBackgroundPalette(QString filename);
    void loadSpritesPalette(QString filename);
    void loadTileset(QString filename);
    void loadNameTables(QStringList filenames);

private:
    QSettings *mSettings;
    QString mFilename;
};

#endif // PROJECT_H
