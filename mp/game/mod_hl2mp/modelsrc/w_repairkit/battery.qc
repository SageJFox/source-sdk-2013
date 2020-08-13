$ModelName "weapons/w_repairkit.mdl"

$Body "Body" "battery_reference.smd"

$LOD 10
{
	replacemodel "battery_reference.smd" "battery_reference_lod1.smd"
}
$LOD 15
{
	replacemodel "battery_reference.smd" "battery_reference_lod2.smd"
}
$LOD 20
{
	replacemodel "battery_reference.smd" "battery_reference_lod3.smd"
}
$LOD 25
{
	replacemodel "battery_reference.smd" "battery_reference_lod4.smd"
}

$SurfaceProp "item"

$Contents "solid"
$opaque

$CDMaterials "models\items"

$illumposition 0 0 0 ValveBiped.Bip01_R_Hand


$Sequence "idle" {
	"battery_reference.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$CollisionModel "battery_physics.smd"
{
	$mass 5
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "
	$concave

}

$KeyValues
{
	prop_data 
	{
		"base"		"Item.Small"
	}
}
