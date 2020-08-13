$ModelName "weapons/v_find.mdl"

$Body "arm" "v_arm.smd"

$BodyGroup "finds"
{
	studio "paper01.smd"
	studio "paper02.smd"
	studio "paper03.smd"
}

$SurfaceProp "default"

$Contents "solid"
$opaque
$illumposition 0 0 0 ValveBiped.Bip01

$CDMaterials "models\Weapons\V_hand"
$CDMaterials "models\props_repose"
$CDMaterials "models\props_c17"

$TextureGroup "skinfamilies"
{
	{ "paper_find01"		} //generic
	{ "paper_find02"		} //inconvenient blackmail
	{ "paper_find03"		} //damning blackmail
	{ "paper_find04"		} //juicy secret
	{ "paper_find05"		} //dangerous secret
	{ "paper_advantage01"	} //little white lie
	{ "paper_advantage02"	} //bold-faced lie
	{ "paper_advantage03"	} //sound advice
	{ "paper_advantage04"	} //threatening action
	{ "paper_advantage05"	} //read target
}

$Sequence "idle01" {
	"v_grenade_anims\idle01.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$Sequence "drawbackhigh" {
	"v_grenade_anims\drawbackhigh.smd"
	activity "ACT_VM_PULLBACK_HIGH" 1
	{ event 3900 4 "" }
	fadein 0.2
	fadeout 0.2
	fps 20
}

$Sequence "drawbacklow" {
	"v_grenade_anims\drawbacklow.smd"
	activity "ACT_VM_PULLBACK_LOW" 1
	{ event 3900 4 "" }
	fadein 0.2
	fadeout 0.2
	fps 20
}

$Sequence "throw" {
	"v_grenade_anims\throw.smd"
	activity "ACT_VM_THROW" 1
	{ event 3005 1 "" }
	fadein 0.2
	fadeout 0.2
	fps 20
}

$Sequence "roll" {
	"v_grenade_anims\roll.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	{ event 3013 2 "" }
	fadein 0.2
	fadeout 0.2
	fps 20
}

$Sequence "lob" {
	"v_grenade_anims\lob.smd"
	activity "ACT_VM_HAULBACK" 1
	{ event 3016 2 "" }
	fadein 0.2
	fadeout 0.2
	fps 20
}

$Sequence "draw" {
	"v_grenade_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	snap
	fps 30
}
