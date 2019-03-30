// license: BSD-3-Clause
// copyright-holders: Pierpaolo Prazzoli, David Haywood, Dirk Best
/***************************************************************************

    Little Casino
    Mini Vegas 4in1

    Non-Payout 'Gambling' style games.

    TODO:
    - color version of the first version is undumped?
    - figure out the rest of the dipswitches
    - keyboard


Mini Vegas
CC-089  (c) Entertainment Enterprises Ltd. 1983
 +---------------------------------------------------------------------+
 |        +-+                  M2114          A.IC19         18.432MHz |
 |        |H|                                                      D1  |
 |        |D|                  M2114  M2114                            |
++  2003C |6|                                 B.IC18               +-+ |
|         |8|                  M2114  M2114                        | | |
|         |2|                                                      |9| |
|         |1|                  M2114  M2114   C.IC17               |9| |
|         +-+                                                      |3| |
|         +-+                         M2114                        |7| |
|   DSW3  |H|                         TC5514  D.IC16               | | |
|         |D|       +-+                                            +-+ |
|         |6|       | |                                            +-+ |
|         |8|       |8|               TC5514  E.IC15               |R| |
|         |2|       |9|                                            |6| |
|         |1|       |1|                                            |5| |
++        +-+       |0|               TC5514  F.IC14               |0| |
 |                  | |                                            |2| |
 |    VR1     DSW2  +-+                                            +-+ |
 |  MB3712    DSW1  BAT               TC5514  G.IC13                   |
 +---------------------------------------------------------------------+

  CPU: Rockwell R6502AP 2MHz
Video: TMS9937 NL CRT5037 NMOS Single chip Video Timer/Controller (VTC)
Sound: AY-3-8910
       MB3712 5.7W Amp
  OSC: 18.432MHz
  RAM: M2114-3 1KBx4 SRAM x 8
       TC5514P-1 1KBx4 SRAM x 4
  DSW: 3 8-switch dipswitches
  VR1: Volume pot
Other: Hitachi HD46821P 1MHz NMOS Peripheral Interface Adapter (PIA) x 2
       NEC uPA2003C Darlington Array
       3.6v Battery
       D1 - Power On Diode
       44 pin edge connector


***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "video/tms9927.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "ltcasino.lh"
#include "ltcasinn.lh"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ltcasino_state : public driver_device
{
public:
	ltcasino_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia(*this, "pia%u", 0U),
		m_vtc(*this, "vtc"),
		m_video_ram(*this, "video_ram"),
		m_attribute_ram(*this, "attribute_ram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_lamps(*this, "button_%u", 0U),
		m_tilemap(nullptr)
	{ }

	void init_mv4in1();

	void ltcasino(machine_config &config);
	void ltcasinn(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<pia6821_device, 2> m_pia;
	required_device<crt5037_device> m_vtc;
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_attribute_ram;
	required_device<gfxdecode_device> m_gfxdecode;
	output_finder<5> m_lamps;

	tilemap_t *m_tilemap;

	void main_map(address_map &map);

	void ltcasinn_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(ltcasino_tile_info);
	TILE_GET_INFO_MEMBER(ltcasinn_tile_info);

	void machine_start_ltcasino();
	void machine_start_ltcasinn();

	DECLARE_WRITE8_MEMBER(output_r_w);
	DECLARE_WRITE8_MEMBER(output_t_w);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ltcasino_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).ram();
	map(0x8000, 0xcfff).rom();
	map(0xd000, 0xd7ff).ram().share(m_video_ram);
	map(0xe000, 0xe7ff).ram().share(m_attribute_ram);
	map(0xec00, 0xec03).rw("pia0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xec10, 0xec13).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xec20, 0xec21).r("aysnd", FUNC(ay8910_device::data_r));
	map(0xec20, 0xec21).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0xec30, 0xec3f).rw("vtc", FUNC(crt5037_device::read), FUNC(crt5037_device::write));
	map(0xf000, 0xffff).rom();
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( ltcasino )
	PORT_START("COIN")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1) PORT_WRITE_LINE_DEVICE_MEMBER("pia0", pia6821_device, ca1_w)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN2) PORT_WRITE_LINE_DEVICE_MEMBER("pia0", pia6821_device, cb1_w)

	PORT_START("Q")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1) PORT_NAME("Button 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2) PORT_NAME("Button 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3) PORT_NAME("Button 3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4) PORT_NAME("Button 4")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5) PORT_NAME("Button 5")
	PORT_DIPNAME(0x20, 0x00, "Enable Craps") // off to enter service
	PORT_DIPSETTING(   0x20, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x40, 0x00, "Enable Poker") // off to enter service
	PORT_DIPSETTING(   0x40, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x80, 0x00, "Enable Black Jack") // off to enter service
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))

	PORT_START("S")
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "DSW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "DSW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "DSW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "DSW3:5")
	PORT_DIPNAME(0x60, 0x00, DEF_STR( Service_Mode ))
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x20, DEF_STR( Off ))
	PORT_DIPSETTING(   0x40, DEF_STR( Off ))
	PORT_DIPSETTING(   0x60, DEF_STR( On ))
	PORT_DIPLOCATION("DSW3:6,7")
	PORT_DIPNAME(0x80, 0x00, DEF_STR( Demo_Sounds ))
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPLOCATION("DSW3:8")

	PORT_START("A")
	PORT_DIPNAME(0x03, 0x03, "Hands to Start")
	PORT_DIPSETTING(   0x03, "3")
	PORT_DIPSETTING(   0x02, "4")
	PORT_DIPSETTING(   0x01, "5")
	PORT_DIPSETTING(   0x00, "6")
	PORT_DIPLOCATION("A:1,2")
	PORT_DIPNAME(0x0c, 0x0c, DEF_STR( Coinage ))
	PORT_DIPSETTING(   0x00, DEF_STR( 2C_2C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_1C ))
	PORT_DIPLOCATION("A:3,4")
	PORT_DIPNAME(0x10, 0x10, "Coin Limit")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x10, DEF_STR( On )) // limits to 15 coins
	PORT_DIPLOCATION("A:5")
	PORT_DIPNAME(0x20, 0x00, DEF_STR( Unknown ))
	PORT_DIPSETTING(   0x00, DEF_STR( On )) // needs to be 0 or ltcasinn can reset on coin-up
	PORT_DIPSETTING(   0x20, DEF_STR( Off ))
	PORT_DIPLOCATION("A:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "A:7")
	PORT_DIPNAME(0x80, 0x80, "Screen Mode")
	PORT_DIPSETTING(   0x80, "60 Hz")
	PORT_DIPSETTING(   0x00, "50 Hz")
	PORT_DIPLOCATION("A:8")

	PORT_START("B")
	PORT_DIPNAME(0x01, 0x01, "Show High Scores") // When it's OFF, it doesn't show or allow High Scores
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPLOCATION("B:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "B:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "B:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "B:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "B:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "B:6")
	PORT_DIPNAME(0x40, 0x40, "Bonus Hand")
	PORT_DIPSETTING(   0x40, "100000 & 200000")
	PORT_DIPSETTING(   0x00, DEF_STR( None ))
	PORT_DIPLOCATION("B:7")
	PORT_DIPNAME(0x80, 0x80, "Demo") // some kind of demo play?
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPLOCATION("B:8")
INPUT_PORTS_END

static INPUT_PORTS_START( ltcasinn )
	PORT_INCLUDE(ltcasino)

	PORT_MODIFY("COIN")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_MODIFY("B")
	PORT_DIPNAME(0x02, 0x02, "Memory Test") // tests d000 to d7ff
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x02, DEF_STR( On ))
	PORT_DIPLOCATION("B:2")

	PORT_MODIFY("S")
	PORT_DIPNAME(0x01, 0x01, "Keyboard")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPLOCATION("DSW3:1")
INPUT_PORTS_END

static INPUT_PORTS_START( mv4in1 )
	PORT_INCLUDE(ltcasino)

	PORT_MODIFY("Q")
	PORT_DIPNAME(0x20, 0x00, "Enable Dice") // must be off to enter service
	PORT_DIPSETTING(   0x20, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x40, 0x00, "Enable Poker") // must be off to enter service
	PORT_DIPSETTING(   0x40, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPNAME(0x80, 0x00, "Enable 21") // must be off to enter service
	PORT_DIPSETTING(   0x80, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))

	PORT_MODIFY("S")
	PORT_DIPNAME(0x01, 0x01, "Keyboard")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPLOCATION("DSW3:1")
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void ltcasino_state::ltcasinn_palette(palette_device &palette) const
{
	for (int i = 0; i < 8; i++)
	{
		// basic 3 bit palette
		palette.set_indirect_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 2), pal1bit(i >> 1)));

		// setup pen colors for the drawgfx system
		for (int j = 0; j < 8; j++)
		{
			palette.set_pen_indirect((j << 4) | (i << 1) | 0, i);
			palette.set_pen_indirect((j << 4) | (i << 1) | 1, j);
		}
	}
}

/*
 x--- ---- tile bank
 -xxx ---- foreground color
 ---- x--- unknown (used by ltcasino)
 ---- -xxx background color
 */
