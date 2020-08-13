// Created by Crowbar 0.53

$ModelName "weapons/W_357.mdl"

$BodyGroup "studio"
{
	studio "w_357_HL2DM_357_reference.smd"
}


$SurfaceProp "weapon"

$Contents "solid"

$CDMaterials "models\weapons\w_357\"

$Attachment "muzzle" "ValveBiped.Muzzle" 0 0 0 rotate -90 -90 0

$CBox 0 0 0 0 0 0

$BBox -1.563 -1.255 -3.944 14.334 2.163 4.361


$Sequence "idle" {
	"w_357_anims\idle.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$Sequence "attack1" {
	"w_357_anims\attack1.smd"
	activity "ACT_RANGE_ATTACK_PISTOL" 1
	{ event 5003 0 "2" }
	{ event 3014 0 "0" }
	fadein 0.2
	fadeout 0.2
	fps 30
}

$CollisionModel "w_357_physics.smd"
{
	$mass 10
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "

}
