/* Disco
 */

#include "../all.inc"

#declare P = -25

camera { location <0, 0, P> right x look_at 0 }

#include "../light.pov"

#include "colors.inc"

#declare Disco =
blob {
	threshold .65
	sphere { <0, 0, 0>, 7, 1 pigment { Yellow } }

	sphere { <3 * sin(radians(0)), -1, 3 * cos(radians(0))>, 1, -1
		pigment { Blue } }
	sphere { <3 * sin(radians(45)), -1, 3 * cos(radians(45))>, 1, -1
		pigment { Blue } }
	sphere { <3 * sin(radians(90)), -1, 3 * cos(radians(90))>, 1, -1
		pigment { Blue } }
	sphere { <3 * sin(radians(135)), -1, 3 * cos(radians(135))>, 1, -1
		pigment { Blue } }
	sphere { <3 * sin(radians(180)), -1, 3 * cos(radians(180))>, 1, -1
		pigment { Blue } }
	sphere { <3 * sin(radians(225)), -1, 3 * cos(radians(225))>, 1, -1
		pigment { Blue } }
	sphere { <3 * sin(radians(270)), -1, 3 * cos(radians(270))>, 1, -1
		pigment { Blue } }
	sphere { <3 * sin(radians(315)), -1, 3 * cos(radians(315))>, 1, -1
		pigment { Blue } }

	sphere { <0, -2, 0>, 2, -1
		pigment { Blue } }

	finish { phong 1 }
}

object {
	Disco
	rotate clock * 360 * y
}


//END