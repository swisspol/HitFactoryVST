#pragma	SC once

/**************************************/
#include			<CodeFragments.h>
#if defined(powerc) || defined (__powerc)
#include			<SoundSprocket.h>

enum {
	siPreMixerSoundComponent	= 'prmx',
	siSSpCPULoadLimit			= '3dll',
	siSSpSetup					= '3dst',
	siSSpLocalization			= '3dif',
	siSSpFilterVersion			= '3dfv'
};
#endif
/**************************************/

//STRUCTURES:

typedef struct sample
{
	short			fileID;
	short			type;
	short			sndNb;
	short			totalSndNb;
	long			time;
	
	Handle			snd;
	SndChannelPtr	chan;
	
	Str63			name;
	FSSpec			spec;
};
	
typedef struct instrument
{
	short			fileID;
	
	Handle			snd;
	
	Str63			name;
	FSSpec			spec;
};

typedef struct rythm
{
	short			fileID;
	
	Handle			snd;
	
	Str63			name;
	FSSpec			spec;
};

typedef struct record
{
	long				time;
	unsigned short	action;
	long				command;
	long				command_2;
};

typedef struct prefs
{
	//Infos générales:
	Str63				registed;
	Str255				userName;
	FSSpec				soundsFolder;
	
	//Options:
	Boolean			loopDTD;
	unsigned char		flags;
	unsigned char		quantizeFlags;
	
	//Fenêtres:
	Boolean			samplesWin;
	Boolean			smpWinExt;
	Boolean			recordWin;
	Boolean			recWinExt;
	Boolean			instrumentsWin;
	Boolean			insWinExt;
	Boolean			rythmsWin;
	Boolean			rytWinExt;
	Point				smpWinPos;
	Point				recWinPos;
	Point				insWinPos;
	Point				rytWinPos;
	
	//Configuration:
	short				nbSamples;
	unsigned char		nbInsChannels;
	unsigned char		nbRytChannels;
	unsigned char		reserved1;
	unsigned char		reserved2;
	long				bufferSize;
	long				outputVol;
	short				loopSpeed;
	
	//MIDI:
	Boolean			enableMIDIDriver;
	unsigned char		MIDIFlags;
	short				MIDIDriverMode;
	unsigned char		MIDISamplerChannel;
	unsigned char		MIDISynthChannel;
	unsigned char		MIDIRBChannel;
	unsigned char		reserved3;
	
	//SAMPLER:
	Boolean			SReverb;
	short				STurbo;
	long				SSourceMode[maxNbSamples];
	short				SStatus[maxNbSamples];
	short				SGroup[maxNbSamples];
	short				SSndNb[maxNbSamples];
	short				SVol[maxNbSamples];
	short				SPan[maxNbSamples];
	Fixed				SRate[maxNbSamples];
	short				SRelativeRate[maxNbSamples];
	short				SLoop[maxNbSamples];
	Boolean			SPlaying[maxNbSamples];
	short				SMaxVol[maxNbSamples];
	
	//SYNTHETISEUR:
	short				synthMode;
	Boolean			IReverb;
	long				ISourceMode;
	short				ITurbo;
	short				IVol;
	short				IMaxVol;
	
	//BOITE A RYTHMES:
	Boolean			RReverb;
	long				RSourceMode;
	short				RTurbo;
	Boolean			RPlaying;
	short				RBPM;
	short				RCurrent;
	short				RNext;
	short				RVol;
	short				RMaxVol;
	short				RPartVol[kPatternParts];
	short				RPartPan[kPatternParts];
	short				RPartPitch[kPatternParts];
	Boolean			RPartMute[kPatternParts];
	
	//GROUPES:
	Boolean			enableGroups;
	Boolean			GRelative;
	Boolean			GPlay;
	Boolean			GVol;
	Boolean			GPan;
	Boolean			GPitch;
	Boolean			GOptions;
	Boolean			GInit;
	
	//COULEURS:
	RGBColor			sampleColor;
	RGBColor			DTDColor;
	RGBColor			displayColor;
	
	//EFFETS:
#if defined(powerc) || defined (__powerc)
	SSpLocalizationData	localization;
#else
	char				localization[196];
#endif
};

typedef struct ExtTMTask
{
	TMTask				theTask;
	long				appA5;
};
typedef ExtTMTask* ExtTMTaskPtr;

typedef struct openData
{
	StandardFileReply*	reply;
	Boolean			extendedDLOG;
	Boolean			update;
	OSType*			fileListPtr;
	Boolean			addBank;
};

//IMPORT PLUG-INS STRUCTURES:

typedef struct importPlugIn
{
	ConnectionID	connID;
	OSType			importFileTypes[kTypesPerImport];
};

typedef struct importPlugInInfos
{
	OSType			importFileType[kTypesPerImport];
	long			flags;
	short			minimumVersion,
					lastVersion;
};
typedef importPlugInInfos* importPlugInInfosPtr;

//DIRECT TO DISK PLUG-INS STRUCTURES:

typedef struct DTDPlugIn
{
	ConnectionID	connID;
	OSType			DTDFileTypes[kTypesPerDTD];
};

typedef struct DTDPlugInInfos
{
	OSType			DTDFileType[3];
	long			flags;
	short			minimumVersion,
					lastVersion;
};
typedef DTDPlugInInfos* DTDPlugInInfosPtr;

typedef struct DTDPlugInParameters
{
	SndChannelPtr	channel;
	short			fileID;
	Handle			bufferHandle;
};
typedef DTDPlugInParameters* DTDPlugInParametersPtr;

//EDITOR PLUG-INS STRUCTURES:

typedef struct editorPlugIn
{
	ConnectionID	connID;
	short			resFile;
	Boolean		update;
};

typedef struct editorPlugInInfos
{
	Str31 			name;
	long			flags;
	short			minimumVersion,
					lastVersion;
};
typedef editorPlugInInfos* editorPlugInInfosPtr;