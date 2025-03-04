/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef GUI_CONVOI_INFO_T_H
#define GUI_CONVOI_INFO_T_H


#include "gui_frame.h"
#include "components/gui_scrollpane.h"
#include "components/gui_textarea.h"
#include "components/gui_textinput.h"
#include "components/gui_speedbar.h"
#include "components/gui_button.h"
#include "components/gui_label.h"
#include "components/gui_chart.h"
#include "components/gui_obj_view_t.h"
#include "components/action_listener.h"
#include "components/gui_tab_panel.h"
#include "components/gui_button_to_chart.h"
#include "components/gui_combobox.h"
#include "components/gui_image.h"
#include "../convoihandle_t.h"
#include "simwin.h"

#include "../utils/cbuffer_t.h"
#include "components/gui_convoy_payloadinfo.h"
#include "components/gui_schedule_item.h"

#include "times_history_container.h"
#include "components/gui_colorbox.h"
#include "components/gui_line_lettercode.h"

#define BUTTON_COUNT convoi_t::MAX_CONVOI_COST


/**
 * Displays an information window for a convoi
 */
class convoi_info_t : public gui_frame_t, private action_listener_t
{
public:
	enum sort_mode_t {
		by_destination          = 0,
		by_via                  = 1,
		by_amount_via           = 2,
		by_amount               = 3,
		by_origin               = 4,
		by_origin_sum           = 5,
		by_destination_detail   = 6,
		by_wealth_detail        = 7,
		by_wealth_via           = 8,
		by_accommodation_detail = 9,
		by_accommodation_via    = 10,
		SORT_MODES              = 11
	};

private:
	/**
	* Buffer for freight info text string.
	*/
	cbuffer_t freight_info;

	gui_textarea_t text;
	obj_view_t view;
	gui_label_buf_t speed_label, profit_label, running_cost_label, weight_label, target_label, line_label;
	gui_label_buf_t distance_label, lb_working_method;
	gui_textinput_t input;
	gui_loadingbar_t loading_bar;
	gui_speedbar_t speed_bar;
	gui_routebar_t route_bar;
	sint32 next_reservation_index;
	gui_chart_t chart;
	gui_image_t img_reverse_route;

	button_t button;
	button_t follow_button;
	button_t go_home_button;
	button_t no_load_button;
	button_t replace_button;

	button_t details_button;
	button_t reverse_button;

	gui_aligned_container_t next_halt_cells;
	gui_schedule_entry_number_t next_halt_number;

	gui_times_history_t cont_times_history;

	static sint16 tabstate;
	gui_tab_panel_t switch_mode;
	gui_aligned_container_t container_freight, container_stats, container_line, *container_top;
	gui_scrollpane_t scroll_freight, scroll_times_history;

	gui_combobox_t freight_sort_selector;
	button_t line_button; // goto line ...
	bool line_bound;
	gui_line_lettercode_t lc_preview;

	convoihandle_t cnv;
	sint32 mean_convoi_speed;
	sint32 max_convoi_speed;

	// current pointer to route ...
	sint32 cnv_route_index;

	char cnv_name[256],old_cnv_name[256];

	void update_labels();

	// resets textinput to current convoi name
	// necessary after convoi was renamed
	void reset_cnv_name();

	// rename selected convoi
	// checks if possible / necessary
	void rename_cnv();

	//static bool route_search_in_progress;

	static const char *sort_text[SORT_MODES];

	gui_button_to_chart_array_t button_to_chart;

	void init(convoihandle_t cnv);

	void set_tab_opened();

public:
	convoi_info_t(convoihandle_t cnv = convoihandle_t());

	virtual ~convoi_info_t();

	/**
	 * Set the window associated helptext
	 * @return the filename for the helptext, or NULL
	 */
	const char * get_help_filename() const OVERRIDE { return "convoiinfo.txt"; }

	/**
	 * Draw new component. The values to be passed refer to the window
	 * i.e. It's the screen coordinates of the window where the
	 * component is displayed.
	 */
	void draw(scr_coord pos, scr_size size) OVERRIDE;

	bool is_weltpos() OVERRIDE;

	koord3d get_weltpos( bool set ) OVERRIDE;

	bool action_triggered(gui_action_creator_t*, value_t) OVERRIDE;

	/**
	 * called when convoi was renamed
	 */
	void update_data() { reset_cnv_name(); set_dirty(); }

	bool infowin_event(const event_t *ev) OVERRIDE;

	void rdwr( loadsave_t *file ) OVERRIDE;

	uint32 get_rdwr_id() OVERRIDE { return magic_convoi_info; }
};

#endif
