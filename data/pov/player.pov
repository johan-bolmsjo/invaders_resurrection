/* Player
 */

#include "../all.inc"

#declare P = -40

camera { location <0, 0, P> right x look_at 0 }

#include "../light.pov"

#include "colors.inc"


#declare Body =
lathe {
	cubic_spline
	9,
	<0, 3>,
	<0, 3>,

	<0, 10>,
	<5, 3>,
	<5, -5>,
	<4, -9>,
	<0, -10>,
	
	<0, -8>,
	<0, -8>
	pigment { Orange }
	finish { phong .2 }
}


#declare Wing =
prism {
	cubic_spline
	.25, -.25, 7,
	<-6, -10>,
	<0, 0>, <0, -12>, <-6, -10>, <-5, -5>, <0, 0>,
	<-6, -10>
	rotate -90 * x
	texture { T_Chrome_2B }
}


#declare Bridge =
difference {
	box { <-3, -8, -.1>, <3, 2, .1> }
	cylinder { <0, 2, -1>, <0, 2, 1>, 1 }
	cylinder { <0, -8, -1>, <0, -8, 1>, 1 }
	texture { T_Chrome_2B }
}

#declare Rocket =
merge {
	object {
		Body
		scale <.4, .8, .4>
		translate <-3, -1.5, 0>
	}
	object {
		Body
		scale <.4, .8, .4>
		translate <3, -1.5, 0>
	}
	object {
		Wing
		translate <-3, 4, 0>
	}
	object {
		Wing
		translate <-3, 4, 0>
		rotate 180 * y
	}
	object {
		Bridge
	}
}


object {
	Rocket
	//rotate <60, -30, 0>
	rotate clock * 360 * y
}


//END