/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef OBJ_GEBAEUDE_H
#define OBJ_GEBAEUDE_H


#include "../ifc/sync_steppable.h"
#include "simobj.h"
#include "../simcolor.h"
#include "../tpl/minivec_tpl.h"
#include "../simworld.h"
#include "../bauer/goods_manager.h"

class building_tile_desc_t;
class fabrik_t;
class stadt_t;
class goods_desc_t;
class planquadrat_t;

/**
 * Asynchronous or synchronous animations for buildings.
 */
class gebaeude_t : public obj_t, sync_steppable
{
private:
	const building_tile_desc_t *tile;

	/**
	 * Time control for animation progress.
	 */
	uint16 anim_time;

	/**
	 * Is this a sync animated object?
	 */
	uint8 sync:1;

	/**
	 * Boolean flag if a construction site or buildings image
	 * shall be displayed.
	 */
	uint8 show_construction:1;

	/**
	 * if true, this ptr union contains a factory pointer
	 */
	uint8 is_factory:1;

	uint8 season:3;
	uint8 background_animated:1;

	uint8 remove_ground:1;  // true if ground image can go

	uint8 anim_frame;

	sint64 construction_start;  // Time in ticks. "Pit" under-construction graphics handled by sync_step()
    sint32 purchase_time;       // Date in months


	/**
	* either point to a factory or a city
	*/
	union {
		fabrik_t  *fab;
		stadt_t *stadt;
	} ptr;

	/**
	 * Initializes all variables with safe, usable values
	 */
	void init();

	/**
	 * Stores the dynamic population of this building:
	 * either as visitor demand (commercial) or population
	 * (residential). This is the fully adjusted figure,
	 * representing passengers to be generated or demanded
	 * per game month.
	 */
	union people_t
	{
		uint16 population;
		uint16 visitor_demand;
	} people;

	union adjusted_people_t
	{
		uint16 population;
		uint16 visitor_demand;
	} adjusted_people;

	/**
	 * Stores the dynamic number of jobs in this building
	 * at present. This is the fully adjusted figure,
	 * representing commuters to be demanded per game month.
	 */
	uint16 jobs;
	uint16 adjusted_jobs;

	/**
	 * Stores the dynamic level of mail demand in this building
	 * at present. This is the fully adjusted figure,
	 * representing packets of mail to be generated or demanded
	 * per game month.
	 */
	uint16 mail_demand;
	uint16 adjusted_mail_demand;

	/** The following variables record the proportion of
	 * successful passenger trips generated by this building
	 * in the current and last year respectively.
	 */
	uint16 passengers_generated_commuting;
	uint16 passengers_succeeded_commuting;
	uint16 passenger_success_percent_last_year_commuting;

	uint16 passengers_generated_visiting;
	uint16 passengers_succeeded_visiting;
	uint16 passenger_success_percent_last_year_visiting;

	uint16 mail_generated;
	uint16 mail_delivery_succeeded_last_year;
	uint16 mail_delivery_succeeded;
	uint16 mail_delivery_success_percent_last_year;

	/**
	* This is the number of jobs supplied by this building
	* multiplied by the number of ticks per month, subtracted
	* from the creation time, to which is added the number
	* of ticks per month whenever a commuter reaches this
	* destination. Further, this value is set so that,
	* whenever a number is added to it, it will never be less
	* than that number plus the number of ticks per month
	* multiplied by the number of available jobs minus
	* the current time. This is intended to prevent more
	* commuters going to this building each month than there
	* are jobs available for them.
	* @author: jamespetts
	*/
	sint64 available_jobs_by_time;

	// This is true if the building is in one or more world lists.
	sint8 is_in_world_list;

	// Whether the passenger and mail figures need recalculating or not.
	bool loaded_passenger_and_mail_figres;

	minivec_tpl<const planquadrat_t*> building_tiles;

#ifdef INLINE_OBJ_TYPE
protected:
	gebaeude_t(obj_t::typ type);
	gebaeude_t(obj_t::typ type, koord3d pos,player_t *player, const building_tile_desc_t *t);
	void init(player_t *player, const building_tile_desc_t *t);

public:
	gebaeude_t(loadsave_t *file, bool do_not_add_to_world_list);
	gebaeude_t(koord3d pos,player_t *player, const building_tile_desc_t *t);
#else
protected:
	gebaeude_t();

public:
	gebaeude_t(loadsave_t *file);
	gebaeude_t(koord3d pos,player_t *player, const building_tile_desc_t *t);
#endif
	virtual ~gebaeude_t();