TILE_GET_INFO_MEMBER(ltcasino_state::ltcasino_tile_info)
{
	uint16_t code = m_video_ram[tile_index];
	// +1 on attribute offset otherwise glitches occurs on left side of objects?
	uint8_t attr = m_attribute_ram[(tile_index + 1) & 0x7ff];

	code |= BIT(attr, 7) << 8;

	SET_TILE_INFO_MEMBER(0, code, 0, 0);
}

TILE_GET_INFO_MEMBER(ltcasino_state::ltcasinn_tile_info)
{
	uint16_t code = m_video_ram[tile_index];
	// +1 on attribute offset otherwise glitches occurs on left side of objects?
	uint8_t attr = m_attribute_ram[(tile_index + 1) & 0x7ff];

	code |= BIT(attr, 7) << 8;

	SET_TILE_INFO_MEMBER(0, code, ((attr & 0x70) >> 1) | (attr & 7), 0);
}

uint32_t ltcasino_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->mark_all_dirty();
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_ltcasino )
	GFXDECODE_ENTRY("gfx1", 0, tiles8x8_layout, 0, 1)
GFXDECODE_END

static GFXDECODE_START( gfx_ltcasinn )
	GFXDECODE_ENTRY("gfx1", 0, tiles8x8_layout, 0, 64)
