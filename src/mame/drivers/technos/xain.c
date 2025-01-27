/***************************************************************************

Solar Warrior / Xain'd Sleena
Technos, 1986

PCB Layout
----------

Top Board

TA-0019-P1-03
|------------------------------------------------------------------------|
|M51516 YM3014 YM2203 YM2203 68A09 P2-0.49                               |
|       YM3014                                                           |
|                                  6116                                 |--|
|                                                                       |  |
|        2018                      *                                    |  |
|        6148                                                           |  |
|                           PT-0.59                                     |  |
|                                  P3-0.46                              |  |
|                                                                       |  |
|J                                                                      |  |
|A    DSW2                         P4-0.45                              |--|
|M                                                     2018              |
|M                                                                       |
|A                                 P5-0.44                               |
|     DSW1                                                               |
|                                                                       |--|
|                                  P6-0.43                              |  |
|            68B09                                                      |  |
|                                              68B09                    |  |
|                   P9-02.66       P7-0.42                              |  |
|                                                                       |  |
|                                                                       |  |
| 68705             PA-03.65       P8-0.41                              |  |
| (PZ-0.113)                                P1-02.29  P0-02.15   6264   |--|
|                                                                        |
|                                  *                                     |
|------------------------------------------------------------------------|
Notes:
        6809 - Hitachi HD68A09 / HD68B09 CPU, running at 1.500MHz [12/8] (x3, DIP40)
      YM2203 - Yamaha YM2203C sound chip, running at 3.000MHz [12/4] (x2, DIP40)
       68705 - Motorola MC68705P5S Microcontroller, running at 3.000MHz [12/4] (labelled 'PZ-0, DIP28)
           * - Empty socket
        6264 - Hitachi HM6264LP-15 8K x8 SRAM (DIP28)
        2018 - Toshiba TMM2018D-45 2K x8 SRAM (DIP24)
        6148 - Hitachi HM6148HP-45 1K x4 SRAM (DIP18)
        6116 - Hitachi HM6116LP-3 2K x8 SRAM (DIP24)

       VSync - 57Hz

        ROMs - Name         Device       Use
               P9-02.66     TMM24256   \ CPU1 program
               PA-03.65     MBM27256   /

               P0-02.15     TMM24256   \ CPU2 program
               P1-02.29     TMM24256   /

               P2-0.49      TMM24256     Sound CPU program

               P3-0.46      TMM24256   / GFX
               P4-0.45      TMM24256   |
               P5-0.44      TMM24256   |
               P6-0.43      TMM24256   |
               P7-0.42      TMM24256   |
               P8-0.41      TMM24256   /

Bottom Board

TA-0019-P2-03
|------------------------------------------------------------------------|
|    PK-0.136  PC-0.114                                                  |
|    PL-0.135  PD-0.113                                                  |
|                                                                       |--|
|    PM-0.134  PE-0.112                                                 |  |
|    PN-02.133 PF-02.111                                                |  |
|                                                                       |  |
|           2018                                                        |  |
|                                                                       |  |
|    PO-0.131  PG-0.109                                                 |  |
|                                                                       |  |
|    PP-0.130  PH-0.108                                                 |--|
|                                                                  12MHz |
|    PQ-0.129  PI-0.107                                PB-0.24           |
|                                                                        |
|    PR-0.128  PJ-0.106                                6116              |
|                                                                       |--|
|         2018                                                          |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|                                                                       |  |
|         2018                                                   2018   |  |
|                                                                       |--|
|             2018                                                       |
|             2018                                                       |
|------------------------------------------------------------------------|
Notes:
        2018 - Toshiba TMM2018D-45 2K x8 SRAM (DIP24)

        ROMs - Name         Device       Use
               PB-0.24      TMM24256   / GFX
               PC-0.114     TMM24256   |
               PD-0.113     TMM24256   |
               PE-0.112     TMM24256   |
               PF-02.111    MBM27256   |
               PG-0.109     TMM24256   |
               PH-0.108     TMM24256   |
               PI-0.107     TMM24256   |
               PJ-0.106     TMM24256   |
               PK-0.136     TMM24256   |
               PL-0.135     TMM24256   |
               PM-0.134     TMM24256   |
               PN-02.133    MBM27256   |
               PO-0.131     TMM24256   |
               PP-0.130     TMM24256   |
               PQ-0.129     TMM24256   |
               PR-0.128     TMM24256   /


Driver by Carlos A. Lozano & Rob Rosenbrock & Phil Stroffolino
Updates by Bryan McPhail, 12/12/2004:
    Fixed NMI & FIRQ handling according to schematics.
    Fixed clock speeds.
    Implemented GFX priority register/priority PROM

    Xain has a semi-bug that shows up in MAME - at 0xa26b there is a tight
    loop that checks for the VBLANK input bit going high.  However at the
    start of a game the VBLANK interrupt routine doesn't return before
    the VBLANK input bit goes low (VBLANK is held high for 8 scanlines only).
    This would cause the emulation to hang, but it would work on the real
    board because the instruction currently being decoded would finish
    before the NMI was taken, so the VBLANK bit and NMI are not actually
    exactly synchronised in practice.  This is currently hacked in MAME
    by raising the VBLANK bit a scanline early.

***************************************************************************/

