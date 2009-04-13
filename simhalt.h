/*
 * Copyright (c) 1997 - 2001 Hansj�rg Malthaner
 *
 * This file is part of the Simutrans project under the artistic licence.
 * (see licence.txt)
 */

#ifndef simhalt_h
#define simhalt_h

//#ifndef OLD_PATHING
//#define NEW_PATHING
//#endif

#include "convoihandle_t.h"
#include "linehandle_t.h"
#include "halthandle_t.h"

#include "simdings.h"
#include "simtypes.h"

#include "bauer/warenbauer.h"

#include "besch/ware_besch.h"

#include "dataobj/koord.h"

#include "tpl/slist_tpl.h"
#include "tpl/vector_tpl.h"
#include "tpl/quickstone_hashtable_tpl.h"
#include "tpl/fixed_list_tpl.h"
#ifdef NEW_PATHING
#include "tpl/binary_heap_tpl.h"
#endif

#define MAX_HALT_COST				7 // Total number of cost items
#define MAX_MONTHS					12 // Max history
#define MAX_HALT_NON_MONEY_TYPES	7 // number of non money types in HALT's financial statistic
#define HALT_ARRIVED				0 // the amount of ware that arrived here
#define HALT_DEPARTED				1 // the amount of ware that has departed from here
#define HALT_WAITING				2 // the amount of ware waiting
#define HALT_HAPPY					3 // number of happy passangers
#define HALT_UNHAPPY				4 // number of unhappy passangers
#define HALT_NOROUTE				5 // number of no-route passangers
#define HALT_CONVOIS_ARRIVED        6 // number of convois arrived this month

class cbuffer_t;
class grund_t;
class fabrik_t;
class karte_t;
class koord3d;
#ifdef LAGER_NOT_IN_USE
class lagerhaus_t;
#endif
class loadsave_t;
class schedule_t;
class spieler_t;
class ware_t;

// -------------------------- Haltestelle ----------------------------

/**
 * Haltestellen in Simutrans. Diese Klasse managed das Routing und Verladen
 * von Waren. Eine Haltestelle ist somit auch ein Warenumschlagplatz.
 *
 * @author Hj. Malthaner
 * @see stadt_t
 * @see fabrik_t
 * @see convoi_t
 */
class haltestelle_t
{
public:
	enum station_flags { NOT_ENABLED=0, PAX=1, POST=2, WARE=4, CROWDED=8 };
#ifndef NEW_PATHING
	enum routine_result_flags { NO_ROUTE=0, ROUTE_OK=1, ROUTE_OVERCROWDED=8 };
#endif
	//13-Jan-02     Markus Weber    Added
	enum stationtyp {invalid=0, loadingbay=1, railstation = 2, dock = 4, busstop = 8, airstop = 16, monorailstop = 32, tramstop = 64, maglevstop=128, narrowgaugestop=256 }; //could be combined with or!

private:
	/**
	 * Manche Methoden m�ssen auf alle Haltestellen angewandt werden
	 * deshalb verwaltet die Klasse eine Liste aller Haltestellen
	 * @author Hj. Malthaner
	 */
	static slist_tpl<halthandle_t> alle_haltestellen;

	/**
	 * finds a stop by its name
	 * @author prissi
	 */
	static stringhashtable_tpl<halthandle_t> all_names;

	/*
	 * struct holds new financial history for line
	 * @author hsiegeln
	 */
	sint64 financial_history[MAX_MONTHS][MAX_HALT_COST];

	/**
	 * initialize the financial history
	 * @author hsiegeln
	 */
	void init_financial_history();

	uint8 status_color;
	uint16 capacity[3]; // passenger, post, goods
	uint8 overcrowded[8];	// bit set, when overcrowded
	void recalc_status();

public:
	/**
	 * Tries to generate some pedestrians on the sqaure and the
	 * adjacent sqaures. Return actual number of generated
	 * pedestrians.
	 *
	 * @author Hj. Malthaner
	 */
	static int erzeuge_fussgaenger(karte_t *welt, koord3d pos, int anzahl);

	/* searches for a stop at the given koordinate
	 * this is called damend often, so we should think about inline it
	 * @return halthandle_t(), if nothing found
	 * @author prissi
	 */
	static halthandle_t get_halt(karte_t *welt, const koord pos);

