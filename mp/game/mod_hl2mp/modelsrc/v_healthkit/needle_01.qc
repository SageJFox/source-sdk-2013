$ModelName "props_junk/garbage_syringeneedle001a.mdl"

$StaticProp

$Body "needle" "needle_01_needle.smd"

$LOD 11
{
	replacemodel "needle_01_needle.smd" "needle_01_needle_lod1.smd"
}
$LOD 22
{
	replacemodel "needle_01_needle.smd" "needle_01_needle_lod2.smd"
}

$SurfaceProp "plastic"

$Contents "solid"
$opaque

$illumposition 0 0 0

$CDMaterials "models\props_junk"


$Sequence "idle" {
	"needle_01_anims\idle.smd"
	fadein 0.2
	fadeout 0.2
	fps 30
}

$CollisionModel "needle_01_physics.smd"
{
	$mass 0.18458
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
		"base" "Item.Small"
	}
}