GFXDECODE_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ltcasino_state::init_mv4in1()
{
	uint8_t *rom = memregion("maincpu")->base();
	for (int i = 0; i < 0x10000; i++)
		rom[i] = bitswap<8>(rom[i], 7, 6, 5, 4, 3, 1, 2, 0);
}

void ltcasino_state::machine_start_ltcasino()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ltcasino_state::ltcasino_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_lamps.resolve();
}

void ltcasino_state::machine_start_ltcasinn()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(ltcasino_state::ltcasinn_tile_info), this), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_lamps.resolve();
}

WRITE8_MEMBER(ltcasino_state::output_r_w)
{
	// 7------- unknown (toggles rapidly)
	// -6------ unknown (toggles rapidly)
	// --5----- coin counter
	// ---43210 button lamps 5 to 1

	m_lamps[0] = BIT(data, 0); // button 1
	m_lamps[1] = BIT(data, 1); // button 2
	m_lamps[2] = BIT(data, 2); // button 3
	m_lamps[3] = BIT(data, 3); // button 4
	m_lamps[4] = BIT(data, 4); // button 5

	machine().bookkeeping().coin_counter_w(0, BIT(data, 5));
}

WRITE8_MEMBER(ltcasino_state::output_t_w)
{
	// 76543210 unknown

	logerror("output_t_w: %02x\n", data);
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void ltcasino_state::ltcasino(machine_config &config)
{
	M6502(config, m_maincpu, 18.432_MHz_XTAL/16); // clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ltcasino_state::main_map);

	PIA6821(config, m_pia[0], 0);
	m_pia[0]->readpa_handler().set_ioport("Q");
	m_pia[0]->writepb_handler().set(FUNC(ltcasino_state::output_r_w));

	PIA6821(config, m_pia[1], 0);
	m_pia[1]->readpa_handler().set_ioport("S");
	m_pia[1]->writepb_handler().set(FUNC(ltcasino_state::output_t_w));

	MCFG_MACHINE_START_OVERRIDE(ltcasino_state, ltcasino)

	// video hardware
	CRT5037(config, m_vtc, 18.432_MHz_XTAL/16); // this clock gives about 61/51 hz
	m_vtc->set_char_width(8);
	m_vtc->set_screen("screen");
	m_vtc->set_visarea(48, 463, 0, 255);
	m_vtc->vsyn_callback().set_inputline("maincpu", 0); // ?
	m_vtc->vsyn_callback().append(m_pia[0], FUNC(pia6821_device::cb2_w)); // ?

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL/2, 560, 48, 464, 268, 0, 256);
	screen.set_screen_update(FUNC(ltcasino_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_ltcasino);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 18.432_MHz_XTAL/16)); // clock unknown
	aysnd.port_a_read_callback().set_ioport("A");
	aysnd.port_b_read_callback().set_ioport("B");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.4);
}

