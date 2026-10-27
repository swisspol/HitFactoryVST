#include			<Types.r>
#include			<BalloonTypes.r>

//RESOURCES:

resource 'hdlg' (3000, "HELP General Pref") { //General Preferences Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //Accept
			{0, 0},
			{0,0,0,0},
			3000, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Cancel
			{0, 0},
			{0,0,0,0},
			3000, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Max volume
			{0, 0},
			{0,0,0,0},
			3000, 3,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Select HF
			{0, 0},
			{0,0,0,0},
			3000, 4,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Tracks
			{0, 0},
			{0,0,0,0},
			3000, 5,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Turbo
			{0, 0},
			{0,0,0,0},
			3000, 6,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Loop DTD
			{0, 0},
			{0,0,0,0},
			3000, 7,
			0, 0,
			3000, 8,
			0, 0
		},
		HMStringResItem { //Polyphony
			{0, 0},
			{0,0,0,0},
			3000, 9,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Turbo
			{0, 0},
			{0,0,0,0},
			3000, 10,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //RB Poly
			{0, 0},
			{0,0,0,0},
			3000, 11,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //RB Turbo
			{0, 0},
			{0,0,0,0},
			3000, 12,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //ByPass headers
			{0, 0},
			{0,0,0,0},
			3000, 13,
			0, 0,
			3000, 14,
			0, 0
		},
		HMStringResItem { //Interrupt
			{0, 0},
			{0,0,0,0},
			3000, 15,
			0, 0,
			3000, 16,
			0, 0
		},
		HMStringResItem { //DTD Buffer
			{0, 0},
			{0,0,0,0},
			3000, 17,
			0, 0,
			0, 0,
			0, 0
		}
	}
};

resource 'hdlg' (3500, "HELP MIDI Pref") { //MIDI Preferences Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //Accept
			{0, 0},
			{0,0,0,0},
			3500, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Cancel
			{0, 0},
			{0,0,0,0},
			3500, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //On
			{0, 0},
			{0,0,0,0},
			3500, 3,
			0, 0,
			3500, 4,
			0, 0
		},
		HMStringResItem { //AMM
			{0, 0},
			{0,0,0,0},
			3500, 5,
			0, 0,
			3500, 5,
			0, 0
		},
		HMStringResItem { //OMS
			{0, 0},
			{0,0,0,0},
			3500, 6,
			0, 0,
			3500, 6,
			0, 0
		},
		HMStringResItem { //FG only
			{0, 0},
			{0,0,0,0},
			3500, 7,
			0, 0,
			3500, 8,
			0, 0
		},
		HMStringResItem { //Auto
			{0, 0},
			{0,0,0,0},
			3500, 9,
			0, 0,
			3500, 10,
			0, 0
		},
		HMStringResItem { //extern clock
			{0, 0},
			{0,0,0,0},
			3500, 11,
			0, 0,
			3500, 12,
			0, 0
		},
		HMStringResItem { //AllToSampler
			{0, 0},
			{0,0,0,0},
			3500, 13,
			0, 0,
			3500, 13,
			0, 0
		},
		HMStringResItem { //AllToSamplerChan
			{0, 0},
			{0,0,0,0},
			3500, 14,
			0, 0,
			3500, 14,
			0, 0
		},
		HMStringResItem { //AllToSynthetizer
			{0, 0},
			{0,0,0,0},
			3500, 15,
			0, 0,
			3500, 15,
			0, 0
		},
		HMStringResItem { //AllToSynthetizerChan
			{0, 0},
			{0,0,0,0},
			3500, 16,
			0, 0,
			3500, 16,
			0, 0
		},
		HMStringResItem { //AllToRBox
			{0, 0},
			{0,0,0,0},
			3500, 17,
			0, 0,
			3500, 17,
			0, 0
		},
		HMStringResItem { //Both
			{0, 0},
			{0,0,0,0},
			3500, 18,
			0, 0,
			3500, 18,
			0, 0
		},
		HMStringResItem { //SamplerChan
			{0, 0},
			{0,0,0,0},
			3500, 19,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //SynthetizerChan
			{0, 0},
			{0,0,0,0},
			3500, 20,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //RBChan
			{0, 0},
			{0,0,0,0},
			3500, 21,
			0, 0,
			0, 0,
			0, 0
		}
	}
};

resource 'hdlg' (3600, "HELP Reverb Pref") { //Reverb Preferences Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //Accept
			{0, 0},
			{0,0,0,0},
			3600, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Cancel
			{0, 0},
			{0,0,0,0},
			3600, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Enable reverb
			{0, 0},
			{0,0,0,0},
			3600, 3,
			0, 0,
			3600, 4,
			0, 0
		},
		HMStringResItem { //Distance to walls
			{0, 0},
			{0,0,0,0},
			3600, 5,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Walls attenuation
			{0, 0},
			{0,0,0,0},
			3600, 6,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Reverb
			{0, 0},
			{0,0,0,0},
			3600, 7,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Reference distance
			{0, 0},
			{0,0,0,0},
			3600, 8,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Current distance
			{0, 0},
			{0,0,0,0},
			3600, 9,
			0, 0,
			0, 0,
			0, 0
		}
	}
};