	// Hajo: for future compatibility, migrate to this call
	// but since we allow only for a single stop per planquadrat, this is as good as the above
	static halthandle_t get_halt(karte_t *welt, const koord3d pos);

	static const slist_tpl<halthandle_t>& get_alle_haltestellen() { return alle_haltestellen; }

	/**
	 * Station factory method. Returns handles instead of pointers.
	 * @author Hj. Malthaner
	 */
	static halthandle_t create(karte_t *welt, koord pos, spieler_t *sp);

	/**
	 * Station factory method. Returns handles instead of pointers.
	 * @author Hj. Malthaner
	 */
	static halthandle_t create(karte_t *welt, loadsave_t *file);

	/*
	* removes a ground tile from a station, deletes the building and, if last tile, also the halthandle
	* @author prissi
	*/
	static bool remove(karte_t *welt, spieler_t *sp, koord3d pos, const char *&msg);

	/**
	 * Station destruction method.
	 * @author Hj. Malthaner
	 */
	static void destroy(halthandle_t &halt);

	/**
	 * destroys all stations
	 * @author Hj. Malthaner
	 */
	static void destroy_all(karte_t *);

private:
	/**
	 * Handle for ourselves. Can be used like the 'this' pointer
	 * @author Hj. Malthaner
	 */
	halthandle_t self;

public:
	/**
	 * Liste aller felder (Grund-Objekte) die zu dieser Haltestelle geh�ren
	 * @author Hj. Malthaner
	 */
	struct tile_t
	{
		tile_t(grund_t* grund_) : grund(grund_) {}

		bool operator ==(const tile_t& o) { return grund == o.grund; }
		bool operator !=(const tile_t& o) { return grund != o.grund; }

		grund_t*       grund;
		convoihandle_t reservation;
	};

#ifdef NEW_PATHING
	
	// Data on direct connexions from one station to the next.
	// @author: jamespetts
	struct connexion
	{
		// Times in minutes
		uint16 journey_time;
		uint16 waiting_time;
		
		// Convoy only used if line not used 
		// (i.e., if the best route involves using a convoy without a line)
		linehandle_t best_line;
		convoihandle_t best_convoy;

		// TODO: Consider whether to add comfort
	};

	struct path_node
	{
		halthandle_t halt;
		uint16 journey_time;
		path_node* link;

		// Necessary for sorting in a binary heap
		inline bool operator <= (const path_node p) const { return journey_time <= p.journey_time; }

	};

	// Data on paths to ultimate destinations
	// @author: jamespetts
	struct path
	{
		halthandle_t next_transfer;
		uint16 journey_time;
		path() { journey_time = 65535; }
		
		//TODO: Consider whether to add comfort
	};
#endif


private:
	slist_tpl<tile_t> tiles;

	koord init_pos;	// for halt without grounds, created during game initialisation

#ifdef NEW_PATHING
	// Table of all direct connexions to this halt, with routing information.
	// Array: one entry per goods type.
	quickstone_hashtable_tpl<haltestelle_t, connexion> connexions[16];

	quickstone_hashtable_tpl<haltestelle_t, path> paths[16];

	// The number of iterations of paths currently traversed. Used for
	// detecting when max_transfers has been reached.
	uint16 iterations;

	// Allocation of memory for nodes used during the pathing search.
	// @author: jamespetts
	path_node* path_nodes;

#else
	// List with all reachable destinations (old method)
	vector_tpl<halthandle_t>* warenziele;
#endif

	// loest warte_menge ab
	// "solves wait mixes off" (Babelfish); "solves warte volume from" (Google)
	vector_tpl<ware_t> **waren;

	/**
	 * Liste der angeschlossenen Fabriken
	 * @author Hj. Malthaner
	 */
	slist_tpl<fabrik_t *> fab_list;

	spieler_t *besitzer_p;
	static karte_t *welt;

	/**
	 * What is that for a station (for the image)
	 * @author prissi
	 */
	stationtyp station_type;

	uint8 rebuilt_destination_counter;	// new schedule, first rebuilt destinations asynchroniously
	uint8 reroute_counter;						// the reroute goods

	/* station flags (most what enabled) */
	uint8 enables;

