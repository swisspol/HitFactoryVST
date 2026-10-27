#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"
#include			"Record Codes.h"

//EXPORT:

#define						kAsync								true

//CONSTANTES LOCALES:

#define						kNoiseMakerVersion					0x00010000

#define						kNoiseMakerManufacturer				'Pol '
#define						kNoiseMakerType						kSoundOutputDeviceType
#define						kNoiseMakerSubType					kClassicSubType

#define						kDelegateComponentCall				((ComponentRoutineUPP) -1L)

#define						siOutputChannel						'OtCh'
#define						siSize								'Size'
#define						siSetMusicData						'MsDt'
#define						siBufferSize						'BfSz'
#define						siStart								'Strt'
#define						siStop								'Stop'

#if pro
	#define						kNumChannels						2
	#define						kSampleRate							rate44khz
	#define						kSampleSize							16
	
	#define						kInterruptBufferSamples				128 //512
#else
	#define						kNumChannels						1
	#define						kSampleRate							rate22050hz
	#define						kSampleSize							16
	
	#define						kInterruptBufferSamples				64 //128
#endif

enum {
	uppCallComponentInitOutputDeviceProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(long))),
	uppCallComponentSetInfoProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(SoundSource)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSType)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(void*))),
	uppCallComponentGetInfoProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(SoundSource)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(OSType)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(void*))),
	uppCallComponentStartSourceProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(short)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(SoundSource *))),
	uppCallComponentPlaySourceBufferProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(ComponentResult)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(Ptr)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(SoundSource)))
		 | STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(SoundParamBlockPtr)))
		 | STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(long)))
};

//STRUCTURES LOCALES:

typedef struct musicData
{
	Handle					buffer;
	long					length;
	Boolean				ready;
};
	
typedef struct {
	ComponentInstance		self;
	ComponentInstance		sourceComponent;
	SoundComponentDataPtr	sourceDataPtr;
	Handle					globalsHandle;
	
	musicData*				music;
	short					currentBuffer;
	long					bufferSize,
							totalSize;
	Boolean				run;
	
	SoundComponentData		hwSettings;
	unsigned long			hwVolume;
	Boolean				hwInterruptsOn;
	Boolean				hwInitialized;
	SndChannelPtr			sndChan;
}
GlobalsRecord, *GlobalsPtr;

//PROTOTYPES:

pascal ComponentResult	NoiseMaker(ComponentParameters *params, GlobalsPtr globals);
ComponentRoutineUPP		GetComponentRoutine(short selector, long*);

OSErr					SetupHardware(GlobalsPtr globals);
void					ReleaseHardware(GlobalsPtr globals);
OSErr					StartHardware(GlobalsPtr globals);
void					StopHardware(GlobalsPtr globals);
OSErr					SetHardwareVolume(GlobalsPtr globals, unsigned long volume);
pascal void			InterruptRoutine(SndChannelPtr chan, SndCommand *cmd);
SoundComponentDataPtr	GetMoreSource(GlobalsPtr globals);
void					CopySamplesToHardware(GlobalsPtr globals, SoundComponentDataPtr sourceDataPtr);

//VARIABLES LOCALES:

UniversalProcPtr		NoiseMakerRoutineUPP = NewComponentRoutineProc(NoiseMaker);
UniversalProcPtr		InterruptRoutineUPP = NewSndCallBackProc(InterruptRoutine);

//FONCTIONS:

static pascal ComponentResult __ComponentRegister(void *unused1)
{
	return 0;
}

static pascal ComponentResult __ComponentVersion(void *unused1)
{
	return kNoiseMakerVersion;
}

static pascal ComponentResult __ComponentCanDo(void *unused1, short selector)
{
	ComponentRoutineUPP	theRtn;
	long				procInfo;
	
	theRtn = GetComponentRoutine(selector, &procInfo);

	if((theRtn == nil) || (theRtn == kDelegateComponentCall))
	return 0;
	else
	return 1;
}

static pascal ComponentResult __ComponentOpen(void *unused1, ComponentInstance self)
{
	Handle			h;
	GlobalsPtr		globals;

	h = NewHandleClear(sizeof(GlobalsRecord));
	if(h == nil)
	return(MemError());

	HLock(h);
	globals = (GlobalsPtr) *h;
	SetComponentInstanceStorage(self, (Handle) globals);

	globals->globalsHandle = h;
	
	return noErr;
}

static pascal ComponentResult __ComponentClose(GlobalsPtr globals, ComponentInstance self)
{
	if(globals)	{
		if(globals->hwInitialized)
		ReleaseHardware(globals);

		if(globals->sourceComponent)
		CloseMixerSoundComponent(globals->sourceComponent);

		DisposeHandle(globals->globalsHandle);
	}

	return noErr;
}

