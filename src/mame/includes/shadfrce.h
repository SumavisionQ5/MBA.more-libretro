/**************************/
/*	SHADOW FORCE	  */
/**************************/

class shadfrce_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine)
	{
		return auto_alloc_clear(&machine, shadfrce_state(machine));
	}

	shadfrce_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* devices */
	running_device	*maincpu;
	running_device	*audiocpu;

	/* memory pointers */
	UINT16		*fgvideoram;
	UINT16		*bg0videoram;
	UINT16		*bg1videoram;
	UINT16		*spvideoram;
	UINT16		*spvideoram_old;

	/* memory size */
	size_t		spvideoram_size;

	/* video-related */
	tilemap_t	*fgtilemap;
	tilemap_t	*bg0tilemap;
	tilemap_t	*bg1tilemap;
	INT32		raster_scanline;
	INT32		raster_irq_enable;
	INT32		video_enable;
	INT32		vblank;

	/* misc */
	INT32		irqs_enable;
	INT32		prev_value;
};


/*----------- defined in video/shadfrce.c -----------*/

WRITE16_HANDLER( shadfrce_bg0scrollx_w );
WRITE16_HANDLER( shadfrce_bg1scrollx_w );
WRITE16_HANDLER( shadfrce_bg0scrolly_w );
WRITE16_HANDLER( shadfrce_bg1scrolly_w );

VIDEO_START( shadfrce );
VIDEO_EOF(shadfrce);
VIDEO_UPDATE( shadfrce );

WRITE16_HANDLER( shadfrce_fgvideoram_w );
WRITE16_HANDLER( shadfrce_bg0videoram_w );
WRITE16_HANDLER( shadfrce_bg1videoram_w );
