$ModelName "weapons\w_healthkit.mdl"

$Body "Body" "healthKit_reference.smd"

$LOD 10
{
	replacemodel "healthKit_reference.smd" "healthKit_reference_lod1.smd"
}
$LOD 15
{
	replacemodel "healthKit_reference.smd" "healthKit_reference_lod2.smd"
}
$LOD 25
{
	replacemodel "healthKit_reference.smd" "healthKit_reference_lod3.smd"
}
$LOD 35
{
	replacemodel "healthKit_reference.smd" "healthKit_reference_lod4.smd"
}

$SurfaceProp "item"

$Contents "solid"

$CDMaterials "models\items"
$opaque

$illumposition 0 0 0 ValveBiped.Bip01_R_Hand

$Sequence "idle" {
	"healthkit_reference.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
	loop
}

$CollisionModel "healthkit_physics.smd"
{
	$mass 10
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
