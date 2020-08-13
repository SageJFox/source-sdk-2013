$ModelName "weapons/v_dbshotgun.mdl"

$Body "studio" "v_shotgun_Shotgun_reference.smd"

$BodyGroup "shell"
{
	studio "shell.smd"
	blank
}

$SurfaceProp "default"

$Contents "solid"

$CDMaterials "models\Weapons\V_hand"
$CDMaterials "models\Weapons\V_shotgun"
$CDMaterials "models\weapons\shell"

$Attachment "muzzle" "ValveBiped.Gun" -0.03 0.51 23.45 rotate -90 0 0
$Attachment "1" "ValveBiped.Gun" -0.95 1.14 -5.59 rotate 0 180 -180

$Sequence "idle01" {
	"v_shotgun_anims\idle01.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
	loop
}

$Sequence "fire01" {
	"v_shotgun_anims\fire01.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$Sequence "altfire" {
	"v_shotgun_anims\altfire.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	{ event AE_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
	fadein 0.2
	fadeout 0.2
	node "2"
	snap
	fps 30
}

$Sequence "draw" {
	"v_shotgun_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	snap
	fps 30
}

$Sequence "holster" {
	"v_shotgun_anims\holster.smd"
	activity "ACT_VM_HOLSTER" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$Sequence "reload_start_both" {
	"v_shotgun_anims\reload1.smd"
	activity "ACT_SHOTGUN_RELOAD_START" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 15
}

$Sequence "reload_both" {
	"v_shotgun_anims\reload2.smd"
	activity "ACT_VM_RELOAD" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 10
}

$Sequence "reload_single" {
	"v_shotgun_anims\reload2.smd"
	activity "ACT_VM_RELOAD_SILENCED" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$Sequence "reload_end" {
	"v_shotgun_anims\reload3.smd"
	activity "ACT_SHOTGUN_RELOAD_FINISH" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$Sequence "reload_start_single" {
	"v_shotgun_anims\reload1.smd"
	activity "ACT_SHOTGUN_PUMP" 1
	fadein 0.2
	fadeout 0.2
	node "0"
	fps 30
}

$Sequence "dryfire" {
	"v_shotgun_anims\dryfire.smd"
	activity "ACT_VM_DRYFIRE" 1
	fadein 0.2
	fadeout 0.2
	node "2"
	fps 30
}

$Sequence "lowered" {
	"v_shotgun_anims\lowered.smd"
	activity "ACT_VM_IDLE_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	node "1"
	fps 30
	loop
}

$Sequence "lowered_to_idle" {
	"v_shotgun_anims\lowered_to_idle.smd"
	activity "ACT_VM_LOWERED_TO_IDLE" 1
	fadein 0.2
	fadeout 0.2
	transition "1" "0"
	fps 30
}

$Sequence "idle_to_lowered" {
	"v_shotgun_anims\idle_to_lowered.smd"
	activity "ACT_VM_IDLE_TO_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	transition "0" "1"
	fps 30
}

$Sequence "fire" {
	"v_shotgun_anims\fire.smd"
	activity "ACT_RANGE_ATTACK_SHOTGUN" 1
	{ event AE_MUZZLEFLASH 0 "SHOTGUN MUZZLE" }
	{ event 6001 0 "3" }
	{ event 3004 0 "" }
	fadein 0.2
	fadeout 0.2
	fps 5
}
