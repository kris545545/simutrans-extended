/*
 * This file is part of the Simutrans-Extended project under the Artistic License.
 * (see LICENSE.txt)
 */

#ifndef BODEN_MONORAILBODEN_H
#define BODEN_MONORAILBODEN_H


#include "grund.h"

class monorailboden_t : public grund_t
{
protected:
	void calc_image_internal(const bool calc_only_snowline_change) OVERRIDE;

public:
	monorailboden_t(loadsave_t *file, koord pos ) : grund_t( koord3d(pos,0) ) { rdwr(file); }
	monorailboden_t(koord3d pos,slope_t::type slope);

	void rdwr(loadsave_t *file) OVERRIDE;

	const char *get_name() const OVERRIDE {return "Monorailboden";}
	typ get_typ() const OVERRIDE { return monorailboden; }
};

#endif