#include "emu.h"
#if defined(ARM_ENABLED)
#include "deprecat.h"
#endif

#include "cpu/m6809/m6809.h"
#include "cpu/m6805/m6805.h"
#include "sound/2203intf.h"

#include "includes/xain.h"


#if defined(ARM_ENABLED)
static INTERRUPT_GEN( xain_interrupt )
{
	xain_state *state = device->machine->driver_data<xain_state>();

	int scanline = 255 - cpu_getiloops(device);

	/* FIRQ(IMS) fires every on every 8th scanline (except 0) */
	if (scanline & 0x08)
		cpu_set_input_line(device, M6809_FIRQ_LINE, ASSERT_LINE);

	/* NMI fires on scanline 248 (VBL) and is latched */
	if (scanline == 248)
		cpu_set_input_line(device, INPUT_LINE_NMI, ASSERT_LINE);

	/* VBLANK input bit is held high from scanline 248 - 255 */
	state->vblank = (scanline >= 248 - 1) ? 1 : 0;
}
#else
/*
	Based on the Solar Warrior schematics, vertical timing counts as follows:

	   08,09,0A,0B,...,FC,FD,FE,FF,E8,E9,EA,EB,...,FC,FD,FE,FF,
	   08,09,....

	Thus, it counts from 08 to FF, then resets to E8 and counts to FF again.
	This gives (256 - 8) + (256 - 232) = 248 + 24 = 272 total scanlines.

	VBLK is signalled starting when the counter hits F8, and continues through
	the reset to E8 and through until the next reset to 08 again.

	Since MAME's video timing is 0-based, we need to convert this.
*/

INLINE int scanline_to_vcount(int scanline)
{
	int vcount = scanline + 8;

	if (vcount < 0x100)
		return vcount;
	else
		return (vcount - 0x18) | 0x100;
}

static TIMER_DEVICE_CALLBACK( xain_scanline )
{
	xain_state *state = timer.machine->driver_data<xain_state>();

	int scanline = param;
	int screen_height = timer.machine->primary_screen->height();
	int vcount_old = scanline_to_vcount((scanline == 0) ? screen_height - 1 : scanline - 1);
	int vcount = scanline_to_vcount(scanline);

	/* update to the current point */
	if (scanline > 0)
		timer.machine->primary_screen->update_partial(scanline - 1);

	/* FIRQ (IMS) fires every on every 8th scanline (except 0) */
	if (!(vcount_old & 0x08) && (vcount & 0x08))
		cpu_set_input_line(state->maincpu, M6809_FIRQ_LINE, ASSERT_LINE);

	/* NMI fires on scanline 248 (VBL) and is latched */
	if (vcount == 0xf8)
		cpu_set_input_line(state->maincpu, INPUT_LINE_NMI, ASSERT_LINE);

	/* VBLANK input bit is held high from scanlines 248-255 */
	state->vblank = (vcount >= 248 - 1) ? 1 : 0;	// -1 is a hack - see notes above
}
#endif