	void set_pax_enabled(bool yesno)  { yesno ? enables |= PAX  : enables &= ~PAX;  }
	void set_post_enabled(bool yesno) { yesno ? enables |= POST : enables &= ~POST; }
	void set_ware_enabled(bool yesno) { yesno ? enables |= WARE : enables &= ~WARE; }

	/**
	 * Found route and station uncrowded
	 * @author Hj. Malthaner
	 */
	uint32 pax_happy;

	/**
	 * Found no route
	 * @author Hj. Malthaner
	 */
	uint32 pax_no_route;

	/**
	 * Station crowded
	 * @author Hj. Malthaner
	 */
	uint32 pax_unhappy;

	/**
	 * Haltestellen werden beim warenrouting markiert. Jeder durchgang
	 * hat eine eindeutige marke
	 *
	 * "Stops are at the routing were highlighted. Each passage has a unique brand" (Google)
	 * @author Hj. Malthaner
	 */
	uint32 marke;

//#define USE_QUOTE

#ifdef USE_QUOTE
	// for station rating
	//const char * quote_bezeichnung(int quote, convoihandle_t cnv) const;
	const char * quote_bezeichnung(int quote) const;
#endif

	/**
	 * versucht die ware mit beriets wartender ware zusammenzufassen
	 * @author Hj. Malthaner
	 */
	bool vereinige_waren(const ware_t &ware);

	// add the ware to the internal storage, called only internally
	void add_ware_to_halt(ware_t ware);

	/**
	 * liefert wartende ware an eine Fabrik
	 * @author Hj. Malthaner
	 */
	void liefere_an_fabrik(const ware_t& ware);

	/*
	* parameter to ease sorting
	* sortierung is local and stores the sortorder for the individual station
	* @author hsiegeln
	*/
	uint8 sortierung;
	bool resort_freight_info;

	haltestelle_t(karte_t *welt, loadsave_t *file);
	haltestelle_t(karte_t *welt, koord pos, spieler_t *sp);
	~haltestelle_t();

	// Record of waiting times. Takes a list of the last 16 waiting times per type of goods.
	// Getter method will need to average the waiting times. 
	// @author: jamespetts
	quickstone_hashtable_tpl<haltestelle_t, fixed_list_tpl<uint16, 16>> waiting_times[16];

#ifdef NEW_PATHING
	// Used for pathfinding. The list is stored on the heap so that it can be re-used
	// if searching is aborted part-way through.
	// @author: jamespetts
	binary_heap_tpl<path_node*> open_list;

	// Whether the search for the destination has completed: if so, the search will not
	// re-run unless the results are stale.
	bool search_complete;

	// When the connexions were last recalculated. Needed for checking whether they need to
	// be recalculated again. 
	// @author: jamespetts
	uint16 connexions_timestamp;

	// Likewise for paths
	// @author: jamespetts
	uint16 paths_timestamp;
#endif

public:
	/**
	* Called every 255 steps
	* will distribute the goods to changed routes (if there are any)
	* @author Hj. Malthaner
	*/
	void reroute_goods();

	/**
	 * getter/setter for sortby
	 * @author hsiegeln
	 */
	uint8 get_sortby() { return sortierung; }
	void set_sortby(uint8 sm) { resort_freight_info =true; sortierung = sm; }

	/**
	 * Calculates a status color for status bars
	 * @author Hj. Malthaner
	 */
	COLOR_VAL get_status_farbe() const { return status_color; }

	/**
	 * Draws some nice colored bars giving some status information
	 * @author Hj. Malthaner
	 */
	void display_status(sint16 xpos, sint16 ypos) const;

	/**
	 * sucht umliegende, erreichbare fabriken und baut daraus die
	 * Fabrikliste auf.
	 * @author Hj. Malthaner
	 */
	void verbinde_fabriken();
	void remove_fabriken(fabrik_t *fab);

#ifndef NEW_PATHING
	/**
	 * Rebuilds the list of reachable destinations
	 *
	 * @author Hj. Malthaner
	 */
	void rebuild_destinations();
#endif

	uint8 get_rebuild_destination_counter() const  { return rebuilt_destination_counter; }

#ifdef NEW_PATHING
	// New routing method: finds shortest route in *time*, not necessarily distance
	// @ author: jamespetts