void ltcasino_state::ltcasinn(machine_config &config)
{
	ltcasino(config);

	MCFG_MACHINE_START_OVERRIDE(ltcasino_state, ltcasinn)

	config.device_remove("palette");
	PALETTE(config, "palette", FUNC(ltcasino_state::ltcasinn_palette), 128, 8);

	m_gfxdecode->set_info(gfx_ltcasinn);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ltcasino )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "a", 0x8000, 0x1000, CRC(14909fee) SHA1(bf53fa65da7f013ea1ac6b4942cdfdb34ef16252) )
	ROM_LOAD( "b", 0x9800, 0x0800, CRC(1473f854) SHA1(eadaec1f6d653e61458bc262945c20140f4530eb) )
	ROM_LOAD( "c", 0xa800, 0x0800, CRC(7a07004b) SHA1(62bd0f3d12b7eada6fc271abea60569aca7262b0) )
	ROM_LOAD( "d", 0xb800, 0x0800, CRC(5148cafc) SHA1(124039f48784bf032f612714db73fb67a216a1e7) )
	ROM_LOAD( "e", 0xc800, 0x0800, CRC(5f9e103a) SHA1(b0e9ace4c3962c06e5250fac16a245dca711350f) )
	ROM_LOAD( "f", 0xf000, 0x1000, CRC(7345aada) SHA1(6640f5eb1130c8f1cb197eb12b8e6403c7f8d34d) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "v", 0x0000, 0x0800, CRC(f1f75675) SHA1(8f3777e6b2a3f824f94b28669cac501ec02bbf36) )
ROM_END

ROM_START( ltcasinn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc2_ra.bin", 0x8000, 0x1000, CRC(1a595442) SHA1(b8fe3e5ed2024a57187c0ce547c1bbef2429ed63) )
	ROM_LOAD( "lc2_rb.bin", 0x9000, 0x1000, CRC(4f5502c1) SHA1(cd1b7c08d26fed71c45e44ebd208bd18dc262e8f) )
	ROM_LOAD( "lc2_rc.bin", 0xa000, 0x1000, CRC(990283b8) SHA1(8a3fe5be8381894b8e8dd14c7d42190e60a25600) )
	ROM_LOAD( "lc2_rd.bin", 0xb000, 0x1000, CRC(884f39dc) SHA1(fe149faf118279205e82760c5052cefb88a2f5be) )
	ROM_LOAD( "lc2_re.bin", 0xc000, 0x1000, CRC(fae38204) SHA1(e5908734cee0a89d873ab3761ded285f8ae138d3) )
	ROM_LOAD( "lc2_rf.bin", 0xf000, 0x1000, CRC(7e8ad9d3) SHA1(8cbe342af7d9f32b2214664db318edd3d2e75630) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "lc2_rv.bin", 0x0000, 0x1000, CRC(84cbee7b) SHA1(742831d5ae0db6c7c644a18a837831ee0474d472) )
ROM_END

ROM_START( mv4in1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "g.ic13", 0x8000, 0x1000, CRC(ac33bd85) SHA1(fd555f70d0a7040473d35ec38e19185671a471ea) )
	ROM_LOAD( "f.ic14", 0x9000, 0x1000, CRC(f95c87d1) SHA1(df5ed53722ec55a97eabe10b0ed3f1ba32cbe55f) )
	ROM_LOAD( "e.ic15", 0xa000, 0x1000, CRC(e525fcf2) SHA1(f1ec0c514e25ec4a1caf737ff8a962c81fb2706a) )
	ROM_LOAD( "d.ic16", 0xb000, 0x1000, CRC(ab34673f) SHA1(520a173a342a27b5f9d995e6f53c3a2f0f359f9e) )
	ROM_LOAD( "c.ic17", 0xc000, 0x1000, CRC(e384edf4) SHA1(99042528ce2b35191248d90162ca06a1a585667c) )
	ROM_LOAD( "b.ic18", 0xf000, 0x1000, CRC(3450b862) SHA1(816d13fd8d03c299c1dbecf971ee5fae2f1d64bc) )

	ROM_REGION( 0x1000, "gfx1", 0 )
	ROM_LOAD( "a.ic19", 0x0000, 0x1000, CRC(a25c125e) SHA1(e0ba83ccddbd82a2bf52585ae0accb9192cbb00e) )
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//     YEAR  NAME      PARENT    MACHINE   INPUT     CLASS           INIT         ROTATION  COMPANY                            FULLNAME                 FLAGS
GAMEL( 1982, ltcasino, 0,        ltcasino, ltcasino, ltcasino_state, empty_init,  ROT0,     "Digital Controls Inc.",           "Little Casino (older)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK, layout_ltcasino )
GAMEL( 1983, mv4in1,   ltcasino, ltcasinn, mv4in1,   ltcasino_state, init_mv4in1, ROT0,     "Entertainment Enterprises, Ltd.", "Mini Vegas 4in1",       MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK, layout_ltcasinn )
GAMEL( 1984, ltcasinn, 0,        ltcasinn, ltcasinn, ltcasino_state, empty_init,  ROT0,     "Digital Controls Inc.",           "Little Casino (newer)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK, layout_ltcasinn )