static WRITE8_HANDLER( xainCPUA_bankswitch_w )
{
	xain_state *state = space->machine->driver_data<xain_state>();

	state->xain_pri = data & 0x07;
	memory_set_bank(space->machine, "bank1", (data >> 3) & 0x01);
}

static WRITE8_HANDLER( xainCPUB_bankswitch_w )
{
	memory_set_bank(space->machine, "bank2", data & 0x01);
}

static WRITE8_HANDLER( xain_sound_command_w )
{
	soundlatch_w(space, offset, data);
	cputag_set_input_line(space->machine, "audiocpu", M6809_IRQ_LINE, HOLD_LINE);
}

static WRITE8_HANDLER( xain_main_irq_w )
{
	switch (offset)
	{
		case 0:		/* 0x3a09 - NMI clear */
			cputag_set_input_line(space->machine, "maincpu", INPUT_LINE_NMI, CLEAR_LINE);
			break;
		case 1:		/* 0x3a0a - FIRQ clear */
			cputag_set_input_line(space->machine, "maincpu", M6809_FIRQ_LINE, CLEAR_LINE);
			break;
		case 2:		/* 0x3a0b - IRQ clear */
			cputag_set_input_line(space->machine, "maincpu", M6809_IRQ_LINE, CLEAR_LINE);
			break;
		case 3:		/* 0x3a0c - IRQB assert */
			cputag_set_input_line(space->machine, "sub", M6809_IRQ_LINE, ASSERT_LINE);
			break;
	}
}

static WRITE8_HANDLER( xain_irqA_assert_w )
{
	cputag_set_input_line(space->machine, "maincpu", M6809_IRQ_LINE, ASSERT_LINE);
}

static WRITE8_HANDLER( xain_irqB_clear_w )
{
	cputag_set_input_line(space->machine, "sub", M6809_IRQ_LINE, CLEAR_LINE);
}

static CUSTOM_INPUT( xain_vblank_r )
{
	xain_state *state = field->port->machine->driver_data<xain_state>();
	return state->vblank;
}

static READ8_HANDLER( xain_68705_r )
{
	xain_state *state = space->machine->driver_data<xain_state>();

	state->_mcu_ready = 1;

	return state->from_mcu;
}

static WRITE8_HANDLER( xain_68705_w )
{
	xain_state *state = space->machine->driver_data<xain_state>();

	state->from_main = data;
	state->_mcu_accept = 0;

	if (state->mcu != NULL)
		cpu_set_input_line(state->mcu, 0, ASSERT_LINE);
}

/***************************************************************************

    MC68705P5 I/O

***************************************************************************/

READ8_HANDLER( xain_68705_port_a_r )
{
	xain_state *state = space->machine->driver_data<xain_state>();
	return (state->port_a_out & state->ddr_a) | (state->port_a_in & ~state->ddr_a);
}

WRITE8_HANDLER( xain_68705_port_a_w )
{
	xain_state *state = space->machine->driver_data<xain_state>();
	state->port_a_out = data;
}

WRITE8_HANDLER( xain_68705_ddr_a_w )
{
	xain_state *state = space->machine->driver_data<xain_state>();
	state->ddr_a = data;
}

READ8_HANDLER( xain_68705_port_b_r )
{
	xain_state *state = space->machine->driver_data<xain_state>();
	return (state->port_b_out & state->ddr_b) | (state->port_b_in & ~state->ddr_b);
}

WRITE8_HANDLER( xain_68705_port_b_w )
{
	xain_state *state = space->machine->driver_data<xain_state>();

	if (state->ddr_b & 0x02)
	{
		if (~data & 0x02)
			state->port_a_in = state->from_main;

		/* Rising edge of PB1 */
		else if ((~state->port_b_out & 0x02) && (data & 0x02))
		{
			state->_mcu_accept = 1;
			cpu_set_input_line(state->mcu, 0, CLEAR_LINE);
		}
	}
	if (state->ddr_b & 0x04)
	{
		/* Rising edge of PB2 */
		if ((~state->port_b_out & 0x04) && (data & 0x04))
		{
			state->_mcu_ready = 0;
			state->from_mcu = state->port_a_out;
		}
	}

	state->port_b_out = data;
}