	// Direct connexions from this station. Replaces rebuild_destinations()
	void rebuild_connexions(uint8 category);

	// Ultimate paths from this station. Packets searching for a path need only
	// grab a pre-calculated path from the hashtable generated by this method.
	void calculate_paths(halthandle_t goal, uint8 category);

#endif

	void rotate90( const sint16 y_size );

	spieler_t *get_besitzer() const {return besitzer_p;}

	// just for info so far
	sint64 calc_maintenance();

	bool make_public_and_join( spieler_t *sp );

#ifndef NEW_PATHING
	const vector_tpl<halthandle_t> *get_warenziele_passenger() const {return warenziele;}
	const vector_tpl<halthandle_t> *get_warenziele_mail() const {return warenziele+1;}

	// returns the matchin warenziele
	const vector_tpl<halthandle_t> *get_warenziele(uint8 catg_index) const {return warenziele+catg_index;}
#endif

	const slist_tpl<fabrik_t*>& get_fab_list() const { return fab_list; }

	/**
	 * Haltestellen messen regelmaessig die Fahrplaene pruefen
	 * @author Hj. Malthaner
	 */
	void step();

	/**
	 * Called every month/every 24 game hours
	 * @author Hj. Malthaner
	 */
	void neuer_monat();

	karte_t* get_welt() const { return welt; }

#ifndef NEW_PATHING
	/**
	 * Kann die Ware nicht zum Ziel geroutet werden (keine Route), dann werden
	 * Ziel und Zwischenziel auf koord::invalid gesetzt.
	 *
	 * @param ware die zu routende Ware
	 * @author Hj. Malthaner
	 *
	 * for reverse routing, also the next to last stop can be added, if next_to_ziel!=NULL
	 *
	 * if avoid_overcrowding is set, a valid route in only found when there is no overflowing stop in between
	 *
	 * @author prissi
	 */
	int suche_route( ware_t &ware, koord *next_to_ziel, bool avoid_overcrowding );
#else
	// @author: jamespetts, although much is borrowed from suche_route
	// Returns the journey time of the best possible route from this halt. Time == 65535 when there is no route.
	uint16 find_route(ware_t &ware, uint16 journey_time);
	uint16 find_route(ware_t &ware) { return find_route(ware, 65535); }
#endif

	int get_pax_enabled()  const { return enables & PAX;  }
	int get_post_enabled() const { return enables & POST; }
	int get_ware_enabled() const { return enables & WARE; }

	// check, if we accepts this good
	// often called, thus inline ...
	int is_enabled( const ware_besch_t *wtyp ) {
		if(wtyp==warenbauer_t::passagiere) {
			return enables&PAX;
		}
		else if(wtyp==warenbauer_t::post) {
			return enables&POST;
		}
		return enables&WARE;
	}

	/**
	 * Found route and station uncrowded
	 * @author Hj. Malthaner
	 */
	void add_pax_happy(int n);

	/**
	 * Found no route
	 * @author Hj. Malthaner
	 */
	void add_pax_no_route(int n);

	/**
	 * Station crowded
	 * @author Hj. Malthaner
	 */
	void add_pax_unhappy(int n);

	int get_pax_happy()    const { return pax_happy;    }
	int get_pax_no_route() const { return pax_no_route; }
	int get_pax_unhappy()  const { return pax_unhappy;  }


#ifdef LAGER_NOT_IN_USE
	void set_lager(lagerhaus_t* l) { lager = l; }
#endif

	bool add_grund(grund_t *gb);
	void rem_grund(grund_t *gb);

	uint32 get_capacity(uint8 typ) const { return capacity[typ]; }

	bool existiert_in_welt();

	koord get_init_pos() const { return init_pos; }
	koord get_basis_pos() const;
	koord3d get_basis_pos3d() const;

	/* return the closest square that belongs to this halt
	 * @author prissi
	 */
	koord get_next_pos( koord start ) const;

	// true, if this station is overcroded for this category
	bool is_overcrowded( const uint8 idx ) const { return (overcrowded[idx/8] & (1<<(idx%8)))!=0; }