static pascal ComponentResult __InitOutputDevice(GlobalsPtr globals, long actions)
{
	ComponentResult		result;
	
	globals->hwSettings.flags = 0;
	if(kSampleSize == 16)
	globals->hwSettings.format = kTwosComplement;
	else
	globals->hwSettings.format = kOffsetBinary;
	globals->hwSettings.sampleRate = kSampleRate;
	globals->hwSettings.sampleSize = kSampleSize;
	globals->hwSettings.numChannels = kNumChannels;
	globals->hwSettings.sampleCount = kInterruptBufferSamples * 2;

	result = OpenMixerSoundComponent(&globals->hwSettings, 0, &globals->sourceComponent);
	if(result != noErr)
	return result;

	result = SetupHardware(globals);

	if(result == noErr)
	globals->hwInitialized = true;

	globals->currentBuffer = 0;
	globals->run = false;
	globals->totalSize = 0L;
	globals->bufferSize = 0L;
	
	return result;
}

static pascal ComponentResult __GetInfo(GlobalsPtr globals, SoundSource sourceID, OSType selector, void *infoPtr)
{
	ComponentResult		result;
	
	switch(selector) {
		
		case siOutputChannel:
		*((SndChannelPtr*) infoPtr) = globals->sndChan;
		break;
		
		case siSize:
		*((long*) infoPtr) = globals->totalSize;
		break;
		
		default:
		result = SoundComponentGetInfo(globals->sourceComponent, sourceID, selector, infoPtr);
		break;

	}

	return result;
}

static pascal ComponentResult __SetInfo(GlobalsPtr globals, SoundSource sourceID, OSType selector, void *infoPtr)
{
	ComponentResult		result = noErr;

	switch(selector) {
		
		case siHardwareVolume:
		globals->hwVolume = (unsigned long) infoPtr;	
		result = SetHardwareVolume(globals, globals->hwVolume);
		break;

		case siSetMusicData:
		globals->music = (musicData*) infoPtr;
		globals->music[0].length = 0L;
		globals->music[1].length = 0L;
		globals->music[0].ready = false;
		globals->music[1].ready = false;
		break;
		
		case siBufferSize:
		globals->bufferSize = (long) infoPtr;
		break;
		
		case siStart:
		globals->run = true;
		break;
		
		case siStop:
		globals->run = false;
		break;
		
		default:
		result = SoundComponentSetInfo(globals->sourceComponent, sourceID, selector, infoPtr);
		break;
		
	}

	return result;
}

static pascal ComponentResult __StartSource(GlobalsPtr globals, short count, SoundSource *sources)
{
	ComponentResult		result;

	result = SoundComponentStartSource(globals->sourceComponent, count, sources);
	if(result != noErr)
	return result;

	result = StartHardware(globals);

	return result;
}

static pascal ComponentResult __PlaySourceBuffer(GlobalsPtr globals, SoundSource sourceID, SoundParamBlockPtr pb, long actions)
{
	ComponentResult		result;

	result = SoundComponentPlaySourceBuffer(globals->sourceComponent, sourceID, pb, actions);
	if(result != noErr)
	return (result);

	if(!(actions & kSourcePaused))
	result = StartHardware(globals);

	return result;
}

OSErr SetupHardware(GlobalsPtr globals)
{
	OSErr					err;

	globals->sndChan = nil;
	err = SndNewChannel(&globals->sndChan, sampledSynth, initStereo, InterruptRoutineUPP);
	
	return err;
}

void ReleaseHardware(GlobalsPtr globals)
{
	StopHardware(globals);

	SndDisposeChannel(globals->sndChan, true);
}

OSErr StartHardware(GlobalsPtr globals)
{
	OSErr		err = noErr;
	SndCommand	cmd;

	if(!globals->hwInterruptsOn) {
		globals->hwInterruptsOn = true;
		
		cmd.cmd = callBackCmd;
		cmd.param1 = 0;
		cmd.param2 = (long) globals;
	
		err = SndDoCommand(globals->sndChan, &cmd, true);
	}

	return err;
}

void StopHardware(GlobalsPtr globals)
{
	//SndCommand	cmd;

	if(globals->hwInterruptsOn) {
		/*cmd.cmd = quietCmd;
		cmd.param1 = 0;
		cmd.param2 = 0;
		SndDoImmediate(globals->sndChan, &cmd);*/

		globals->hwInterruptsOn = false;
	}
}