WRITE8_HANDLER( xain_68705_ddr_b_w )
{
	xain_state *state = space->machine->driver_data<xain_state>();
	state->ddr_b = data;
}

READ8_HANDLER( xain_68705_port_c_r )
{
	xain_state *state = space->machine->driver_data<xain_state>();

	state->port_c_in = 0;

	if (!state->_mcu_accept)
		state->port_c_in |= 0x01;

	if (state->_mcu_ready)
		state->port_c_in |= 0x02;

	return (state->port_c_out & state->ddr_c) | (state->port_c_in & ~state->ddr_c);
}

WRITE8_HANDLER( xain_68705_port_c_w )
{
	xain_state *state = space->machine->driver_data<xain_state>();
	state->port_c_out = data;
}

WRITE8_HANDLER( xain_68705_ddr_c_w )
{
	xain_state *state = space->machine->driver_data<xain_state>();
	state->ddr_c = data;
}

static CUSTOM_INPUT( mcu_status_r )
{
	xain_state *state = field->port->machine->driver_data<xain_state>();

	UINT8 res = 0;

	if (state->mcu != NULL)
	{
		if (state->_mcu_ready == 1)
			res |= 0x01;
		if (state->_mcu_accept == 1)
			res |= 0x02;
	}
	else
		return 0x03;

	return res;
}

READ8_HANDLER( mcu_comm_reset_r )
{
	xain_state *state = space->machine->driver_data<xain_state>();

	state->_mcu_ready = 1;
	state->_mcu_accept = 1;

	if (state->mcu != NULL)
		cpu_set_input_line(state->mcu, 0, CLEAR_LINE);

	return 0xff;
}


static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(xain_charram_w) AM_BASE_MEMBER(xain_state, xain_charram)
	AM_RANGE(0x2800, 0x2fff) AM_RAM_WRITE(xain_bgram1_w) AM_BASE_MEMBER(xain_state, xain_bgram1)
	AM_RANGE(0x3000, 0x37ff) AM_RAM_WRITE(xain_bgram0_w) AM_BASE_MEMBER(xain_state, xain_bgram0)
	AM_RANGE(0x3800, 0x397f) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x3a00, 0x3a00) AM_READ_PORT("P1")
	AM_RANGE(0x3a00, 0x3a01) AM_WRITE(xain_scrollxP1_w)
	AM_RANGE(0x3a01, 0x3a01) AM_READ_PORT("P2")
	AM_RANGE(0x3a02, 0x3a02) AM_READ_PORT("DSW0")
	AM_RANGE(0x3a02, 0x3a03) AM_WRITE(xain_scrollyP1_w)
	AM_RANGE(0x3a03, 0x3a03) AM_READ_PORT("DSW1")
	AM_RANGE(0x3a04, 0x3a04) AM_READ(xain_68705_r)
	AM_RANGE(0x3a04, 0x3a05) AM_WRITE(xain_scrollxP0_w)
	AM_RANGE(0x3a05, 0x3a05) AM_READ_PORT("VBLANK")
	AM_RANGE(0x3a06, 0x3a06) AM_READ(mcu_comm_reset_r)
	AM_RANGE(0x3a06, 0x3a07) AM_WRITE(xain_scrollyP0_w)
	AM_RANGE(0x3a08, 0x3a08) AM_WRITE(xain_sound_command_w)
	AM_RANGE(0x3a09, 0x3a0c) AM_WRITE(xain_main_irq_w)
	AM_RANGE(0x3a0d, 0x3a0d) AM_WRITE(xain_flipscreen_w)
	AM_RANGE(0x3a0e, 0x3a0e) AM_WRITE(xain_68705_w)
	AM_RANGE(0x3a0f, 0x3a0f) AM_WRITE(xainCPUA_bankswitch_w)
	AM_RANGE(0x3c00, 0x3dff) AM_WRITE(paletteram_xxxxBBBBGGGGRRRR_split1_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x3e00, 0x3fff) AM_WRITE(paletteram_xxxxBBBBGGGGRRRR_split2_w) AM_BASE_GENERIC(paletteram2)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu_map_B, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x1fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x2000, 0x2000) AM_WRITE(xain_irqA_assert_w)
	AM_RANGE(0x2800, 0x2800) AM_WRITE(xain_irqB_clear_w)
	AM_RANGE(0x3000, 0x3000) AM_WRITE(xainCPUB_bankswitch_w)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( mcu_map, ADDRESS_SPACE_PROGRAM, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(xain_68705_port_a_r, xain_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(xain_68705_port_b_r, xain_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(xain_68705_port_c_r, xain_68705_port_c_w)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(xain_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(xain_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(xain_68705_ddr_c_w)
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x1000, 0x1000) AM_READ(soundlatch_r)
	AM_RANGE(0x2800, 0x2801) AM_DEVWRITE("ym1", ym2203_w)
	AM_RANGE(0x3000, 0x3001) AM_DEVWRITE("ym2", ym2203_w)
	AM_RANGE(0x4000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( xsleena )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) )		PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_A ) )		PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )		PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Game_Time ) )	PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x0c, "Slow" )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, "Fast" )
	PORT_DIPSETTING(    0x00, "Very Fast" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )	PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "20k 70k and every 70k" )
	PORT_DIPSETTING(    0x20, "30k 80k and every 80k" )
	PORT_DIPSETTING(    0x10, "20k and 80k" )
	PORT_DIPSETTING(    0x00, "30k and 80k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )		PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "3")
	PORT_DIPSETTING(    0x80, "4")
	PORT_DIPSETTING(    0x40, "6")
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")

	PORT_START("VBLANK")
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x18, IP_ACTIVE_HIGH, IPT_SPECIAL) PORT_CUSTOM(mcu_status_r, NULL)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM(xain_vblank_r, NULL)	/* VBLANK */
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNUSED )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,2) },
	{ STEP2(0*8+1,-1), STEP2(8*8+1,-1), STEP2(16*8+1,-1), STEP2(24*8+1,-1) },
	{ STEP8(0,8) },
	32*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 0, 4 },
	{ STEP4(0*8+3,-1), STEP4(16*8+3,-1), STEP4(32*8+3,-1), STEP4(48*8+3,-1) },
	{ STEP16(0,8) },
	64*8
};