	void rotate90() OVERRIDE;

	void add_alter(sint64 a);

	void set_fab(fabrik_t *fd);
	void set_stadt(stadt_t *s);

	fabrik_t* get_fabrik() const { return is_factory ? ptr.fab : NULL; }
	stadt_t* get_stadt() const;

#ifdef INLINE_OBJ_TYPE
#else
	obj_t::typ get_typ() const { return obj_t::gebaeude; }
#endif

	/**
	 * waytype associated with this object
	 */
	waytype_t get_waytype() const OVERRIDE;

	image_id get_image() const OVERRIDE;
	image_id get_image(int nr) const OVERRIDE;
	image_id get_front_image() const OVERRIDE;
	void mark_images_dirty() const;

	image_id get_outline_image() const OVERRIDE;
	FLAGGED_PIXVAL get_outline_colour() const OVERRIDE;

	// caches image at height 0
	void calc_image() OVERRIDE;

	/**
	 * Called whenever the season or snowline height changes
	 * return false and the obj_t will be deleted
	 */
	bool check_season(const bool) OVERRIDE { calc_image(); return true; }

	/**
	 * @return Building's own name, or factory name (if building
	 * belongs to a factory)
	 */
	virtual const char *get_name() const OVERRIDE;
	const char* get_individual_name() const;

	void get_description(cbuffer_t & buf) const;

	bool is_townhall() const;

	bool is_headquarter() const;

	bool is_monument() const;

	bool is_attraction() const;

	bool is_city_building() const;

	bool is_signalbox() const;

	/// @copydoc obj_t::info
	void info(cbuffer_t & buf) const OVERRIDE;

	void rdwr(loadsave_t *file) OVERRIDE;

	void display_coverage_radius(bool display);

	/**
	 * Play animations of animated buildings.
	 * Count-down to replace construction site image by regular image.
	 */
	sync_result sync_step(uint32 delta_t) OVERRIDE;

	void set_tile( const building_tile_desc_t *t, bool start_with_construction );

	const building_tile_desc_t *get_tile() const { return tile; }

	virtual void show_info() OVERRIDE;

	void cleanup(player_t *player) OVERRIDE;

	/// @copydoc obj_t::finish_rd
	void finish_rd() OVERRIDE;

	// currently animated
	bool is_sync() const { return sync; }

	/**
	 * @returns pointer to first tile of a multi-tile building.
	 */
	const gebaeude_t* get_first_tile() const;

	gebaeude_t* access_first_tile();


	uint16 get_passengers_generated_visiting() const { return passengers_generated_visiting; }
	uint16 get_passengers_generated_commuting() const { return passengers_generated_commuting; }
	uint16 get_passengers_succeeded_visiting() const { return passengers_succeeded_visiting; }
	uint16 get_passengers_succeeded_commuting() const { return passengers_succeeded_commuting; }

	uint16 get_mail_generated() const { return mail_generated; }
	uint16 get_mail_delivery_succeeded_last_year() const { return mail_delivery_succeeded_last_year; }
	uint16 get_mail_delivery_succeeded() const { return mail_delivery_succeeded; }

	uint16 get_passenger_success_percent_this_year_commuting() const { return passengers_generated_commuting > 0 ? (passengers_succeeded_commuting * 100) / passengers_generated_commuting : 65535; }
	uint16 get_passenger_success_percent_last_year_commuting() const { return passenger_success_percent_last_year_commuting; }
	uint16 get_average_passenger_success_percent_commuting() const { return calc_two_years_average(get_passenger_success_percent_this_year_commuting(), passenger_success_percent_last_year_commuting);	}

	uint16 get_passenger_success_percent_this_year_visiting() const { return passengers_generated_visiting > 0 ? (passengers_succeeded_visiting * 100) / passengers_generated_visiting : 65535; }
	uint16 get_passenger_success_percent_last_year_visiting() const { return passenger_success_percent_last_year_visiting; }
	uint16 get_average_passenger_success_percent_visiting() const {	return calc_two_years_average(get_passenger_success_percent_this_year_visiting(), passenger_success_percent_last_year_visiting); }

	uint16 get_mail_delivery_success_percent_this_year() const { return mail_generated > 0 ? (mail_delivery_succeeded * 100) / mail_generated : 65535; }
	uint16 get_mail_delivery_success_percent_last_year() const { return mail_delivery_success_percent_last_year; }
	uint16 get_average_mail_delivery_success_percent() const { return calc_two_years_average(get_mail_delivery_success_percent_this_year(), get_mail_delivery_success_percent_last_year()); }

