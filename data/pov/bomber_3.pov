/*         Alien Disc
 * --+ More fun with blobs +--
 */

#include "../all.inc"

#declare P = -25

camera { location <0, 0, P> right x look_at 0 }

#include "../light.pov"

#include "colors.inc"

#declare Disc =
blob {
	threshold .65
	sphere { <0, 0, 0>, 8, 1 pigment { LightSteelBlue } }

	sphere { <4 * sin(radians(0)), -1.5, 4 * cos(radians(0))>,
		1.5, -2/5 pigment { CornflowerBlue } }
	sphere { <4 * sin(radians(45)), -1.5, 4 * cos(radians(45))>,
		1.5, -2/5 pigment { CornflowerBlue } }
	sphere { <4 * sin(radians(90)), -1.5, 4 * cos(radians(90))>,
		1.5, -2/5 pigment { CornflowerBlue } }
	sphere { <4 * sin(radians(135)), -1.5, 4 * cos(radians(135))>,
		1.5, -2/5 pigment { CornflowerBlue } }
	sphere { <4 * sin(radians(180)), -1.5, 4 * cos(radians(180))>,
		1.5, -2/5 pigment { CornflowerBlue } }
	sphere { <4 * sin(radians(225)), -1.5, 4 * cos(radians(225))>,
		1.5, -2/5 pigment { CornflowerBlue } }
	sphere { <4 * sin(radians(270)), -1.5, 4 * cos(radians(270))>,
		1.5, -2/5 pigment { CornflowerBlue } }
	sphere { <4 * sin(radians(315)), -1.5, 4 * cos(radians(315))>,
		1.5, -2/5 pigment { CornflowerBlue } }

	sphere { <0, -2, 0>, 3, -1 pigment { CornflowerBlue } }
	finish { phong 1 }
}

object {
	Disc
	scale <1, 1/2, 1>
	rotate clock * 360 * y
	//rotate 90 * x
}


//END