/****************************************/
/*	Technos - WWF Super Stars	*/
/****************************************/

class wwfsstar_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine)
	{
		return auto_alloc_clear(&machine, wwfsstar_state(machine));
	}

	wwfsstar_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* devices */
	running_device	*maincpu;
	running_device	*audiocpu;

	/* memory pointers */
	UINT16		*spriteram;
	UINT16		*fg0_videoram;
	UINT16		*bg0_videoram;

	/* video-related */
	tilemap_t	*fg0_tilemap;
	tilemap_t	*bg0_tilemap;
	INT32		vblank;
	INT32		scrollx;
	INT32		scrolly;
};


/*----------- defined in video/wwfsstar.c -----------*/

WRITE16_HANDLER( wwfsstar_fg0_videoram_w );
WRITE16_HANDLER( wwfsstar_bg0_videoram_w );

VIDEO_START( wwfsstar );
VIDEO_UPDATE( wwfsstar );
