/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef GUI_MAP_FRAME_H
#define GUI_MAP_FRAME_H


#include "gui_frame.h"
#include "simwin.h"
#include "components/gui_scrollpane.h"
#include "components/action_listener.h"
#include "components/gui_button.h"
#include "components/gui_combobox.h"
#include "components/gui_label.h"
#include "../tpl/stringhashtable_tpl.h"
#include "../player/simplay.h"
#include "../simline.h"

class karte_ptr_t;

#define MAP_MAX_BUTTONS (31)


/**
 * Scroll-container of map, so we can rightdrag and zoom with wheel
 */
class gui_scrollpane_map_t : public gui_scrollpane_t
{
private:
	bool is_dragging;
	bool is_cursor_hidden;

public:
	gui_scrollpane_map_t(gui_component_t* comp);

	// we use rightclick dragging and scrollwhell for zoom, so we need to catch some events before
	bool infowin_event(event_t const*) OVERRIDE;

	scr_size get_max_size() const OVERRIDE { return scr_size::inf; }

	void zoom(bool magnify);
};

/**
 * Minimap window
 */
class map_frame_t :
	public gui_frame_t,
	public action_listener_t
{
private:
	static karte_ptr_t welt;

	/**
	 * This is kind of hack: we know there can only be one map frame
	 * at a time, and we want to save the current size for the next object
	 * so we use a static variable here.
	 */
	static scr_size window_size;

	static bool legend_visible;
	static bool network_option_visible;
	static bool scale_visible;

	cbuffer_t title_buf;
	convoihandle_t selected_cnv = convoihandle_t(); // for title name cache

	int viewable_players[MAX_PLAYER_COUNT+1];

	vector_tpl<const goods_desc_t *> viewable_freight_types;

	gui_aligned_container_t filter_container, network_filter_container, scale_container, *zoom_row;

	gui_scrollpane_map_t* p_scrolly;

	button_t filter_buttons[MAP_MAX_BUTTONS];
	button_t zoom_buttons[2];
	button_t b_rotate45;
	button_t b_show_legend;
	button_t b_show_network_option;
	button_t b_show_scale;
	button_t b_show_directory;
	button_t b_overlay_networks;
	button_t b_overlay_networks_load_factor;
	button_t b_show_contour;
	button_t b_show_buildings;

	gui_label_buf_t zoom_value_label;
	gui_label_buf_t tile_scale_label;

	gui_combobox_t viewed_player_c;
	gui_combobox_t transport_type_c;
	gui_combobox_t freight_type_c;

//	void zoom(bool zoom_out);
	void update_buttons();
	void show_hide_legend(const bool show);
	void show_hide_network_option(const bool show);
	void show_hide_scale(const bool show);

public:
	/**
	 * Set the window associated helptext
	 * @return the filename for the helptext, or NULL
	 */
	const char *get_help_filename() const OVERRIDE {return "map.txt";}

	static bool zoomed; // if true, zoom label will be uopdated on next redraw
	static scr_coord screenpos;

	/**
	 * Constructor. Adds all necessary Subcomponents.
	 */
	map_frame_t();

	void rdwr( loadsave_t *file ) OVERRIDE;

	uint32 get_rdwr_id() OVERRIDE { return magic_reliefmap; }

	bool infowin_event(event_t const*) OVERRIDE;

	/**
	 * Sets the window sizes
	 */
	void set_windowsize(scr_size size) OVERRIDE;

	/**
	 * Draw new component. The values to be passed refer to the window
	 * i.e. It's the screen coordinates of the window where the
	 * component is displayed.
	 */
	void draw(scr_coord pos, scr_size size) OVERRIDE;

	bool action_triggered(gui_action_creator_t*, value_t) OVERRIDE;

	void set_title();

	// Launch the network map from an external dialog
	void activate_individual_network_mode(koord center_pos = koord::invalid);
	void set_halt(halthandle_t halt = halthandle_t());
};

#endif