OSErr SetHardwareVolume(GlobalsPtr globals, unsigned long volume)
{
	SndCommand	cmd;
	OSErr		err;

	cmd.cmd = volumeCmd;
	cmd.param1 = 0;
	cmd.param2 = volume;

	err = SndDoImmediate(globals->sndChan, &cmd);
	
	return err;
}

pascal void InterruptRoutine(SndChannelPtr chan, SndCommand *cmd)
{	
	GlobalsPtr				globals;
	SoundComponentDataPtr	sourceDataPtr;

	globals = (GlobalsPtr) cmd->param2;

	sourceDataPtr = GetMoreSource(globals);
	if(sourceDataPtr == nil) {
		StopHardware(globals);
		return;
	}

	CopySamplesToHardware(globals, sourceDataPtr);

	if(globals->hwInterruptsOn) {
		cmd->cmd = callBackCmd;
		cmd->param1 = 0;
		cmd->param2 = (long) globals;
	
		SndDoCommand(globals->sndChan, cmd, true);
	}
}

SoundComponentDataPtr GetMoreSource(GlobalsPtr globals)
{
	ComponentResult			result;
	SoundComponentDataPtr	sourceDataPtr = globals->sourceDataPtr;

	if((sourceDataPtr == nil) || (sourceDataPtr->sampleCount == 0)) {
		result = SoundComponentGetSourceData(globals->sourceComponent, &globals->sourceDataPtr);
		sourceDataPtr = globals->sourceDataPtr;

		if((result != noErr) ||	(sourceDataPtr == nil) || (sourceDataPtr->sampleCount == 0))
		return nil;
	}
	
	return sourceDataPtr;
}

void CopySamplesToHardware(GlobalsPtr globals, SoundComponentDataPtr sourceDataPtr)
{
	SndCommand		cmd;
	ExtSoundHeader	sndHeader;
	OSErr			err;
	long			size;

	if(globals->run) {
#if pro
		size = sourceDataPtr->sampleCount * sourceDataPtr->sampleSize / 4;
#else
		size = sourceDataPtr->sampleCount * sourceDataPtr->sampleSize / 8;
#endif
		if(globals->music[globals->currentBuffer].length + size > globals->bufferSize) {
			globals->music[globals->currentBuffer].ready = true;
			if(globals->currentBuffer == 1)
			globals->currentBuffer = 0;
			else
			globals->currentBuffer = 1;
			globals->music[globals->currentBuffer].length = 0;
		}
		BlockMove(sourceDataPtr->buffer, *(globals->music[globals->currentBuffer].buffer) 
			+ globals->music[globals->currentBuffer].length, size);
		globals->music[globals->currentBuffer].length += size;
		
		globals->totalSize += size;
	}

	sndHeader.samplePtr = (Ptr) sourceDataPtr->buffer;
	sndHeader.numChannels = sourceDataPtr->numChannels;
	sndHeader.sampleRate = sourceDataPtr->sampleRate;
	sndHeader.loopStart = 0;
	sndHeader.loopEnd = 0;
	sndHeader.encode = extSH;
	sndHeader.baseFrequency = 60;
	sndHeader.numFrames = sourceDataPtr->sampleCount;
	sndHeader.markerChunk = 0;
	sndHeader.instrumentChunks = 0;
	sndHeader.AESRecording = 0;
	sndHeader.sampleSize = sourceDataPtr->sampleSize;
	sndHeader.futureUse1 = 0;
	sndHeader.futureUse2 = 0;
	sndHeader.futureUse3 = 0;
	sndHeader.futureUse4 = 0;

	cmd.cmd = bufferCmd;
	cmd.param1 = 0;
	cmd.param2 = (long) &sndHeader;

	err = SndDoImmediate(globals->sndChan, &cmd);

	sourceDataPtr->sampleCount = 0;
}

