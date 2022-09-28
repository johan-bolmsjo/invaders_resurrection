/* Missile
 */

#include "../all.inc"

#declare P = -25

camera { location <0, 0, P> right x look_at 0 }

#include "../light.pov"


#declare Fin =
difference {
	box {
		<-.5, -.5, -.5>, <.5, .5, .5>
		scale <1, 2.5, 0.25>
	}
	box {
		<-.5, -.5, -.5>, <.5, .5, .5>
		scale 4
		rotate z * 40
		translate <1, -2.2, 0>
	}
	texture {
		pigment {
			rgb <.4, 0, 0>
		}
		finish {
			ambient .4
			diffuse .4
		}
	}
}


#declare Body =
union {
	cylinder {
		<0, 2.5, 0>, <0, -1.5, 0>, 1
		texture { T_Gold_3C }
	}
	cone {
		<0, -1.5, 0>, 1, <0, -2.5, 0>, 0.5
		texture { T_Silver_3C }
	}
}


#declare Missile =
union {
	object { Body }
	object { Fin translate <1.25, 0.75, 0> rotate y * 0}
	object { Fin translate <1.25, 0.75, 0> rotate y * 90}
	object { Fin translate <1.25, 0.75, 0> rotate y * 180}
	object { Fin translate <1.25, 0.75, 0> rotate y * 270}
}


object {
	Missile
	rotate clock * 360 * y
}


//END