resource 'hdlg' (3700, "HELP Color Pref") { //Color Preferences Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //Accept
			{0, 0},
			{0,0,0,0},
			3700, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Cancel
			{0, 0},
			{0,0,0,0},
			3700, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //sample color
			{0, 0},
			{0,0,0,0},
			3700, 3,
			0, 0,
			0, 0,
			0, 0
		},
		HMSkipItem {
		},
		HMStringResItem { //DTD color
			{0, 0},
			{0,0,0,0},
			3700, 4,
			0, 0,
			0, 0,
			0, 0
		},
		HMSkipItem {
		},
		HMStringResItem { //display color
			{0, 0},
			{0,0,0,0},
			3700, 5,
			0, 0,
			0, 0,
			0, 0
		}
	}
};

resource 'hdlg' (3800, "HELP Quantize Pref") { //Quantize Preferences Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //Accept
			{0, 0},
			{0,0,0,0},
			3800, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Cancel
			{0, 0},
			{0,0,0,0},
			3800, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Quantize samples starts
			{0, 0},
			{0,0,0,0},
			3800, 3,
			0, 0,
			3800, 4,
			0, 0
		},
		HMStringResItem { //Quantize samples stops
			{0, 0},
			{0,0,0,0},
			3800, 5,
			0, 0,
			3800, 6,
			0, 0
		},
		HMStringResItem { //Auto align
			{0, 0},
			{0,0,0,0},
			3800, 7,
			0, 0,
			3800, 8,
			0, 0
		},
		HMStringResItem { //Qunatize RB mute
			{0, 0},
			{0,0,0,0},
			3800, 9,
			0, 0,
			3800, 10,
			0, 0
		}
	}
};

resource 'hdlg' (7000, "HELP Edit Part") { //Edit Part Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //Done
			{0, 0},
			{0,0,0,0},
			7000, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Load sound
			{0, 0},
			{0,0,0,0},
			7000, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Try sound
			{0, 0},
			{0,0,0,0},
			7000, 3,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Delete sound
			{0, 0},
			{0,0,0,0},
			7000, 4,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Name
			{0, 0},
			{0,0,0,0},
			7000, 5,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Part volume
			{0, 0},
			{0,0,0,0},
			7000, 6,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Part panoramic
			{0, 0},
			{0,0,0,0},
			7000, 7,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Part pitch
			{0, 0},
			{0,0,0,0},
			7000, 8,
			0, 0,
			0, 0,
			0, 0
		}
	}
};

resource 'hdlg' (8000, "HELP Piano Pref") { //Piano Preferences Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //Accept
			{0, 0},
			{0,0,0,0},
			8000, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Cancel
			{0, 0},
			{0,0,0,0},
			8000, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Reset
			{0, 0},
			{0,0,0,0},
			8000, 3,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Clear all
			{0, 0},
			{0,0,0,0},
			8000, 4,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Menu
			{0, 0},
			{0,0,0,0},
			0, 0,
			0, 0,
			8000, 5,
			0, 0
		},
		HMStringResItem { //Key up
			{0, 0},
			{0,0,0,0},
			8000, 6,
			0, 0,
			8000, 6,
			0, 0
		},
		HMStringResItem { //List
			{140, 80},
			{0,0,0,0},
			8000, 7,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //KeyUp
			{0, 0},
			{0,0,0,0},
			8000, 5,
			0, 0,
			0, 0,
			0, 0
		}
	}
};

resource 'hdlg' (9000, "HELP Export Compile") { //Export compile Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //OK
			{0, 0},
			{0,0,0,0},
			9000, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Cancel
			{0, 0},
			{0,0,0,0},
			9000, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //size
			{0, 0},
			{0,0,0,0},
			9000, 3,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //rate
			{0, 0},
			{0,0,0,0},
			9000, 4,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //channels
			{0, 0},
			{0,0,0,0},
			9000, 5,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //player
			{0, 0},
			{0,0,0,0},
			9000, 6,
			0, 0,
			9000, 7,
			0, 0
		},
		HMStringResItem { //smpl poly
			{0, 0},
			{0,0,0,0},
			9000, 8,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //syth poly
			{0, 0},
			{0,0,0,0},
			9000, 9,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //ryt poly
			{0, 0},
			{0,0,0,0},
			9000, 10,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //volume
			{0, 0},
			{0,0,0,0},
			9000, 11,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //autoplay
			{0, 0},
			{0,0,0,0},
			9000, 12,
			0, 0,
			9000, 13,
			0, 0
		}
	}
};

resource 'hdlg' (9100, "HELP Export AIFF") { //Export AIFF Dialog
	HelpMgrVersion,
	0,
	hmDefaultOptions,
	0,
	6,
	HMSkipItem {
	},
	{
		HMStringResItem { //OK
			{0, 0},
			{0,0,0,0},
			9100, 1,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //Cancel
			{0, 0},
			{0,0,0,0},
			9100, 2,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //type
			{0, 0},
			{0,0,0,0},
			9100, 3,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //creator
			{0, 0},
			{0,0,0,0},
			9100, 4,
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem { //feedback
			{0, 0},
			{0,0,0,0},
			9100, 5,
			0, 0,
			9100, 6,
			0, 0
		},
		HMStringResItem { //buffer
			{0, 0},
			{0,0,0,0},
			9100, 7,
			0, 0,
			0, 0,
			0, 0
		}
	}
};