static ComponentRoutineUPP GetComponentRoutine(short selector, long* funcProcInfo)
{
	void*		theRtn;

	if(selector < 0)
	switch(selector) {
		
		case kComponentRegisterSelect:
		theRtn = __ComponentRegister;
		*funcProcInfo = uppCallComponentRegisterProcInfo;
		break;

		case kComponentVersionSelect:
		theRtn = __ComponentVersion;
		*funcProcInfo = uppCallComponentVersionProcInfo;
		break;

		case kComponentCanDoSelect:
		theRtn = __ComponentCanDo;
		*funcProcInfo = uppCallComponentCanDoProcInfo;
		break;

		case kComponentCloseSelect:
		theRtn = __ComponentClose;
		*funcProcInfo = uppCallComponentCloseProcInfo;
		break;

		case kComponentOpenSelect:
		theRtn = __ComponentOpen;
		*funcProcInfo = uppCallComponentOpenProcInfo;
		break;

		default:
		theRtn = nil;
		break;
		
	}
	else
		if(selector < kDelegatedSoundComponentSelectors)
		switch(selector) {
			case kSoundComponentInitOutputDeviceSelect:
			theRtn = __InitOutputDevice;
			*funcProcInfo = uppCallComponentInitOutputDeviceProcInfo;
			break;

			default:
			theRtn = nil;
			break;
		}
		else
		switch(selector) {
			
			case kSoundComponentSetInfoSelect:
			theRtn = __SetInfo;
			*funcProcInfo = uppCallComponentSetInfoProcInfo;
			break;
			
			case kSoundComponentGetInfoSelect:
			theRtn = __GetInfo;
			*funcProcInfo = uppCallComponentGetInfoProcInfo;
			break;

			case kSoundComponentStartSourceSelect:
			theRtn = __StartSource;
			*funcProcInfo = uppCallComponentStartSourceProcInfo;
			break;
			
			case kSoundComponentPlaySourceBufferSelect:
			theRtn = __PlaySourceBuffer;
			*funcProcInfo = uppCallComponentPlaySourceBufferProcInfo;
			break;

			default:
			theRtn = kDelegateComponentCall;
			break;
			
		}

	return (RoutineDescriptor*) theRtn;
}

pascal ComponentResult NoiseMaker(ComponentParameters *params, GlobalsPtr globals)
{
	ComponentRoutineUPP	theRtn;
	ComponentResult		result;
	long				theProcInfo;

	theRtn = GetComponentRoutine(params->what, &theProcInfo);

	if(theRtn == nil)
	result = badComponentSelector;
	else
		if(theRtn == kDelegateComponentCall)
		result = DelegateComponentCall(params, globals->sourceComponent);
		else
		result = CallComponentFunctionWithStorageProcInfo((Handle) globals, params, (ProcPtr) theRtn, theProcInfo);

	return result;
}

OSErr RegisterMixer()
{
	ComponentDescription		noiseMakerDescription;
	Component					componentID;
	OSErr						result;
	
	//Register the component
	noiseMakerDescription.componentType				= kNoiseMakerType;
	noiseMakerDescription.componentSubType			= kNoiseMakerSubType;
	noiseMakerDescription.componentManufacturer		= kNoiseMakerManufacturer;
#if pro
	noiseMakerDescription.componentFlags			= k16BitIn | kStereoIn
														| k16BitOut | kStereoOut | kHighQuality; // kReverse kRealTime  cmpWantsRegisterMessage
#else
	noiseMakerDescription.componentFlags			= k8BitRawIn | kStereoIn
														| k8BitRawOut | kStereoOut | kHighQuality;
#endif
	noiseMakerDescription.componentFlagsMask	= 0L;
	componentID = RegisterComponent(&noiseMakerDescription, NoiseMakerRoutineUPP, 0, nil, nil, nil);
	if(!componentID)
	result = -1;
	else
	result = 0;
	
	return result;
}

unsigned long Get_FreeSpace(short vRefNum)
{
	HParamBlockRec		blk;
	OSErr				err;
	
	blk.volumeParam.ioCompletion = nil;
	blk.volumeParam.ioNamePtr = nil;
	blk.volumeParam.ioVRefNum = vRefNum;
	blk.volumeParam.ioVolIndex = 0;
	err = PBHGetVInfo(&blk, false);
	if(err)
	return 0L;
	
	return (blk.volumeParam.ioVFrBlk * blk.volumeParam.ioVAlBlkSiz / 1024) /* * 0.98 */;
}

void CleanAndLoopSound(Handle aSound)
{
	SoundHeaderPtr		stdHeader;
	ExtSoundHeaderPtr	extHeader;
	long				offset;
	
	HLock(aSound);
	
	GetSoundHeaderOffset((SndListHandle) aSound, &offset);
	BlockMove(*aSound + offset, *aSound, GetHandleSize(aSound) - offset);
	stdHeader = (SoundHeaderPtr) *aSound;
	extHeader = (ExtSoundHeaderPtr) *aSound;
	
	stdHeader->samplePtr		= nil;
	
	stdHeader->loopStart	= 0;
	if(stdHeader->encode == extSH)
	stdHeader->loopEnd		= extHeader->numFrames;
	else
	stdHeader->loopEnd		= stdHeader->length;
	
	HUnlock(aSound);
	SetHandleSize(aSound, GetHandleSize(aSound) - offset);
}