	void add_passengers_generated_visiting(uint16 number) { passengers_generated_visiting += number; }
	void add_passengers_succeeded_visiting(uint16 number) { passengers_succeeded_visiting += number; }

	void add_passengers_generated_commuting(uint16 number) { passengers_generated_commuting += number; }
	void add_passengers_succeeded_commuting(uint16 number) { passengers_succeeded_commuting += number; }

	void set_passengers_visiting_last_year(uint16 value) { passenger_success_percent_last_year_visiting = value; }
	void set_passengers_commuting_last_year(uint16 value) { passenger_success_percent_last_year_commuting = value; }

	void add_mail_generated(uint16 number) { mail_generated += number; }
	void add_mail_delivery_succeeded(uint16 number) { mail_delivery_succeeded += number; }
	void set_mail_delivery_succeeded_last_year(uint16 value) { mail_delivery_succeeded_last_year = value; }
	void set_mail_delivery_success_percent_last_year(uint16 value) { mail_delivery_success_percent_last_year = value; }

	void new_year();

	void check_road_tiles(bool del);

	uint16 get_weight() const;

	bool get_is_factory() const { return is_factory; }

	sint32 get_purchase_time() const { return purchase_time; }

	/**
	* Call this method when commuting passengers are sent to this building.
	* Pass the number of passengers being sent.
	* @author: jamespetts, August 2013
	*/
	void set_commute_trip(uint16 number);

	uint16 get_adjusted_population() const;
	uint16 get_adjusted_population_by_class(uint8 p_class) const;

	uint16 get_visitor_demand() const;
	uint16 get_adjusted_visitor_demand() const;
	uint16 get_adjusted_visitor_demand_by_class(uint8 p_class) const;
	inline void set_adjusted_visitor_demand(uint16 new_visitor_demand) { adjusted_people.visitor_demand = new_visitor_demand; }

	inline uint16 get_jobs() const { return jobs; }
	inline uint16 get_adjusted_jobs() const { return adjusted_jobs; }
	uint16 get_adjusted_jobs_by_class(uint8 p_class) const;
	inline void set_adjusted_jobs(uint16 new_jobs) { adjusted_jobs = new_jobs; }

	inline uint16 get_mail_demand() const { return mail_demand; }
	inline uint16 get_adjusted_mail_demand() const { return adjusted_mail_demand; }
	inline void set_adjusted_mail_demand(uint16 new_mail_demand) { adjusted_mail_demand = new_mail_demand; }

	bool jobs_available() const;

	/**
	 * @returns true if both building tiles are part of one (multi-tile) building.
	 */
	bool is_same_building(gebaeude_t* other) const;

	// @returns whether it is within the player's network. helper function for making the list
	bool is_within_players_network(const player_t* player, uint8 catg_index = goods_manager_t::INDEX_NONE) const;

	/**
	* Returns the number of jobs left in this building this month.
	* Note: this is measured in *adjusted* jobs.
	*/
	sint32 check_remaining_available_jobs() const;

	/*
	* Returns a percentage of the staffing level for this building
	*/
	sint32 get_staffing_level_percentage() const;

	uint8 get_random_class(const goods_desc_t* wtyp);

	bool get_is_in_world_list() const { return is_in_world_list > 0; }
	void set_in_world_list(bool value) { value ? is_in_world_list = 1 : is_in_world_list = 0; }

	sint64 get_available_jobs_by_time() const { return available_jobs_by_time; }
	void set_available_jobs_by_time(sint64 value) { available_jobs_by_time = value; }

	bool get_loaded_passenger_and_mail_figres() const { return loaded_passenger_and_mail_figres; }
	void set_loaded_passenger_and_mail_figres(bool value) { loaded_passenger_and_mail_figres = value; }

	const minivec_tpl<const planquadrat_t*> &get_tiles() { return building_tiles; }
	void set_building_tiles();

	void connect_by_road_to_nearest_city();

private:
	// Calculate last 2 years(13-24 months) average percentage
	inline uint16 calc_two_years_average(uint16 this_year, uint16 last_year) const {
		if (last_year == 65535) {
			return this_year;
		}
		else if (this_year == 65535) {
			return last_year;
		}
		else {
			uint8 month = welt->get_last_month() + 1;
			return (last_year * 12 + this_year * month) / (12 + month);
		}
	}

};


template<> inline gebaeude_t* obj_cast<gebaeude_t>(obj_t* const d)
{
	return dynamic_cast<gebaeude_t*>(d);
}

#endif
