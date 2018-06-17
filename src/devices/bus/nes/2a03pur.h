// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_NES_2A03PUR_H
#define MAME_BUS_NES_2A03PUR_H

#pragma once

#include "nxrom.h"


// ======================> nes_racermate_device

class nes_2a03pur_device : public nes_nrom_device
{
public:
	// construction/destruction
	nes_2a03pur_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER(read_h) override;
	virtual DECLARE_WRITE8_MEMBER(write_l) override;

	virtual void pcb_reset() override;

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	uint8_t m_reg[8];
};


// device type definition
DECLARE_DEVICE_TYPE(NES_2A03PURITANS, nes_2a03pur_device)

#endif // MAME_BUS_NES_2A03PUR_H
