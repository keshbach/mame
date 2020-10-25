// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

Cobra Command:
  2 BAC06 background generator chips, same as Dec0.
  1 MXC06 chip for sprites, same as Dec0.
  256 colours, palette generated by ram.

The Real Ghostbusters:
1 Deco VSC30 (M60348) (on DE-0259-1 sub board)
1 Deco HMC20 (M60232) (on DE-0259-1 sub board)
1 x BAC06 (on DE-0273-1 board)

  1 playfield, same as above, with rowscroll
  1024 colours from 2 proms.
  Sprite hardware close to above, there are some unused (unknown) bits per sprite.

Super Real Darwin:
  1 playfield, x-scroll only
  Closer to earlier Darwin 4078 board than above games.

Last Mission/Shackled:
    Has 1 Deco VSC30 (M60348) (From readme file)
    Has 1 Deco HMC20 (M60232) (From readme file)

    1 playfield
    Sprite hardware same as Karnov.
    (Shackled) Palettes 8-15 for tiles seem to have priority over sprites.

Gondomania:
    Has two large square surface mount chips: [ DRL 40, 8053, 8649a ]
    Has 1 Deco VSC30 (M60348)
    Has 1 Deco HMC20 (M60232)
    Priority - all tiles with *pens* 8-15 appear over sprites with palettes 8-15.

Oscar:
    Uses MXC-06 custom chip for sprites.
    Uses BAC-06 custom chip for background.
    I can't find what makes the fix chars...
    Priority - tiles with palettes 8-15 have their *pens* 8-15 appearing over
sprites.

***************************************************************************/

#include "emu.h"
#include "includes/dec8.h"

void dec8_state::dec8_bg_data_w(offs_t offset, uint8_t data)
{
	m_bg_data[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset / 2);
}

uint8_t dec8_state::dec8_bg_data_r(offs_t offset)
{
	return m_bg_data[offset];
}


void dec8_state::dec8_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset / 2);
}

void dec8_state::srdarwin_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fix_tilemap->mark_tile_dirty(offset);
}


void dec8_state::dec8_scroll2_w(offs_t offset, uint8_t data)
{
	m_scroll2[offset] = data;
}

void dec8_state::srdarwin_control_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0: /* Top 3 bits - bank switch, bottom 4 - scroll MSB */
		m_mainbank->set_entry((data >> 5));
		m_scroll2[0] = data & 0xf;
		return;

	case 1:
		m_scroll2[1] = data;
		return;
	}
}

void dec8_state::lastmisn_control_w(uint8_t data)
{
	/*
	    Bit 0x0f - ROM bank switch.
	    Bit 0x10 - Unused
	    Bit 0x20 - X scroll MSB
	    Bit 0x40 - Y scroll MSB
	    Bit 0x80 - Hold subcpu reset line high if clear, else low
	*/
	m_mainbank->set_entry(data & 0x0f);

	m_scroll2[0] = (data >> 5) & 1;
	m_scroll2[2] = (data >> 6) & 1;

	if (data & 0x80)
		m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	else
		m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

void dec8_state::shackled_control_w(uint8_t data)
{
	/* Bottom 4 bits - bank switch, Bits 4 & 5 - Scroll MSBs */
	m_mainbank->set_entry(data & 0x0f);

	m_scroll2[0] = (data >> 5) & 1;
	m_scroll2[2] = (data >> 6) & 1;
}

void dec8_state::lastmisn_scrollx_w(uint8_t data)
{
	m_scroll2[1] = data;
}

void dec8_state::lastmisn_scrolly_w(uint8_t data)
{
	m_scroll2[3] = data;
}

void dec8_state::gondo_scroll_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x0:
		m_scroll2[1] = data; /* X LSB */
		break;
	case 0x8:
		m_scroll2[3] = data; /* Y LSB */
		break;
	case 0x10:
		m_scroll2[0] = (data >> 0) & 1; /* Bit 0: X MSB */
		m_scroll2[2] = (data >> 1) & 1; /* Bit 1: Y MSB */
		/* Bit 2 is also used in Gondo & Garyoret */
		break;
	}
}

