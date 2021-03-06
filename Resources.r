#include			<Types.r>

//RESOURCES:

resource 'kind' (128, "") { //Hit Factory Custom kind strings
	'HiFc',
	0,
	{
		'MSic',
		"Hit Factory music",
		
		'CMus',
		"Hit Factory compiled music",
	
		'Bank',
		"Hit Factory bank",
		
		'Patt',
		"Hit Factory pattern set",
	
		'Data',
		"Hit Factory data",
		
		'IpPI',
		"Hit Factory import plug-in",
		
		'EdPI',
		"Hit Factory editor plug-in",
		
		'ItPI',
		"Hit Factory interface plug-in",
		
		'DDPI',
		"Hit Factory Direct To Disk plug-in"
	}
};

resource 'open' (128, "") { //FileTypes Hit Factory can open
	'HiFc',
	{
		'MSic',
		'CMus',
		'Bank',
		'Patt',
		'sfil',
		'AIFF'
	}
};