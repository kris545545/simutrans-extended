/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef GUI_REPLACE_FRAME_H
#define GUI_REPLACE_FRAME_H


#include "gui_frame.h"

#include "../simconvoi.h"

#include "components/action_listener.h"
#include "components/gui_button.h"
#include "components/gui_convoy_assembler.h"
#include "components/gui_label.h"
#include "components/gui_numberinput.h"

#include "../dataobj/replace_data.h"

#include "../utils/cbuffer_t.h"

/**
 * Replace frame, makes convoys be marked for replacing.
 */
class replace_frame_t : public gui_frame_t,
                        public action_listener_t
{
private:
	/**
	 * The convoy to be replaced
	 */
	convoihandle_t cnv;

	cbuffer_t title_buf;

	bool replace_line;	// True if all convoys like this in its line are to be replaced
	bool replace_all;	// True if all convoys like this are to be replaced
	bool depot;		// True if convoy is to be sent to depot only
	replace_data_t *rpl;
	enum {state_replace=0, state_sell, state_skip, n_states};
	uint8 state;
	uint16 replaced_so_far;
	sint64 money;

	bool copy;
	convoihandle_t master_convoy;

	/**
	 * Gui elements
	 */
	button_t bt_details;
	// main action buttons
	button_t bt_autostart;
	button_t bt_depot;
	button_t bt_mark;
	gui_combobox_t cb_replace_target;
	button_t bt_reset;
	button_t bt_clear;
	// option buttons
	button_t bt_retain_in_depot;
	button_t bt_use_home_depot;
	button_t bt_allow_using_existing_vehicles;
	// inputs
	gui_label_t lb_inp[n_states];
	gui_numberinput_t numinp[n_states];
	gui_label_buf_t lb_text[n_states];

	gui_label_buf_t lb_money;

	vector_tpl<gui_image_list_t::image_data_t*> current_convoi_pics;
	gui_image_list_t current_convoi;
	gui_scrollpane_t scrollx_convoi;

	//gui_numberinput_t	numinp[n_states];
	gui_convoy_assembler_t convoy_assembler;

	// Some helper functions
	bool replace_convoy(convoihandle_t cnv, bool mark);
	inline void start_replacing() {state=state_replace; replaced_so_far=0;}
	uint8 get_present_state();

	sint64 calc_total_cost();

	void init();
	void init_table();

	// registered convoy vehicles to assembler and convoy(only init).
	void set_vehicles(bool init=false);

public:
	replace_frame_t(convoihandle_t cnv = convoihandle_t());

	// This is also called when the convoy name is changed.
	void set_title();

	/**
	 * Update texts, image lists and buttons according to the current state.
	 */
	void update_data();

	/**
	 * Set the window size
	 */
	void set_windowsize(scr_size size) OVERRIDE;

	/**
	 * Set the window associated helptext
	 * @return the filename for the helptext, or NULL
	 */
	const char * get_help_filename() const OVERRIDE {return "replace.txt";}

	//bool infowin_event(const event_t *ev) OVERRIDE;

	void draw(scr_coord pos, scr_size gr) OVERRIDE;

	bool action_triggered( gui_action_creator_t *comp, value_t extra) OVERRIDE;

	const convoihandle_t get_convoy() const { return cnv; }

	virtual ~replace_frame_t();

	// for reload from the save
	void set_convoy(convoihandle_t cnv) { this->cnv=cnv; init(); }
	uint32 get_rdwr_id() OVERRIDE;

	void rdwr(loadsave_t *) OVERRIDE;
};

#endif
