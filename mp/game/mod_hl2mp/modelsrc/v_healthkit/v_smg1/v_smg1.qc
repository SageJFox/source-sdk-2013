$ModelName "weapons/v_healthkit.mdl"

$Body "studio" "v_smg1_Smg1_reference.smd"

$SurfaceProp "default"

$Contents "solid"

$CDMaterials "models\Weapons\V_hand"
$CDMaterials "models\Weapons\V_smg1"

$Attachment "muzzle" "ValveBiped.muzzle" 0 0 0 rotate 0 0 0
$Attachment "1" "ValveBiped.eject" 0 0 0 rotate 0 0 0

$Sequence "idle01" {
	"v_smg1_anims\idle01.smd"
	activity "ACT_VM_IDLE" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 30
	loop
}

$Sequence "fire01" {
	"v_smg1_anims\fire01.smd"
	activity "ACT_VM_PRIMARYATTACK" 1
	//{ event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
	//{ event 6001 0 "0" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$Sequence "fire02" {
	"v_smg1_anims\fire02.smd"
	activity "ACT_VM_RECOIL1" 1
	//{ event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
	//{ event 6001 0 "0" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$Sequence "fire03" {
	"v_smg1_anims\fire03.smd"
	activity "ACT_VM_RECOIL2" 1
	//{ event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
	//{ event 6001 0 "0" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$Sequence "fire04" {
	"v_smg1_anims\fire04.smd"
	activity "ACT_VM_RECOIL3" 1
	//{ event AE_MUZZLEFLASH 0 "SMG1 MUZZLE" }
	//{ event 6001 0 "0" }
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$Sequence "altfire" {
	"v_smg1_anims\altfire.smd"
	activity "ACT_VM_SECONDARYATTACK" 1
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$Sequence "draw" {
	"v_smg1_anims\draw.smd"
	activity "ACT_VM_DRAW" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	snap
	fps 30
}

$Sequence "reloadstart" {
	"v_smg1_anims\reload.smd"
	activity "ACT_SHOTGUN_RELOAD_START" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 60 //30
}

$Sequence "reload" {
	"v_smg1_anims\reload.smd"
	activity "ACT_VM_RELOAD" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 200 //30
}

$Sequence "reloadfast" {
	"v_smg1_anims\dryfire.smd"
	activity "ACT_SHOTGUN_PUMP" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 60 //30
}

$Sequence "reloadend" {
	"v_smg1_anims\reload.smd"
	activity "ACT_SHOTGUN_RELOAD_FINISH" 1
	fadein 0.2
	fadeout 0.2
	node "Ready"
	fps 60 //30
}

$Sequence "dryfire" {
	"v_smg1_anims\dryfire.smd"
	activity "ACT_VM_DRYFIRE" 1
	fadein 0.2
	fadeout 0.2
	node "Fire"
	snap
	fps 30
}

$Sequence "idletolow" {
	"v_smg1_anims\idletolow.smd"
	activity "ACT_VM_LOWERED_TO_IDLE" 1
	fadein 0.2
	fadeout 0.2
	transition "Ready" "Low"
	fps 30
}

$Sequence "lowtoidle" {
	"v_smg1_anims\lowtoidle.smd"
	activity "ACT_VM_IDLE_TO_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	transition "Low" "Ready"
	fps 30
}

$Sequence "lowidle" {
	"v_smg1_anims\lowidle.smd"
	activity "ACT_VM_IDLE_LOWERED" 1
	fadein 0.2
	fadeout 0.2
	node "Low"
	fps 30
	loop
}
