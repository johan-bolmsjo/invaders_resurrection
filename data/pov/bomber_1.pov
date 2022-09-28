/* Alien Bomber
 */

#include "../all.inc"

#declare P = -25

camera { location <0, 0, P> right x look_at 0 }

#include "../light.pov"

#include "colors.inc"

#declare Bomber =
blob {
	threshold .65
	sphere { <0, 0, 0>, 8, 1 pigment { Orange } }
	sphere { <2.6, -1, 2.6>, 1.8, 1 pigment { Green } }
	sphere { <2.6, -1, -2.6>, 1.8, 1 pigment { Green } }
	sphere { <-2.6, -1, 2.6>, 1.8, 1 pigment { Green } }
	sphere { <-2.6, -1, -2.6>, 1.8, 1 pigment { Green } }
	sphere { <0, -2, 0>, 3, -1 pigment { OrangeRed } }
	finish { phong 1 }
}

object {
	Bomber
	rotate clock * 360 * y
}


//END