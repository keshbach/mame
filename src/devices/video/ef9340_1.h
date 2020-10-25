// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/***************************************************************************

    Thomson EF9340 + EF9341 teletext graphics chips

***************************************************************************/

#ifndef MAME_VIDEO_EF9340_1_H
#define MAME_VIDEO_EF9340_1_H

#pragma once


class ef9340_1_device : public device_t,
						public device_video_interface
{
public:
	// construction/destruction
	template <typename T>
	ef9340_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&screen_tag)
		: ef9340_1_device(mconfig, tag, owner, clock)
	{
		set_screen(std::forward<T>(screen_tag));
	}

	// configuration helpers
	ef9340_1_device &set_offsets(int x, int y) { m_offset_x = x; m_offset_y = y; return *this; } // when used with overlay chip
	auto write_exram() { return m_write_exram.bind(); } // ADR0-ADR3 in a0-a3, B in a4-a11, A in a12-a19
	auto read_exram() { return m_read_exram.bind(); } // "

	ef9340_1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	inline bitmap_ind16 *get_bitmap() { return &m_tmp_bitmap; }
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ef9341_write(u8 command, u8 b, u8 data);
	u8 ef9341_read(u8 command, u8 b);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	inline u16 ef9340_get_c_addr(u8 x, u8 y);
	inline void ef9340_inc_c();

	void ef9340_scanline(int vpos);

	/* timers */
	static constexpr device_timer_id TIMER_LINE = 0;
	static constexpr device_timer_id TIMER_BLINK = 1;

	emu_timer *m_line_timer;
	emu_timer *m_blink_timer;

	required_region_ptr<u8> m_charset;

	bitmap_ind16 m_tmp_bitmap;

	int m_offset_x = 0;
	int m_offset_y = 0;

	struct
	{
		u8 TA;
		u8 TB;
		bool busy;
	} m_ef9341;

	struct
	{
		u8 X;
		u8 Y;
		u8 Y0;
		u8 R;
		u8 M;
		bool blink;
		int blink_prescaler;
		bool h_parity;
	} m_ef9340;

	u8 m_ram_a[0x400];
	u8 m_ram_b[0x400];

	devcb_write8 m_write_exram;
	devcb_read8 m_read_exram;
};


// device type definition
DECLARE_DEVICE_TYPE(EF9340_1, ef9340_1_device)

#endif // MAME_VIDEO_EF9340_1_H