OSErr Music2AIFF()
{	
	StandardFileReply		theReply;
	short					temp,
							fileNumber;
	OSType					fileType,
							fileCreator;						
	Boolean				feedback,
							loop = true;
	SndChannelPtr			continueChannel = nil,
							outputChannel = nil;
	Handle					silenceSound;
	Str255					defaultName,
							DLGText;
	Component				sndComponent;
	ComponentDescription	description;
	SoundComponentLink		theLink;
	musicData				data[2];
	long					bufferSize,
							length,
							startTick,
							oldTick;
	Str31					musicLength,
							currentLength;
#if kAsync
	ParamBlockRec			param;
#endif
	
#if demo
	Do_Error(-30008, 904);
	return false;
#else	
	//Durée:
	for(i = 0; i < nbNotes; ++i)
	if(tempRec[i].action == kEnd)
	break;
	length = tempRec[i].time * 105 / 10000; //Correction 5%
	
	NumToString(length / 60, musicLength);
	musicLength[musicLength[0] + 1] = ':';
	musicLength[musicLength[0] + 2] = '0';
	musicLength[musicLength[0] + 3] = '0';
	NumToString(length % 60, currentLength);
	BlockMove(&currentLength[1], &musicLength[musicLength[0] + 4 - currentLength[0]], currentLength[0]);
	musicLength[0] += 3;
#if pro
	NumToString(length * 176400 / 1000, currentLength);
#else
	NumToString(length * 44100 / 1000, currentLength);
#endif
	ParamText(musicLength, currentLength, nil, nil);
	
	//OPTIONS:
	theDialog = GetNewDialog(9100, nil, (WindowPtr) -1);
	GetDItem(theDialog, 3, nil, &dialogItem[0], &aRect);
	GetDItem(theDialog, 4, nil, &dialogItem[1], &aRect);
	GetDItem(theDialog, 5, nil, &dialogItem[2], &aRect);
	GetDItem(theDialog, 6, nil, &dialogItem[3], &aRect);
	GetDItem(theDialog, 23, nil, &dialogItem[4], &aRect);
	
	SetGWorld((CGrafPtr) theDialog, nil);
	//SetCtlValue((ControlHandle) dialogItem[2], true);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	OutLine_Item(theDialog, 7);
	OutLine_Item(theDialog, 9);
#if pro
	SetIText(dialogItem[4], "\p16Bits 44kHz\rstereo");
#else
	SetIText(dialogItem[4], "\p16Bits 22kHz\rmono");
#endif
	SelIText(theDialog, 3, 0, 32767); //Misc
	SetDialogDefaultItem(theDialog, 1);
	
	while(loop) {
		ModalDialog(nil, &itemHit);
		switch(itemHit) {
			case 1:
			case 2:
			loop = false;
			break;
			
			case 5:
			i = GetCtlValue((ControlHandle) dialogItem[2]);
			SetCtlValue((ControlHandle) dialogItem[2], !i);
			break;
			
			case 15:
			HMSetBalloons(!HMGetBalloons());
			if(!HMGetBalloons()) {
				OutLine_Item(theDialog, 7);
				OutLine_Item(theDialog, 9);
				DrawDialog(theDialog);
			}
			break;
			
		}
	}
	if(itemHit == 2) {
		DisposeDialog(theDialog);
		for(i = 0; i < 20; ++i)
		DisposeHandle(dialogItem[i]);
		return false;
	}
	
	GetIText(dialogItem[0], theString);
	BlockMove(&theString[1], &fileType, sizeof(OSType));
	GetIText(dialogItem[1], theString);
	BlockMove(&theString[1], &fileCreator, sizeof(OSType));
	feedback = GetCtlValue((ControlHandle) dialogItem[2]);
	GetIText(dialogItem[3], theString);
	StringToNum(theString, &bufferSize);
	bufferSize = bufferSize * 1024;
	DisposeDialog(theDialog);
	for(i = 0; i < 20; ++i)
	DisposeHandle(dialogItem[i]);
	
	//Où sauver?
	Where:
	UseResFile(mainResFile);
	GetIndString(defaultName, 0, 3);
	GetIndString(DLGText, 0, 4);
	CustomPutFile(DLGText, defaultName, &theReply, 30000, whereToShow, nil,
		nil, nil, nil, nil);
	if(!theReply.sfGood)
	return false;
	
	//Tester espace disque
#if pro
	if(Get_FreeSpace(theReply.sfFile.vRefNum) < (length * 176400 / 1000 + 100)) {
#else
	if(Get_FreeSpace(theReply.sfFile.vRefNum) < (length * 44100 / 1000 + 100)) {
#endif
		Do_Error(-34, 110);
		goto Where;
	}
	
	Anim_WaitCursor();
	//Crée fichier
	if(theReply.sfReplacing) { //Efface le fichier si existe déjà
		theError = FSpDelete(&theReply.sfFile);
		if(theError) {
			Do_Error(theError, 104);
			return false;
		}
	}
	theError = FSpCreate(&theReply.sfFile, fileCreator, fileType, theReply.sfScript); //Crée le fichier
	if(theError) {
		Do_Error(theError, 101);
		return false;
	}
	theError = FSpOpenDF(&theReply.sfFile, fsRdWrPerm, &fileNumber); //Ouvre le fichier
	if(theError) {
		Do_Error(theError, 102);
		return false;
	}
	SetFPos(fileNumber, 1, 0);
	SetupAIFFHeader(fileNumber, kNumChannels, kSampleRate, kSampleSize, 'NONE', 0, 0);
	
	Anim_WaitCursor();
	//Ferme tout:
	for(i = 0; i < thePrefs.nbSamples; ++i) {
		SndDisposeChannel(samples[i].chan, true);
		samples[i].chan = nil;
	}
	for(i = 0; i < thePrefs.nbInsChannels; ++i) {
		SndDisposeChannel(insChannels[i], true);
		insChannels[i] = nil;
	}
	for(i = 0; i < thePrefs.nbRytChannels; ++i) {
		SndDisposeChannel(rytChannels[i], true);
		rytChannels[i] = nil;
	}
	
	Anim_WaitCursor();
	//Enregistre le mixer:
	theError = RegisterMixer();
	if(theError) {
		Do_Error(theError, 514);
		return false;
	}
	description.componentType			= kNoiseMakerType;
	description.componentSubType		= kNoiseMakerSubType;
	description.componentManufacturer	= kNoiseMakerManufacturer;
	description.componentFlags			= 0L;
	description.componentFlagsMask		= kAnyComponentFlagsMask;
	sndComponent = FindNextComponent(0, &description);
	if(sndComponent == nil) {
		Do_Error(theError, 514);
		return false;
	}
	
	Anim_WaitCursor();
	//Prépare les données:
	data[0].buffer = NewHandle(bufferSize);
	data[1].buffer = NewHandle(bufferSize);
	if(data[0].buffer == nil || data[1].buffer == nil) {
		Do_Error(-108, 0);
		return false;
	}
	HLock(data[0].buffer);
	HLock(data[1].buffer);
	
	Anim_WaitCursor();
	//Silence en continu:
	silenceSound = Get1Resource('snd ', 128);
	DetachResource(silenceSound);
	CleanAndLoopSound(silenceSound);
	HLock(silenceSound);
	theError = SndNewChannel(&continueChannel, kUseOptionalOutputDevice, (long) sndComponent, nil);
	if(theError) {
		Do_Error(theError, 504);
		return false;
	}
	if(feedback) {
		SndGetInfo(continueChannel, siOutputChannel, (void*) &outputChannel);
		theCommand.cmd		= soundCmd;
		theCommand.param1	= 0;
		theCommand.param2	= (long) *silenceSound;
		SndDoImmediate(outputChannel, &theCommand);
	}
	else
	SndSetInfo(continueChannel, siHardwareVolume, (void*) 0L);
	SndSetInfo(continueChannel, siSetMusicData, (void*) data);
	SndSetInfo(continueChannel, siBufferSize, (void*) bufferSize);
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *silenceSound;
	SndDoImmediate(continueChannel, &theCommand);
	theCommand.cmd		= freqCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 60;
	SndDoImmediate(continueChannel, &theCommand);
	
	Anim_WaitCursor();
	//Recrée les SndChannels
	theLink.description.componentType			= kSoundEffectsType;
	theLink.description.componentSubType		= kSSpLocalizationSubType;
	theLink.description.componentManufacturer	= 0;
	theLink.description.componentFlags			= 0;
	theLink.description.componentFlagsMask		= 0;
	theLink.mixerID								= nil;
	theLink.linkID								= nil;
	for(i = 0; i < thePrefs.nbSamples; ++i) {	
		theError = SndNewChannel(&samples[i].chan, kUseOptionalOutputDevice, (long) sndComponent, nil);
		if(theError) {
			Do_Error(theError, 504);
			return false;
		}
		theCommand.cmd		= reInitCmd;
		theCommand.param1	= 0;
		theCommand.param2	= initStereo;
		SndDoImmediate(samples[i].chan, &theCommand);
		samples[i].chan->userInfo = SetCurrentA5();
		if(thePrefs.SReverb && samples[i].chan) {
			theError = SndSetInfo(samples[i].chan, siPreMixerSoundComponent, &theLink);
			if(theError) {
				Do_Error(theError, 507);
				return false;
			}
		}
		theCommand.cmd		= soundCmd;
		theCommand.param1	= 0;
		if(samples[i].fileID)
		theCommand.param2	= (long) *samples[i].snd;
		else
		theCommand.param2	= (long) *noSound;
		SndDoImmediate(samples[i].chan, &theCommand);
	}
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *noSound;
	for(i = 0; i < maxNbInstruments; ++i)
		if(instruments[i].fileID)
		theCommand.param2	= (long) *instruments[i].snd;
	for(i = 0; i < thePrefs.nbInsChannels; ++i) {
		theError = SndNewChannel(&insChannels[i], kUseOptionalOutputDevice, (long) sndComponent, nil);
		if(theError) {
			Do_Error(theError, 504);
			return false;
		}
		SndDoImmediate(insChannels[i], &theCommand);
	}
	SCD_SetSynthetizerMode(thePrefs.synthMode);
	temp = -1;
	for(i = 0; i < kPatternParts; ++i)
		if(rythms[i].fileID)
		temp = i;
	for(i = 0; i < thePrefs.nbRytChannels; ++i) {
		theError = SndNewChannel(&rytChannels[i], kUseOptionalOutputDevice, (long) sndComponent, nil);
		if(theError) {
			Do_Error(theError, 504);
			return false;
		}
		theCommand.cmd		= reInitCmd;
		theCommand.param1	= 0;
		theCommand.param2	= initStereo;
		SndDoImmediate(rytChannels[i], &theCommand);
		theCommand.cmd		= soundCmd;
		theCommand.param1	= 0;
		if(temp == -1)
		theCommand.param2	= (long) *noSound;
		else
		theCommand.param2	= (long) *rythms[temp].snd;
		SndDoImmediate(rytChannels[i], &theCommand);
	}
	
	Anim_WaitCursor();
	//Montre le dialogue:
	ParamText(musicLength, nil, nil, nil);
	theDialog = GetNewDialog(9110, nil, (WindowPtr) -1);
	GetDItem(theDialog, 1, nil, &dialogItem[0], &aRect);
	GetDItem(theDialog, 2, nil, &dialogItem[1], &aRect);
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	SetCtlMin((ControlHandle) dialogItem[0], 0);
	SetCtlMax((ControlHandle) dialogItem[0], length);
	SetCtlValue((ControlHandle) dialogItem[0], 0);
	OutLine_Item(theDialog, 4);
	DrawDialog(theDialog);
	
	Anim_WaitCursor();
	//Prépare la musique:
	playing = true;
	Install_Music();
	activeChan = 0;
	writePos = readPos = 0;
	SndSetInfo(continueChannel, siStart, nil);
	TM_InstallReadTask();
	startTick = TickCount();
	oldTick = startTick / 60;
	
#if kAsync
	param.ioParam.ioCompletion = 0L;
	param.ioParam.ioRefNum = fileNumber;
	param.ioParam.ioPosMode = 0;
	param.ioParam.ioPosOffset = 0L;
	param.ioParam.ioResult = 0;
	param.ioParam.ioActCount = 0L;
#endif
	
	//Run
	while(!Button()) {
		if(((TickCount() - startTick) / 60) > length)
		break;
		if(data[0].ready) {
			bufferSize = data[0].length;
#if kAsync
			param.ioParam.ioBuffer = *(data[0].buffer);
			param.ioParam.ioReqCount = bufferSize;
			theError = PBWrite(&param, true);
#else			
			theError = FSWrite(fileNumber, &bufferSize, *(data[0].buffer));
#endif
			data[0].ready = false;
			if(theError) {
				Do_Error(theError, 103);
				break;
			}
		}
		if(data[1].ready) {
			bufferSize = data[1].length;
#if kAsync			
			param.ioParam.ioBuffer = *(data[1].buffer);
			param.ioParam.ioReqCount = bufferSize;
			theError = PBWrite(&param, true);
#else
			theError = FSWrite(fileNumber, &bufferSize, *(data[1].buffer));
#endif
			data[1].ready = false;
			if(theError) {
				Do_Error(theError, 103);
				break;
			}
		}
		if(TickCount() / 60 > oldTick) {
			//Anim_WaitCursor();
			SetCtlValue((ControlHandle) dialogItem[0], (TickCount() - startTick) / 60);
			NumToString(((TickCount() - startTick) / 60) / 60, currentLength);
			currentLength[currentLength[0] + 1] = ':';
			currentLength[currentLength[0] + 2] = '0';
			currentLength[currentLength[0] + 3] = '0';
			NumToString(((TickCount() - startTick) / 60) % 60, theString);
			BlockMove(&theString[1], &currentLength[currentLength[0] + 4 - theString[0]], theString[0]);
			currentLength[0] += 3;
			SetIText(dialogItem[1], currentLength);
			
			oldTick = TickCount() / 60;
		}
	}
	
	//Arrête la musique:
	TM_KillReadTask();
	SndSetInfo(continueChannel, siStop, nil);
	for(i = 0; i < thePrefs.nbSamples; ++i)
	if(thePrefs.SPlaying[i]) {
		if(samples[i].type == sampleType)
		SCD_StopSample(i);
		else
		SCD_StopDirectToDisk(i);
	}
	chanFinished = nil;
	theCommand.cmd		= quietCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	for(i = 0; i < thePrefs.nbInsChannels; ++i)
	SndDoImmediate(insChannels[i], &theCommand);
	RB_Stop();
	for(i = 0; i < thePrefs.nbRytChannels; ++i)
	SndDoImmediate(rytChannels[i], &theCommand);
	playing = false;
	recording = false;
	position = 0;
	
	//Ferme le dialogue:
	DisposeDialog(theDialog);
	for(i = 0; i < 20; ++i)
	DisposeHandle(dialogItem[i]);
	
	//Ecrit header:
	SndGetInfo(continueChannel, siSize, (void*) &bufferSize);
	SetFPos(fileNumber, 1, 0);
	SetupAIFFHeader(fileNumber, kNumChannels, kSampleRate, kSampleSize, 'NONE', bufferSize, 0);
	FSClose(fileNumber);
	FlushVol(nil, theReply.sfFile.vRefNum);
		
	Anim_WaitCursor();
	//Ferme tout:
	for(i = 0; i < thePrefs.nbSamples; ++i) {
		SndDisposeChannel(samples[i].chan, true);
		samples[i].chan = nil;
	}
	for(i = 0; i < thePrefs.nbInsChannels; ++i) {
		SndDisposeChannel(insChannels[i], true);
		insChannels[i] = nil;
	}
	for(i = 0; i < thePrefs.nbRytChannels; ++i) {
		SndDisposeChannel(rytChannels[i], true);
		rytChannels[i] = nil;
	}
	SndDisposeChannel(continueChannel, true);
	HUnlock(silenceSound);
	DisposeHandle(silenceSound);
	UnregisterComponent(sndComponent);
	HUnlock(data[0].buffer);
	HUnlock(data[1].buffer);
	DisposeHandle(data[0].buffer);
	DisposeHandle(data[1].buffer);
	
	Anim_WaitCursor();
	//Recrée les SndChannels
	for(i = 0; i < thePrefs.nbSamples; ++i) {
		SCD_CreateTrack(i);
		theCommand.cmd		= soundCmd;
		theCommand.param1	= 0;
		if(samples[i].fileID)
		theCommand.param2	= (long) *samples[i].snd;
		else
		theCommand.param2	= (long) *noSound;
		SndDoImmediate(samples[i].chan, &theCommand);
	}
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *noSound;
	for(i = 0; i < maxNbInstruments; ++i)
		if(instruments[i].fileID)
		theCommand.param2	= (long) *instruments[i].snd;
	for(i = 0; i < thePrefs.nbInsChannels; ++i) {
		theError = SndNewChannel(&insChannels[i], sampledSynth, initStereo, nil);
		if(theError) {
			Do_Error(theError, 504);
			return false;
		}
		SndDoImmediate(insChannels[i], &theCommand);
	}
	SCD_SetSynthetizerMode(thePrefs.synthMode);
	temp = -1;
	for(i = 0; i < kPatternParts; ++i)
		if(rythms[i].fileID)
		temp = i;
	for(i = 0; i < thePrefs.nbRytChannels; ++i) {
		theError = SndNewChannel(&rytChannels[i], sampledSynth, initStereo, nil);
		if(theError) {
			Do_Error(theError, 504);
			return false;
		}
		theCommand.cmd		= reInitCmd;
		theCommand.param1	= 0;
		theCommand.param2	= initStereo;
		SndDoImmediate(rytChannels[i], &theCommand);
		theCommand.cmd		= soundCmd;
		theCommand.param1	= 0;
		if(temp == -1)
		theCommand.param2	= (long) *noSound;
		else
		theCommand.param2	= (long) *rythms[temp].snd;
		SndDoImmediate(rytChannels[i], &theCommand);
	}
	
	SetGWorld((CGrafPtr) mainWin, nil);
	InvalRect(&shellRect);
	if(thePrefs.instrumentsWin) {
		SetGWorld((CGrafPtr) instrumentsWin, nil);
		InvalRect(&instrumentsRect);
	}
	if(thePrefs.rythmsWin) {
		SetGWorld((CGrafPtr) rythmsWin, nil);
		InvalRect(&rythmsRect);
	}
	SetCCursor(mouseCursor);
	
	return true;
#endif
}