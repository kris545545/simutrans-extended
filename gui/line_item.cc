/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#include <algorithm>

#include "line_item.h"
#include "../simline.h"
#include "../simmenu.h"
#include "../simworld.h"
#include "../utils/cbuffer_t.h"


const char* line_scrollitem_t::get_text() const
{
	return line->get_name();
}


PIXVAL line_scrollitem_t::get_color() const
{
	return line->get_state_color();
}


void line_scrollitem_t::set_text(char const* const t)
{
	if(  t  &&  t[0]  &&  strcmp(t, line->get_name())) {
		// text changed => call tool
		cbuffer_t buf;
		buf.printf( "l%u,%s", line.get_id(), t );
		tool_t *tool = create_tool( TOOL_RENAME | SIMPLE_TOOL );
		tool->set_default_param( buf );
		karte_ptr_t welt;
		welt->set_tool( tool, line->get_owner() );
		// since init always returns false, it is safe to delete immediately
		delete tool;
	}
}


// static helper function for sorting lineintems

line_scrollitem_t::sort_modes_t line_scrollitem_t::sort_mode = line_scrollitem_t::SORT_BY_NAME;


bool line_scrollitem_t::compare(const gui_component_t *aa, const gui_component_t *bb)
{
	const line_scrollitem_t *a = dynamic_cast<const line_scrollitem_t*>(aa);
	const line_scrollitem_t *b = dynamic_cast<const line_scrollitem_t*>(bb);
	// good luck with mixed lists
	assert(a != NULL && b != NULL); (void)(a == b);

	if(  sort_mode != SORT_BY_NAME  ) {
		switch(  sort_mode  ) {
			case SORT_BY_NAME: // default
				break;
			case SORT_BY_ID:
				return (a->get_line().get_id() - b->get_line().get_id())<0;
			case SORT_BY_PROFIT:
				return (a->get_line()->get_finance_history(1,LINE_PROFIT) - b->get_line()->get_finance_history(1,LINE_PROFIT))<0;
			case SORT_BY_TRANSPORTED:
				return (a->get_line()->get_finance_history(1,LINE_PAX_DISTANCE) - b->get_line()->get_finance_history(1,LINE_PAX_DISTANCE))<0;
			case SORT_BY_CONVOIS:
				return (a->get_line()->get_finance_history(1,LINE_CONVOIS) - b->get_line()->get_finance_history(1,LINE_CONVOIS))<0;
			case SORT_BY_DISTANCE:
				// normalizing to the number of convoys to get the fastest ones ...
				return (a->get_line()->get_finance_history(1,LINE_DISTANCE)/max(1,a->get_line()->get_finance_history(1,LINE_CONVOIS)) -
						b->get_line()->get_finance_history(1,LINE_DISTANCE)/max(1,b->get_line()->get_finance_history(1,LINE_CONVOIS)) )<0;
			default: break;
		}
		// default sorting ...
	}

	// first: try to sort by line letter code
	const char *alcl = a->get_line()->get_linecode_l();
	const char *blcl = b->get_line()->get_linecode_l();
	if (strcmp(alcl, blcl)) {
		return strcmp(alcl, blcl)<0;
	}
	const char *alcr = a->get_line()->get_linecode_r();
	const char *blcr = b->get_line()->get_linecode_r();
	if (strcmp(alcr, blcr)) {
		return strcmp(alcr, blcr)<0;
	}

	// second: try to sort by number
	const char *atxt = a->get_text();
	int aint = 0;
	// isdigit produces with UTF8 assertions ...
	if(  atxt[0]>='0'  &&  atxt[0]<='9'  ) {
		aint = atoi( atxt );
	}
	else if(  atxt[0]=='('  &&  atxt[1]>='0'  &&  atxt[1]<='9'  ) {
		aint = atoi( atxt+1 );
	}
	const char *btxt = b->get_text();
	int bint = 0;
	if(  btxt[0]>='0'  &&  btxt[0]<='9'  ) {
		bint = atoi( btxt );
	}
	else if(  btxt[0]=='('  &&  btxt[1]>='0'  &&  btxt[1]<='9'  ) {
		bint = atoi( btxt+1 );
	}
	if(  aint!=bint  ) {
		return (aint-bint)<0;
	}
	// otherwise: sort by name
	return strcmp(atxt, btxt)<0;
}