void dec8_state::allocate_buffered_spriteram16()
{
	m_buffered_spriteram16 = make_unique_clear<uint16_t[]>(0x800/2);
	save_pointer(NAME(m_buffered_spriteram16), 0x800/2);
}

/******************************************************************************/


void dec8_state::srdarwin_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &primap)
{
	uint8_t *buffered_spriteram = m_spriteram->buffer();

	/* Sprites */
	for (int offs = 0x200 - 4; offs >= 0; offs -= 4)
	{
		u32 pri_mask = 0;
		int sy2;

		const u32 color = (buffered_spriteram[offs + 1] & 0x03) + ((buffered_spriteram[offs + 1] & 0x08) >> 1);
		if (color == 0) pri_mask |= GFX_PMASK_2;

		const u32 code = buffered_spriteram[offs + 3] + ((buffered_spriteram[offs + 1] & 0xe0) << 3);
		if (!code) continue;

		int sy = buffered_spriteram[offs];
		if (sy == 0xf8) continue;

		int sx = (241 - buffered_spriteram[offs + 2]);

		int fx = buffered_spriteram[offs + 1] & 0x04;
		const bool multi = buffered_spriteram[offs + 1] & 0x10;

		if (flip_screen())
		{
			sy = 240 - sy;
			sx = 240 - sx;
			if (fx) fx = 0; else fx = 1;
			sy2 = sy - 16;
		}
		else sy2 = sy + 16;

		m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				code,
				color,
				fx,flip_screen(),
				sx,sy,primap,pri_mask,0);
		if (multi)
			m_gfxdecode->gfx(1)->prio_transpen(bitmap,cliprect,
				code+1,
				color,
				fx,flip_screen(),
				sx,sy2,primap,pri_mask,0);
	}
}

/******************************************************************************/

void dec8_state::cobracom_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above foreground, background
	if ((colour & 4) == 0)
		pri_mask |= GFX_PMASK_2; // behind foreground, above background

	colour &= 3;
}

uint32_t dec8_state::screen_update_cobracom(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0,cliprect);
	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_tilegen[1]->set_flip_screen(flip);
	m_spritegen_mxc->set_flip_screen(flip);
	m_fix_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 1);
	m_tilegen[1]->deco_bac06_pf_draw(screen,bitmap,cliprect,0, 2);
	m_spritegen_mxc->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(1), m_buffered_spriteram16.get(), 0x800/2);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/******************************************************************************/


TILE_GET_INFO_MEMBER(dec8_state::get_cobracom_fix_tile_info)
{
	int offs = tile_index << 1;
	int tile = m_videoram[offs + 1] + (m_videoram[offs] << 8);
	int color = (tile & 0xe000) >> 13;

	tileinfo.set(0,
			tile & 0xfff,
			color,
			0);
}

VIDEO_START_MEMBER(dec8_state,cobracom)
{
	allocate_buffered_spriteram16();
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_cobracom_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);

	m_game_uses_priority = 0;
}

/******************************************************************************/

uint32_t dec8_state::screen_update_ghostb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_OPAQUE, 0);
	m_spritegen_krn->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(1), m_buffered_spriteram16.get(), 0x400);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(dec8_state::get_ghostb_fix_tile_info)
{
	int offs = tile_index << 1;
	int tile = m_videoram[offs + 1] + (m_videoram[offs] << 8);
	int color = (tile & 0xc00) >> 10;

	tileinfo.set(0,
			tile & 0x3ff,
			color,
			0);
}

VIDEO_START_MEMBER(dec8_state,ghostb)
{
	allocate_buffered_spriteram16();
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_ghostb_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_fix_tilemap->set_transparent_pen(0);

	m_game_uses_priority = 0;

	m_nmi_enable = false;
	save_item(NAME(m_nmi_enable));
}

/******************************************************************************/

