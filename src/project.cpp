#include "project.h"

#include <QSettings>

#define kNameTablesKey "NameTables"
#define kNameTableKey "NameTable"
#define kBGPaletteKey "BackgroundPalette"
#define kSPPaletteKey "SpritesPalette"
#define kTileSetKey "TileSet"

Project::Project()
{
    mSettings = NULL;
}

Project::~Project()
{
    delete mSettings;
}

void Project::load(QString &filename)
{
    mFilename = filename;

    if (mSettings != NULL)
        delete mSettings;

    mSettings = new QSettings(mFilename, QSettings::IniFormat);
    // Get the palettes
    QString tempFilename = mSettings->value(kBGPaletteKey, QString()).toString();
    if (!tempFilename.isEmpty())
        emit loadBackgroundPalette(tempFilename);

    tempFilename = mSettings->value(kSPPaletteKey, QString()).toString();
    if (!tempFilename.isEmpty())
        emit loadSpritesPalette(tempFilename);

    tempFilename = mSettings->value(kTileSetKey, QString()).toString();
    if (!tempFilename.isEmpty())
        emit loadTileset(tempFilename);

    QStringList nametables;
    int size = mSettings->beginReadArray(kNameTablesKey);
    for (int i = 0; i < size; ++i) {
        mSettings->setArrayIndex(i);
        nametables.append(mSettings->value(kNameTableKey).toString());
    }
    mSettings->endArray();
    if (!nametables.isEmpty())
        emit loadNameTables(nametables);
}

void Project::saveAs(QString &filename)
{
    if (mSettings == NULL) {
        mSettings = new QSettings(filename, QSettings::IniFormat);
        return;
    }
}

void Project::setBackgroundPaletteFilename(QString &filename)
{
    if (mSettings == NULL)
        return;

    mSettings->setValue(kBGPaletteKey, filename);
}

void Project::setSpritesPaletteFilename(QString &filename)
{
    if (mSettings == NULL)
        return;

    mSettings->setValue(kSPPaletteKey, filename);
}

void Project::setTileSetFilename(QString &filename)
{
    if (mSettings == NULL)
        return;

    mSettings->setValue(kTileSetKey, filename);
}

void Project::setNameTables(QStringList &filenames)
{
    if (mSettings == NULL)
        return;

    mSettings->beginWriteArray(kNameTablesKey, filenames.size());
    for (int i = 0; i < filenames.size(); ++i)
    {
        mSettings->setArrayIndex(i);
        mSettings->setValue(kNameTableKey, filenames.at(i));
    }
    mSettings->endArray();
}
