/* UFO
 */

#include "../all.inc"

#declare P = -40

camera { location <0, -0.001, P> right x look_at 0 }

#include "../light.pov"

#declare Body =
difference {
	lathe {
		cubic_spline
		9,
		<0, 2.5>,
		<0, 2.5>,
		<0, 2.5>,
		<5, 2.2>,
		<10, 0>,
		<5, -2.2>,
		<0, -2.5>,
		<0, -2.5>,
		<0, -2.5>
	}
	sphere { <0,0,0>, 5 scale <1, .5, 1> translate y * 3.8 }
	pigment { rgb <.2, .3, .4> }
	finish { F_MetalC }
}


#declare Hod =
intersection {
	sphere { <0,0,0>, 5 }
	box { <-5, 0, -5>, <5, 5, 5> }
	pigment { rgbf <.98, .98, 1.0, .3> }
	finish { F_MetalE }
}



#declare UFO =
union {
	object { Body }
	object { Hod translate 1 * y }
}


object {
	UFO
	rotate clock * 360 * x
}


//END