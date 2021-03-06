#pragma	SC once

//CONSTANTES:

//GENERAL:
#define			nbNotes				3800

#define			defaultMaxVolume	256
#define			loopValue			10
#define			timeFactor			1.6
#define			kCycleSize			100

#define			GWBitMapPtr(w)		&(((GrafPtr)(w))->portBits)
#define			ActualTime			(TickCount() - startTime) * timeFactor

#define			kCreateFileIDs		true
#define			kA5savy				true

#define			kEnterDelay			10L
#define			kStartLoadMenusID	200
#define			kNbLoadMenus		10

//IMPORT PLUG-INS:
#define			kMaxImportPlugsIns	15
#define			kTypesPerImport		3
#define			kLoopSound			1
#define			kNewHeader			2

//DIRECT TO DISK PLUG-INS:
#define			kMaxDTDPlugsIns		15
#define			kTypesPerDTD		3
#define			kSupportInterruptTime	1

//EDITOR PLUG-INS:
#define			kMaxEditorPlugsIns	5

//SAMPLER:
#define			maxNbSamples		30
#define			minNbShowSamples	8
#define			maxNbShowSamples	16
#define			samplerWide			80 * nbShowSamples

#define			kMinRelativeRate	-28
#define			kMaxRelativeRate	28

#define			sampleType			1
#define			directToDiskType	2
#define			DTDCode				-1

#define			volPaddleStart		102
#define			volPaddleLength		84
#define			volPaddleEnd		186
#define			insVolPaddleStart	58
#define			insVolPaddleLength	64
#define			insVolPaddleEnd		122
#define			maxWidthSmpl		60

//SYNTHETIZER:
#define			maxNbInstruments	20
#define			maxNbInsChannels	10

#define			pianoSize			120
#define			pianoAdd			2
#define			baseKey				60

#define			maxWidthInst		105

//RYTHMS BOX:
#define			kPatternLength		16
#define			kPatternParts		10
#define			kNbPatterns			20
#define			kMaxRythmsChan		10
#define			kMaxWidthRyth		62
#define			kMaxWidthSet		60

#define			kRytVolPaddleStart	62
#define			kRytVolPaddleLength	64
#define			kRytVolPaddleEnd	126

#define			kDefaultPartVolume	100

//KEYS CODES:
#define			kLeftArrowKey		31488
#define			kRightArrowKey		31744
#define			kUpArrowKey			32256
#define			kDownArrowKey		32000

#define			kTabKey				12288
#define			kDeleteKey			13056
#define			kReturnKey			9216
#define			kEnterKey			19456

#define			kMIDIAllToSmpl		0
#define			kMIDIAllToSmplChan	1
#define			kMIDIAllToSynth		5
#define			kMIDIAllToSynthChan	6
#define			kMIDISmplAndSynth	10
#define			kMIDIAllToRBox		20

//CONSTANTES FLAGS:
#define			kByPassHeaders		1
#define			kNoInterruptBank	2
#define			kExtendedOpenDLG	4
#define			kStopWhenKeyUp		8
#define			kDTDTime			16

#define			kQSmplStart			1
#define			kQSmplPitchAlign	2
#define			kQSmplStop			4
#define			kQRB				8

//CONSTANTES MIDI FLAGS:
#define			kMIDIForeGround		1
#define			kMIDIAutoConnect	2
#define			kUseMidiMgr			4
#define			kUseHFClock			8