	/**
	 * gibt Gesamtmenge derware vom typ typ zur�ck
	 * @author Hj. Malthaner
	 */
	uint32 get_ware_summe(const ware_besch_t *warentyp) const;

	/**
	 * returns total number for a certain position (since more than one factory might connect to a stop)
	 * @author Hj. Malthaner
	 */
	uint32 get_ware_fuer_zielpos(const ware_besch_t *warentyp, const koord zielpos) const;

	/**
	 * gibt Gesamtmenge derw are vom typ typ fuer zwischenziel zur�ck
	 * @author prissi
	 */
	uint32 get_ware_fuer_zwischenziel(const ware_besch_t *warentyp, const halthandle_t zwischenziel) const;

	/**
	 * @returns the sum of all waiting goods (100t coal + 10
	 * passengers + 2000 liter oil = 2110)
	 * @author Markus Weber
	 */
	uint32 sum_all_waiting_goods() const;

	// true, if we accept/deliver this kind of good
	bool gibt_ab(const ware_besch_t *warentyp) const { return waren[warentyp->get_catg_index()] != NULL; }

	/* retrieves a ware packet for any destination in the list
	 * needed, if the factory in question wants to remove something
	 */
	bool recall_ware( ware_t& w, uint32 menge );

	/**
	 * holt ware ab
	 * fetches ware from (Google)
	 *
	 * @return abgeholte menge
	 * @return collected volume (Google)
	 * @author Hj. Malthaner
	 */
	ware_t hole_ab(const ware_besch_t *warentyp, uint32 menge, schedule_t *fpl, convoi_t* cnv);

	/* liefert ware an. Falls die Ware zu wartender Ware dazugenommen
	 * werden kann, kann ware_t gel�scht werden! D.h. man darf ware nach
	 * aufruf dieser Methode nicht mehr referenzieren!
	 *
	 * Ware to deliver. If the goods to waiting to be taken product 
	 * can be ware_t may be deleted! I.e. we must, after calling this 
	 * method no longer refer! (Google)
	 *
	 * The second version is like the first, but will not recalculate the route
	 * This is used for inital passenger, since they already know a route
	 *
	 * @return angenommene menge
	 * @author Hj. Malthaner/prissi
	 */
	uint32 liefere_an(ware_t ware);
	uint32 starte_mit_route(ware_t ware);

#ifndef NEW_PATHING
	/**
	 * wird von Fahrzeug aufgerufen, wenn dieses an der Haltestelle
	 * gehalten hat.
	 * @param typ der bef�rderte warentyp
	 *
	 * "of vehicle is called when this at the bus stop has.
	 * @ param type of product transported" (Google)
	 * 
	 * @author Hj. Malthaner
	 */
	void hat_gehalten(const ware_besch_t *warentyp, const schedule_t *fpl);
#else

	// Adding method for the new routing system. Equivalent to
	// hat_gehalten with the old system. 
	void add_connexion(const ware_besch_t *type, const schedule_t *fpl, const convoihandle_t cnv, const linehandle_t line);
#endif

	const grund_t *find_matching_position(waytype_t wt) const;

	/* checks, if there is an unoccupied loading bay for this kind of thing
	* @author prissi
	*/
	bool find_free_position(const waytype_t w ,convoihandle_t cnv,const ding_t::typ d) const;

	/* reserves a position (caution: railblocks work differently!
	* @author prissi
	*/
	bool reserve_position(grund_t *gr,convoihandle_t cnv);

	/* frees a reserved  position (caution: railblocks work differently!
	* @author prissi
	*/
	bool unreserve_position(grund_t *gr, convoihandle_t cnv);

	/* true, if this can be reserved
	* @author prissi
	*/
	bool is_reservable(const grund_t *gr, convoihandle_t cnv) const;

	void info(cbuffer_t & buf) const;

	/**
	 * @param buf the buffer to fill
	 * @return Goods description text (buf)
	 * @author Hj. Malthaner
	 */
	void get_freight_info(cbuffer_t & buf);

	/**
	 * @param buf the buffer to fill
	 * @return short list of the waiting goods (i.e. 110 Wood, 15 Coal)
	 * @author Hj. Malthaner
	 */
	void get_short_freight_info(cbuffer_t & buf);

	/**
	 * Opens an information window for this station.
	 * @author Hj. Malthaner
	 */
	void zeige_info();