static GFXDECODE_START( xain )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 8 )	/* 8x8 text */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 256, 8 )	/* 16x16 Background */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 384, 8 )	/* 16x16 Background */
	GFXDECODE_ENTRY( "gfx4", 0, tilelayout, 128, 8 )	/* Sprites */
GFXDECODE_END


/* handler called by the 2203 emulator when the internal timers cause an IRQ */
static void irqhandler(running_device *device, int irq)
{
	cputag_set_input_line(device->machine, "audiocpu", M6809_FIRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2203_interface ym2203_config =
{
	{	AY8910_LEGACY_OUTPUT,
		AY8910_DEFAULT_LOADS,
		DEVCB_NULL, DEVCB_NULL,
		DEVCB_NULL, DEVCB_NULL	}, irqhandler
};

static MACHINE_START( xsleena )
{
	xain_state *state = machine->driver_data<xain_state>();

	memory_configure_bank(machine, "bank1", 0, 2, memory_region(machine, "maincpu") + 0x4000, 0xc000);
	memory_configure_bank(machine, "bank2", 0, 2, memory_region(machine, "sub")  + 0x4000, 0xc000);
	memory_set_bank(machine, "bank1", 0);
	memory_set_bank(machine, "bank2", 0);

	state->maincpu = machine->device("maincpu");
	state->audiocpu = machine->device("audiocpu");
	state->subcpu = machine->device("sub");
	state->mcu = machine->device("mcu");

	state_save_register_global(machine, state->xain_pri);
	state_save_register_global(machine, state->vblank);

	if (state->mcu != NULL)		/* xsleenab and xsleenaba has no mcu */
	{
		state_save_register_global(machine, state->from_main);
		state_save_register_global(machine, state->from_mcu);
		state_save_register_global(machine, state->_mcu_ready);
		state_save_register_global(machine, state->_mcu_accept);
		state_save_register_global(machine, state->ddr_a);
		state_save_register_global(machine, state->ddr_b);
		state_save_register_global(machine, state->ddr_c);
		state_save_register_global(machine, state->port_a_in);
		state_save_register_global(machine, state->port_b_in);
		state_save_register_global(machine, state->port_c_in);
		state_save_register_global(machine, state->port_a_out);
		state_save_register_global(machine, state->port_b_out);
		state_save_register_global(machine, state->port_c_out);
	}
}

static MACHINE_DRIVER_START( xsleena )
	MDRV_DRIVER_DATA(xain_state)

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M6809, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(main_map)
#if defined(ARM_ENABLED)
	MDRV_CPU_VBLANK_INT_HACK(xain_interrupt, 256)
#else
	MDRV_TIMER_ADD_SCANLINE("scantimer", xain_scanline, "screen", 0, 1)
#endif
	MDRV_CPU_ADD("sub", M6809, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(cpu_map_B)
	MDRV_CPU_ADD("audiocpu", M6809, CPU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_ADD("mcu", M68705, MCU_CLOCK)
	MDRV_CPU_PROGRAM_MAP(mcu_map)
	MDRV_MACHINE_START(xsleena)
	MDRV_QUANTUM_PERFECT_CPU("maincpu")

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
#if defined(ARM_ENABLED)
	MDRV_SCREEN_REFRESH_RATE(57.444853)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
#else
	MDRV_SCREEN_RAW_PARAMS(PIXEL_CLOCK, 384, 0, 256, 272, 8, 248)	/* based on ddragon driver */
#endif
	MDRV_GFXDECODE(xain)
	MDRV_PALETTE_LENGTH(0x200)
	MDRV_VIDEO_START(xain)
	MDRV_VIDEO_UPDATE(xain)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ym1", YM2203, MCU_CLOCK)
	MDRV_SOUND_CONFIG(ym2203_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)
	MDRV_SOUND_ROUTE(2, "mono", 0.50)
	MDRV_SOUND_ROUTE(3, "mono", 0.40)
	MDRV_SOUND_ADD("ym2", YM2203, MCU_CLOCK)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)
	MDRV_SOUND_ROUTE(2, "mono", 0.50)
	MDRV_SOUND_ROUTE(3, "mono", 0.40)
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( xsleenab )
	MDRV_IMPORT_FROM(xsleena)
	MDRV_DEVICE_REMOVE("mcu")
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/* Xain'd Sleena (World) */
ROM_START( xsleena )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-08.ic66",   0x08000, 0x8000, CRC(5179ae3f) SHA1(9e4e2825e56b090aa759b0da39ccb17ccd77ede2) ) /* Master M6809 Code */
	ROM_LOAD( "pa-09.ic65",   0x04000, 0x4000, CRC(10a7c800) SHA1(f19201fe1414faed649b8e49416025aae44bcb6c) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) ) /* Slave M6809 code */
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) ) /* Sound M6809 Code */

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

