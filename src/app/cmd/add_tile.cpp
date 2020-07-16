// Aseprite
// Copyright (C) 2019-2020  Igara Studio S.A.
//
// This program is distributed under the terms of
// the End-User License Agreement for Aseprite.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/add_tile.h"

#include "app/doc.h"
#include "doc/image_io.h"
#include "doc/sprite.h"
#include "doc/tileset.h"
#include "doc/tilesets.h"

namespace app {
namespace cmd {

using namespace doc;

AddTile::AddTile(doc::Tileset* tileset,
                 const doc::ImageRef& image)
  : WithTileset(tileset)
  , WithImage(image.get())
  , m_size(0)
  , m_tileIndex(doc::tile_i_notile)
  , m_imageRef(image)
{
}

AddTile::AddTile(doc::Tileset* tileset,
                 const doc::tile_index ti)
  : WithTileset(tileset)
  , WithImage(tileset->get(ti).get())
  , m_size(0)
  , m_tileIndex(ti)
  , m_imageRef(nullptr)
{
}

void AddTile::onExecute()
{
  if (m_tileIndex != doc::tile_i_notile) {
    ASSERT(!m_imageRef);
    return;
  }

  doc::Tileset* tileset = this->tileset();
  ASSERT(tileset);
  ASSERT(m_imageRef);

  addTile(tileset, m_imageRef);

  m_imageRef.reset();
}

void AddTile::onUndo()
{
  doc::Tileset* tileset = this->tileset();
  ASSERT(tileset);

  write_image(m_stream, image());
  m_size = size_t(m_stream.tellp());

  tileset->erase(m_tileIndex);

  tileset->sprite()->incrementVersion();
  tileset->incrementVersion();
}

void AddTile::onRedo()
{
  doc::Tileset* tileset = this->tileset();

  ASSERT(!m_imageRef);
  m_imageRef.reset(read_image(m_stream));
  ASSERT(m_imageRef);

  addTile(tileset, m_imageRef);
  m_imageRef.reset();

  m_stream.str(std::string());
  m_stream.clear();
  m_size = 0;
}

void AddTile::onFireNotifications()
{
  doc::Tileset* tileset = this->tileset();

  // Notify that the tileset's changed
  static_cast<Doc*>(tileset->sprite()->document())
    ->notifyTilesetChanged(tileset);
}

void AddTile::addTile(doc::Tileset* tileset, const doc::ImageRef& image)
{
  if (m_tileIndex == doc::tile_i_notile)
    m_tileIndex = tileset->add(image);
  else
    tileset->insert(m_tileIndex, image);

  tileset->sprite()->incrementVersion();
  tileset->incrementVersion();
}

} // namespace cmd
} // namespace app