	/**
	 * @returns the type of a station
	 * (combination of: railstation, loading bay, dock)
	 * @author Markus Weber
	 */
	stationtyp get_station_type() const { return station_type; }
	void recalc_station_type();

	/**
	 * fragt den namen der Haltestelle ab.
	 * Der Name ist der text des ersten Untergrundes der Haltestelle
	 * @return der Name der Haltestelle.
	 * @author Hj. Malthaner
	 */
	const char *get_name() const;

	void set_name(const char *name);

	// create an unique name: better to be called with valid handle, althoug it will work without
	char *create_name(const koord k, const char *typ);

	void rdwr(loadsave_t *file);

	void laden_abschliessen();

	/*
	 * called, if a line serves this stop
	 * @author hsiegeln
	 */
	void add_line(linehandle_t line) { registered_lines.append_unique(line); }

	/*
	 * called, if a line removes this stop from it's schedule
	 * @author hsiegeln
	 */
	void remove_line(linehandle_t line) { registered_lines.remove(line); }

	/*
	 * list of line ids that serve this stop
	 * @author hsiegeln
	 */
	vector_tpl<linehandle_t> registered_lines;

	/**
	 * book a certain amount into the halt's financial history
	 * @author hsiegeln
	 */
	void book(sint64 amount, int cost_type);

	/**
	 * return a pointer to the financial history
	 * @author hsiegeln
	 */
	sint64* get_finance_history() { return *financial_history; }

	/**
	 * return a specified element from the financial history
	 * @author hsiegeln
	 */
	sint64 get_finance_history(int month, int cost_type) const { return financial_history[month][cost_type]; }

	// flags station for a crowded message at the beginning of next month
	void bescheid_station_voll() { enables |= CROWDED; status_color = COL_RED; }

	/* marks a coverage area
	* @author prissi
	*/
	void mark_unmark_coverage(const bool mark) const;

	// @author: jamespetts
	// Returns the proportion of unhappy people of the total of
	// happy and unhappy people.
	float get_unhappy_proportion(uint8 month) const { return financial_history[month][HALT_HAPPY] > 0 ? (float)financial_history[month][HALT_UNHAPPY] / (float)(financial_history[month][HALT_HAPPY] + financial_history[month][HALT_UNHAPPY]) : 0; }
 
	// Getting and setting average waiting times in minutes
	// @author: jamespetts
	uint16 get_average_waiting_time(halthandle_t halt, uint8 category) const;

	void add_waiting_time(uint16 time, halthandle_t halt, uint8 category)
	{
		if(halt.is_bound())
		{
			/*fixed_list_tpl<uint16, 16> tmp = *(new fixed_list_tpl<uint16, 16>);
			if(!waiting_times[category].put(halt, tmp))
			{
				delete &tmp;
			}*/
			if(waiting_times[category].access(halt) == NULL)
			{
				fixed_list_tpl<uint16, 16> *tmp = new fixed_list_tpl<uint16, 16>;
				waiting_times[category].put(halt, *tmp);
			}
			waiting_times[category].access(halt)->add_to_tail(time);
		}
	}

	inline uint16 get_waiting_minutes(uint32 waiting_ticks) const;

#ifdef NEW_PATHING
	quickstone_hashtable_tpl<haltestelle_t, connexion>* get_connexions(uint8 c); 

	// Finds the best path from here to the goal halt.
	// Looks up the paths in the hashtable - if the table
	// is not stale, and the path is in it, use that, or else
	// search for a new path.
	path get_path_to(halthandle_t goal, uint8 category);

	linehandle_t get_preferred_line(halthandle_t transfer, uint8 category) const;
	convoihandle_t get_preferred_convoy(halthandle_t transfer, uint8 category) const;

	// Set to true if a schedule that serves this stop has changed since
	// the connexions were last recalculated. Used for spreading the load
	// of the recalculating connexions algorithm with the pathfinding 
	// algorithm.
	// @author: jamespetts
	bool reschedule;

	// Makes the paths recalculate, even if it would not otherwise be time for
	// them to do so. Does this by making sure that the timestamp is lower than
	// the counter.
	// @author: jamespetts
	void force_paths_stale();
#endif

};
#endif