/* Xain'd Sleena (Japan) */
ROM_START( xsleenaj )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-01.ic66",   0x08000, 0x8000, CRC(370164be) SHA1(65c9951cac7dc3943fa4d5f9919ebb4c4f29b3ae) ) /* the '1' on the label was ink stamped */
	ROM_LOAD( "pa-0.ic65",    0x04000, 0x4000, CRC(d22bf859) SHA1(9edb159bef2eba2c5d93c03c15fbcb87eea52236) ) /* Need to verify proper rom label */
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

/* Solar-Warrior (US) */
ROM_START( solrwarr )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "p9-02.ic66",   0x08000, 0x8000, CRC(8ff372a8) SHA1(0fc396e662419fb9cb5bea11748aa8e0e8d072e6) )
	ROM_LOAD( "pa-03.ic65",   0x04000, 0x4000, CRC(154f946f) SHA1(25b776eb9c494e5302795ae79e494cbfc7c104b1) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-02.ic29",   0x08000, 0x8000, CRC(f5f235a3) SHA1(9f57dd7c5e514afa750edc6da6d263bf1e913c14) )
	ROM_LOAD( "p0-02.ic133",  0x04000, 0x4000, CRC(51ae95ae) SHA1(e03f7ccb0b33b05547577c60a7f92dc75e24b4d6) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x0800, "mcu", 0 )
	ROM_LOAD( "pz-0.113",      0x0000, 0x0800, CRC(a432a907) SHA1(4708a40e3a82dec2c5a64bc5da884a37d503cb6b) ) /* MC68705P5S MCU internal code */

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-02.ic133",  0x18000, 0x8000, CRC(d2ed6f94) SHA1(155a0d1d978f07517400d0c602fc40657f8569dc) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-02.ic111",  0x38000, 0x8000, CRC(6e627a77) SHA1(1d16031acd53c9e691ae7eac8a6f1ae3954fac8c) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

/* Xain'd Sleena (bootleg) */
ROM_START( xsleenab )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "1.rom",        0x08000, 0x8000, CRC(79f515a7) SHA1(e61f18e3639dd9afe16c7bcb90fa7be31905e2c6) )
	ROM_LOAD( "pa-0.ic65",    0x04000, 0x4000, CRC(d22bf859) SHA1(9edb159bef2eba2c5d93c03c15fbcb87eea52236) ) /* Need to verify proper rom label */
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END