void line_scrollitem_t::draw(scr_coord pos)
{
	pos += get_pos();
	scr_coord_val left = D_H_SPACE;
	if (selected) {
		// selected element
		display_fillbox_wh_clip_rgb(pos.x, pos.y - 1, get_size().w, get_size().h + 1, (focused ? SYSCOL_LIST_BACKGROUND_SELECTED_F : SYSCOL_LIST_BACKGROUND_SELECTED_NF), true);
	}

	// line color
	if (line->get_line_color_index()!=255) {
		PIXVAL line_color= line->get_line_color();
		left+=display_line_lettercode_rgb(pos.x, pos.y, line_color, line->get_line_lettercode_style(), line->get_linecode_l(), line->get_linecode_r());
	}

	// text
	left += display_proportional_clip_rgb(pos.x + left, pos.y+D_GET_CENTER_ALIGN_OFFSET(LINEASCENT, size.h), get_text(), ALIGN_LEFT, selected ? (focused ? SYSCOL_LIST_TEXT_SELECTED_FOCUS : SYSCOL_LIST_TEXT_SELECTED_NOFOCUS) : get_color(), true);

	// symbols
	pos.y += D_GET_CENTER_ALIGN_OFFSET(D_FIXED_SYMBOL_WIDTH, size.h)+1;
	const uint8 symbol_interval = D_FIXED_SYMBOL_WIDTH+2;
	if (line->get_state() & simline_t::line_has_stuck_convoy && skinverwaltung_t::pax_evaluation_icons) {
		left = max(left,get_size().w - symbol_interval*4);
		display_color_img(skinverwaltung_t::pax_evaluation_icons->get_image_id(4), pos.x + left, pos.y, 0, false, false);
		left += symbol_interval;
	}
	else if (line->get_state() & simline_t::line_nothing_moved && skinverwaltung_t::alerts) {
		left = max(left,get_size().w - symbol_interval*4);
		display_color_img(skinverwaltung_t::alerts->get_image_id(2), pos.x + left, pos.y, 0, false, false);
		left += symbol_interval;
	}
	else if (line->get_state() & simline_t::line_no_convoys && skinverwaltung_t::alerts) {
		left = max(left,get_size().w - symbol_interval*4);
		display_color_img(skinverwaltung_t::alerts->get_image_id(1), pos.x + left, pos.y, 0, false, false);
		left += symbol_interval;
	}
	if (line->get_state() & simline_t::line_overcrowded && skinverwaltung_t::pax_evaluation_icons) {
		left = max(left,get_size().w - symbol_interval*3);
		display_color_img(skinverwaltung_t::pax_evaluation_icons->get_image_id(1), pos.x + left, pos.y, 0, false, false);
		left += symbol_interval;
	}
	if (line->get_state() & simline_t::line_missing_scheduled_slots && skinverwaltung_t::missing_scheduled_slot) {
		left = max(left,get_size().w - symbol_interval*2);
		display_color_img(skinverwaltung_t::missing_scheduled_slot->get_image_id(0), pos.x + left, pos.y, 0, false, false);
		left += symbol_interval;
	}
	if (line->get_state() & simline_t::line_has_upgradeable_vehicles && skinverwaltung_t::upgradable) {
		left = max(left,get_size().w - symbol_interval);
		display_color_img(skinverwaltung_t::upgradable->get_image_id(1), pos.x + left, pos.y, 0, false, false);
	}
}

scr_size line_scrollitem_t::get_min_size() const
{
	return scr_size(size.w, size.h);
}

scr_size line_scrollitem_t::get_max_size() const
{
	return scr_size(scr_size::inf.w, size.h);
}