// we mimic the priority scheme in dec0.cpp, this was originally a bit different, so this could be wrong
void dec8_state::oscar_tile_cb(tile_data &tileinfo, u32 &tile, u32 &colour, u32 &flags)
{
	tileinfo.group = BIT(colour, 3) ? 1 : 0;
	colour &= 7;
}

uint32_t dec8_state::screen_update_oscar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bool flip = m_tilegen[0]->get_flip_state();
	m_tilegen[0]->set_flip_screen(flip);
	m_spritegen_mxc->set_flip_screen(flip);
	m_fix_tilemap->set_flip(flip ? (TILEMAP_FLIPY | TILEMAP_FLIPX) : 0);

	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER1, 0);
	m_spritegen_mxc->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(1), m_buffered_spriteram16.get(), 0x800/2);
	m_tilegen[0]->deco_bac06_pf_draw(screen,bitmap,cliprect,TILEMAP_DRAW_LAYER0, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(dec8_state::get_oscar_fix_tile_info)
{
	int offs = tile_index << 1;
	int tile = m_videoram[offs + 1] + (m_videoram[offs] << 8);
	int color = (tile & 0xf000) >> 14;

	tileinfo.set(0,
			tile&0xfff,
			color,
			0);
}

VIDEO_START_MEMBER(dec8_state,oscar)
{
	allocate_buffered_spriteram16();
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_oscar_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_tilegen[0]->set_transmask(0, 0xffff, 0x0000);
	m_tilegen[0]->set_transmask(1, 0x00ff, 0xff00);

	m_game_uses_priority = 1;
}

/******************************************************************************/

uint32_t dec8_state::screen_update_lastmisn(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, ((m_scroll2[0] << 8)+ m_scroll2[1]));
	m_bg_tilemap->set_scrolly(0, ((m_scroll2[2] << 8)+ m_scroll2[3]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_spritegen_krn->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(1), m_buffered_spriteram16.get(), 0x400);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t dec8_state::screen_update_shackled(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, ((m_scroll2[0] << 8) + m_scroll2[1]));
	m_bg_tilemap->set_scrolly(0, ((m_scroll2[2] << 8) + m_scroll2[3]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 0, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1 | 1, 0);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 0, 0);
	m_spritegen_krn->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(1), m_buffered_spriteram16.get(), 0x400);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0 | 1, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILEMAP_MAPPER_MEMBER(dec8_state::lastmisn_scan_rows)
{
	/* logical (col,row) -> memory offset */
	return ((col & 0x0f) + ((row & 0x0f) << 4)) + ((col & 0x10) << 4) + ((row & 0x10) << 5);
}

TILE_GET_INFO_MEMBER(dec8_state::get_lastmisn_tile_info)
{
	int offs = tile_index * 2;
	int tile = m_bg_data[offs + 1] + (m_bg_data[offs] << 8);
	int color = tile >> 12;

	if (color & 8 && m_game_uses_priority)
		tileinfo.category = 1;
	else
		tileinfo.category = 0;

	tileinfo.set(2,
			tile & 0xfff,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dec8_state::get_lastmisn_fix_tile_info)
{
	int offs = tile_index << 1;
	int tile = m_videoram[offs + 1] + (m_videoram[offs] << 8);
	int color = (tile & 0xc000) >> 14;

	tileinfo.set(0,
			tile&0xfff,
			color,
			0);
}

VIDEO_START_MEMBER(dec8_state,lastmisn)
{
	allocate_buffered_spriteram16();
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_lastmisn_tile_info)), tilemap_mapper_delegate(*this, FUNC(dec8_state::lastmisn_scan_rows)), 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_lastmisn_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_game_uses_priority = 0;
}

VIDEO_START_MEMBER(dec8_state,shackled)
{
	allocate_buffered_spriteram16();
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_lastmisn_tile_info)), tilemap_mapper_delegate(*this, FUNC(dec8_state::lastmisn_scan_rows)), 16, 16, 32, 32);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_lastmisn_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transmask(0, 0x000f, 0xfff0); // Bottom 12 pens
	m_game_uses_priority = 1;
}