/* Xain'd Sleena (bootleg, bugfixed)
   newer bootleg, fixes some of the issues with the other one
   f205v id 1016	*/
ROM_START( xsleenaba )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "xs87b-10.7d",    0x08000, 0x8000, CRC(3d5f9fb4) SHA1(d315b5415a471e05ee61b84fcaf739c75f890061) )
	ROM_LOAD( "xs87b-11.7c",    0x04000, 0x4000, CRC(81c80d54) SHA1(2049c5843c6134560d8fbacf020cbff34833ded7) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x14000, "sub", 0 )
	ROM_LOAD( "p1-0.ic29",    0x08000, 0x8000, CRC(a1a860e2) SHA1(fb2b152bfafc44608039774436ddf3b17eed979c) )
	ROM_LOAD( "p0-0.ic15",    0x04000, 0x4000, CRC(948b9757) SHA1(3ea840cc47ae6a66f3e5f6a2f3e88475dcfe1840) )
	ROM_CONTINUE(             0x10000, 0x4000 )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "p2-0.ic49",     0x8000, 0x8000, CRC(a5318cb8) SHA1(35fb28c5598e39f22552bb036ae356b78422f080) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "pb-01.ic24",   0x00000, 0x8000, CRC(83c00dd8) SHA1(8e9b19281039b63072270c7a63d9fb30cda570fd) ) /* chars */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "pk-0.ic136",   0x00000, 0x8000, CRC(11eb4247) SHA1(5d2f1fa07b8fb1c6bebfdb02c39282d29813791b) ) /* tiles */
	ROM_LOAD( "pl-0.ic135",   0x08000, 0x8000, CRC(422b536e) SHA1(d5985c0bd1c840cb6f0da6b177a2caaff6db5a04) )
	ROM_LOAD( "pm-0.ic134",   0x10000, 0x8000, CRC(828c1b0c) SHA1(cb9b64073b0ade3885f61545191db4c445e3066b) )
	ROM_LOAD( "pn-0.ic133",   0x18000, 0x8000, CRC(d37939e0) SHA1(301d9f6720857c64a4e070444a07a38138ddd4ef) )
	ROM_LOAD( "pc-0.ic114",   0x20000, 0x8000, CRC(8f0aa1a7) SHA1(be3fdb6204b77dba28b14c5b880d65d7c1d6a161) )
	ROM_LOAD( "pd-0.ic113",   0x28000, 0x8000, CRC(45681910) SHA1(60c3eb4bc08bf11bf09bcd27549c6427fafbb1fb) )
	ROM_LOAD( "pe-0.ic112",   0x30000, 0x8000, CRC(a8eeabc8) SHA1(e5dc31df0b223b65144af3602be5bcb2ff9eebbd) )
	ROM_LOAD( "pf-0.ic111",   0x38000, 0x8000, CRC(e59a2f27) SHA1(4643cea85f8613c36b416f46f9d1753fa9839237) )

	ROM_REGION( 0x40000, "gfx3", 0 )
	ROM_LOAD( "p5-0.ic44",    0x00000, 0x8000, CRC(5c6c453c) SHA1(68c0028d15da8f5e53f09e3d154d18cd9f219601) ) /* tiles */
	ROM_LOAD( "p4-0.ic45",    0x08000, 0x8000, CRC(59d87a9a) SHA1(f23cb9a9d6c6249a8a1f8e2acbc235086b008c7b) )
	ROM_LOAD( "p3-0.ic46",    0x10000, 0x8000, CRC(84884a2e) SHA1(5087010a72226e91a084a61b5089c110dba7e933) )
	/* 0x60000-0x67fff empty */
	ROM_LOAD( "p6-0.ic43",    0x20000, 0x8000, CRC(8d637639) SHA1(301a7893de8f1bb526f5075e2af8203b8af4b0d3) )
	ROM_LOAD( "p7-0.ic42",    0x28000, 0x8000, CRC(71eec4e6) SHA1(3417c52a39a6fc43c51ad707168180f54153177a) )
	ROM_LOAD( "p8-0.ic41",    0x30000, 0x8000, CRC(7fc9704f) SHA1(b6f353fb7fec58f68b9e28be2aa29146ac64ffd4) )
	/* 0x80000-0x87fff empty */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "po-0.ic131",   0x00000, 0x8000, CRC(252976ae) SHA1(534c9148d33e453f3541543a8c0eb4afc59c7de8) ) /* sprites */
	ROM_LOAD( "pp-0.ic130",   0x08000, 0x8000, CRC(e6f1e8d5) SHA1(2ee0227361d1f1358f5b5964dab7e691243cd9ae) )
	ROM_LOAD( "pq-0.ic129",   0x10000, 0x8000, CRC(785381ed) SHA1(95bf4eb29830c589a9793a4138e645e5b77f0c06) )
	ROM_LOAD( "pr-0.ic128",   0x18000, 0x8000, CRC(59754e3d) SHA1(d1781dbc83965fc84492f7282d6813507ba1e81b) )
	ROM_LOAD( "pg-0.ic109",   0x20000, 0x8000, CRC(4d977f33) SHA1(30b446ddb2f32354334ea780c435f2407d128808) )
	ROM_LOAD( "ph-0.ic108",   0x28000, 0x8000, CRC(3f3b62a0) SHA1(ab7e8f0ff707771401e679b6151ad0ea85cfc792) )
	ROM_LOAD( "pi-0.ic107",   0x30000, 0x8000, CRC(76641ee3) SHA1(8fba0fa6639e7bdfb3f7be5e945a55b64411d242) )
	ROM_LOAD( "pj-0.ic106",   0x38000, 0x8000, CRC(37671f36) SHA1(1494eec4ecde9ae1f1101aa13eb301b3f3d06602) )

	ROM_REGION( 0x0100, "proms", 0 ) /* Priority */
	ROM_LOAD( "pt-0.ic59",    0x00000, 0x0100, CRC(fed32888) SHA1(4e9330456b20f7198c1e27ca1ae7200f25595599) ) /* BPROM type MB7114E  Priority (not used) */
ROM_END


GAME( 1986, xsleena,	0,	  xsleena,   xsleena,	0,   ROT0,  "Technos Japan (Taito license)", "Xain'd Sleena (World)", GAME_SUPPORTS_SAVE )
GAME( 1986, xsleenaj,	xsleena,  xsleena,   xsleena,	0,   ROT0,  "Technos Japan", "Xain'd Sleena (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1986, solrwarr,	xsleena,  xsleena,   xsleena,	0,   ROT0,  "Technos Japan (Taito license)", "Solar-Warrior (US)", GAME_SUPPORTS_SAVE )
GAME( 1986, xsleenab,	xsleena,  xsleenab,  xsleena,	0,   ROT0,  "bootleg", "Xain'd Sleena (bootleg)", GAME_SUPPORTS_SAVE )
GAME( 1987, xsleenaba,	xsleena,  xsleenab,  xsleena,	0,   ROT0,  "bootleg", "Xain'd Sleena (bootleg, bugfixed)", GAME_SUPPORTS_SAVE ) // newer bootleg, fixes some of the issues with the other one
