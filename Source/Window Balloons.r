#include			<Types.r>
#include			<BalloonTypes.r>

#include			"Constantes.h"

//RESOURCES:

resource 'hwin' (1000, "HELP Sampler") { //Sampler Window
	HelpMgrVersion,
	hmDefaultOptions,
	{
		1000,
		'hrct',
		7,
		"Sampler"
	}
};

resource 'hrct' (1000, "HELP Sampler Rects") {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{
		HMStringResItem {
			{10, 280},
			{1, 0, 18, 8 * 80},
			10000, 1
		},
		HMStringResItem {
			{0, 0},
			{0, 0, 0, 0},
			10000, 2
		},
		HMStringResItem {
			{144, 280},
			{82, 0, 208, 8 * 80},
			10000, 3
		},
		HMStringResItem {
			{220, 280},
			{213, 0, 229, 8 * 80},
			10000, 4
		},
		HMStringResItem {
			{239, 280},
			{231, 0, 247, 8 * 80},
			10000, 5
		},
		HMStringResItem {
			{259, 280},
			{251, 0, 268, 8 * 80},
			10000, 6
		},
		HMStringResItem {
			{27, 280},
			{20, 0, 34, 8 * 80},
			10000, 7
		},
		HMStringResItem {
			{59, 280},
			{41, 0, 78, 8 * 80},
			10000, 8
		}
	}
};

resource 'hwin' (2000, "HELP Recorder") { //Recorder Window
	HelpMgrVersion,
	hmDefaultOptions,
	{
		2000,
		'hrct',
		8,
		"Recorder"
	}
};

resource 'hrct' (2000, "HELP Recorder Rects") {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{
		HMStringResItem {
			{8, 118},
			{1, 1, 17, 157},
			11000, 1
		},
		HMStringResItem {
			{31, 26},
			{22, 1, 39, 39},
			11000, 2
		},
		HMStringResItem {
			{31, 70},
			{22, 40, 39, 78},
			11000, 3
		},
		HMStringResItem {
			{31, 108},
			{22, 79, 39, 117},
			11000, 4
		},
		HMStringResItem {
			{31, 146},
			{22, 118, 39, 157},
			11000, 5
		}
	}
};

resource 'hwin' (3000, "HELP Synthesizer") { //Recorder Window
	HelpMgrVersion,
	hmDefaultOptions,
	{
		3000,
		'hrct',
		11,
		"Synthesizer"
	}
};

resource 'hrct' (3000, "HELP Synthesizer Rects") {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{
		HMStringResItem {
			{70, 130},
			{21, 5, 145, 199},
			12000, 1
		},
		HMStringResItem {
			{70, 260},
			{33, 207, 145, 279},
			12000, 2
		},
		HMStringResItem {
			{18, 260},
			{4, 207, 31, 280},
			12000, 3
		}
	}
};

resource 'hwin' (4000, "HELP Rhythms Box") { //RB Window
	HelpMgrVersion,
	hmDefaultOptions,
	{
		4000,
		'hrct',
		7,
		"Rhythms Box"
	}
};

resource 'hrct' (4000, "HELP Rhythms Box Rects") {
	HelpMgrVersion,
	hmDefaultOptions,
	0,
	0,
	{
		HMStringResItem {
			{9, 40},
			{1, 1, 18, 78},
			13000, 1
		},
		HMStringResItem {
			{31, 40},
			{22, 1, 39, 78},
			13000, 2
		},
		HMStringResItem {
			{28, 130},
			{18, 104, 38, 155},
			13000, 3
		},
		HMStringResItem {
			{28, 210},
			{18, 186, 38, 237},
			13000, 4
		},
		HMStringResItem {
			{20, 280},
			{1, 263, 39, 317},
			13000, 5
		},
		HMStringResItem {
			{97, 31},
			{47, 1, 147, 64},
			13000, 6
		},
		HMStringResItem {
			{97, 71},
			{47, 66, 147, 76},
			13000, 7
		},
		HMStringResItem {
			{97, 150},
			{47, 78, 147, 238},
			13000, 8
		},
		HMStringResItem {
			{97, 280},
			{47, 248, 147, 314},
			13000, 9
		}
	}
};