/******************************************************************************/

uint32_t dec8_state::screen_update_srdarwin(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_bg_tilemap->set_scrollx(0, (m_scroll2[0] << 8) + m_scroll2[1]);

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 2);
	srdarwin_draw_sprites(bitmap, cliprect, screen.priority());
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(dec8_state::get_srdarwin_fix_tile_info)
{
	int tile = m_videoram[tile_index];
	int color = 0; /* ? */

	tileinfo.category = 0;

	tileinfo.set(0,
			tile,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dec8_state::get_srdarwin_tile_info)
{
	int tile = m_bg_data[2 * tile_index + 1] + (m_bg_data[2 * tile_index] << 8);
	int color = tile >> 12 & 3;
	int bank;

	tile = tile & 0x3ff;
	bank = (tile / 0x100) + 2;

	tileinfo.set(bank,
			tile,
			color,
			0);
	tileinfo.group = color;
}

VIDEO_START_MEMBER(dec8_state,srdarwin)
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_srdarwin_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 16);
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_srdarwin_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transmask(0, 0xffff, 0x0000); // draw as background only
	m_bg_tilemap->set_transmask(1, 0x00ff, 0xff00); // Bottom 8 pens
	m_bg_tilemap->set_transmask(2, 0x00ff, 0xff00); // Bottom 8 pens
	m_bg_tilemap->set_transmask(3, 0x0000, 0xffff); // draw as foreground only
}

/******************************************************************************/

void dec8_state::gondo_colpri_cb(u32 &colour, u32 &pri_mask)
{
	pri_mask = 0; // above foreground, background
	if (colour & 8)
		pri_mask |= GFX_PMASK_2; // behind foreground, above background
}

uint32_t dec8_state::screen_update_gondo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0, cliprect);
	m_bg_tilemap->set_scrollx(0, ((m_scroll2[0] << 8) + m_scroll2[1]));
	m_bg_tilemap->set_scrolly(0, ((m_scroll2[2] << 8) + m_scroll2[3]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER1, 1);
	m_bg_tilemap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_LAYER0, 2);
	m_spritegen_krn->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(1), m_buffered_spriteram16.get(), 0x400);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

uint32_t dec8_state::screen_update_garyoret(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->set_scrollx(0, ((m_scroll2[0] << 8) + m_scroll2[1]));
	m_bg_tilemap->set_scrolly(0, ((m_scroll2[2] << 8) + m_scroll2[3]));

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_spritegen_krn->draw_sprites(screen, bitmap, cliprect, m_gfxdecode->gfx(1), m_buffered_spriteram16.get(), 0x400);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 1, 0);
	m_fix_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

TILE_GET_INFO_MEMBER(dec8_state::get_gondo_fix_tile_info)
{
	int offs = tile_index * 2;
	int tile = m_videoram[offs + 1] + (m_videoram[offs] << 8);
	int color = (tile & 0x7000) >> 12;

	tileinfo.set(0,
			tile&0xfff,
			color,
			0);
}

TILE_GET_INFO_MEMBER(dec8_state::get_gondo_tile_info)
{
	int offs = tile_index * 2;
	int tile = m_bg_data[offs + 1] + (m_bg_data[offs] << 8);
	int color = tile>> 12;

	if (color & 8 && m_game_uses_priority)
		tileinfo.category = 1;
	else
		tileinfo.category = 0;

	tileinfo.set(2,
			tile&0xfff,
			color,
			0);
}

VIDEO_START_MEMBER(dec8_state,gondo)
{
	allocate_buffered_spriteram16();
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_gondo_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_gondo_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_transmask(0, 0x00ff, 0xff00); /* Bottom 8 pens */
	m_game_uses_priority = 0;
}

VIDEO_START_MEMBER(dec8_state,garyoret)
{
	allocate_buffered_spriteram16();
	m_fix_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_gondo_fix_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(dec8_state::get_gondo_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);

	m_fix_tilemap->set_transparent_pen(0);
	m_game_uses_priority = 1;
}

/******************************************************************************/
