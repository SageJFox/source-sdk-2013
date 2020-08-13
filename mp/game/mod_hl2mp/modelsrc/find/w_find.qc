$modelname "weapons/w_find.mdl"

$BodyGroup "finds"
{
	studio "paper01.smd"
	studio "paper02.smd"
	studio "paper03.smd"
}

$surfaceprop "paper"

$contents "solid"

$cdmaterials "models\props_repose"
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

$illumposition 0 0 0 ValveBiped.Bip01

$sequence "idle" {
	"anims/w_idle.smd"
	fadein 0.2
	fadeout 0.2
	loop
	fps 30
}

$collisionmodel "paper01_physics.smd"
{
	$mass 0.008
	$inertia 1
	$damping 0
	$rotdamping 0
	$rootbone " "

}

$keyvalues
{
	prop_data 
	{
		"base"		"Cardboard.Small"
	}
}
