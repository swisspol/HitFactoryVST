#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

//CONSTANTES LOCALES:

#define				kProgress			600

//STRUCTURES LOCALES:

/*typedef struct SoundComponentLink {
	ComponentDescription	description;
	SoundSource				mixerID;
	SoundSource*			linkID;
};

enum {
	kSoundEffectsType = 'snfx'
};

enum {
	kReverbSubType = 'revb',
	kSSpLocalizationSubType = 'snd3'
};*/

// FONCTIONS:

Fixed SCD_CleanUpSound(Handle aSound, Boolean forSampler)
{
	SoundHeaderPtr		stdHeader;
	ExtSoundHeaderPtr	extHeader;
	long				offset;
	/*Fixed				rate,
						result;*/
	
	HLock(aSound);
	
	GetSoundHeaderOffset((SndListHandle) aSound, &offset);
	BlockMove(*aSound + offset, *aSound, GetHandleSize(aSound) - offset);
	stdHeader = (SoundHeaderPtr) *aSound;
	extHeader = (ExtSoundHeaderPtr) *aSound;
	
	stdHeader->samplePtr		= nil;
	
	if(thePrefs.flags & kByPassHeaders) {
		if(forSampler) {
			//if(stdHeader->loopEnd == 0) {
				stdHeader->loopStart	= 0;
				if(stdHeader->encode == extSH)
				stdHeader->loopEnd		= extHeader->numFrames;
				else
				stdHeader->loopEnd		= stdHeader->length;
			//}
		}
		else {
			stdHeader->loopStart		= 0;
			stdHeader->loopEnd			= 0;
		}
		
		stdHeader->baseFrequency		= baseKey;
	}
	
	/*rate = stdHeader->sampleRate;
	if(rate > 0)
	result = FixDiv(rate, rate22khz);
	else {
		rate = rate & (~(1L<<31));
		result = FixDiv(rate, rate22khz) + FixDiv(1L<<30, rate22khz) + FixDiv(1L<<30, rate22khz);
	}*/
	
	HUnlock(aSound);
	SetHandleSize(aSound, GetHandleSize(aSound) - offset);
	
	return UnsignedFixedMulDiv(stdHeader->sampleRate, 0x00010000, rate22khz);
}

void SCD_StartSample(short num)
{
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *samples[num].snd;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	theCommand.cmd		= freqCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 1;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	theCommand.cmd		= rateCmd;
	theCommand.param1	= 0;
	theCommand.param2	= thePrefs.SRate[num] + thePrefs.SRelativeRate[num] * kProgress;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	thePrefs.SPlaying[num] = true;
}

void SCD_StopSample(short num)
{
	/*theCommand.cmd		= quietCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	SndDoImmediate(samples[num].chan, &theCommand);*/
	
	theCommand.cmd		= bufferCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *noSound;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	thePrefs.SPlaying[num] = false;
}

typedef OSErr (*SDDPI_Proc)(DTDPlugInParametersPtr);

void SCD_StartDirectToDisk(short num)
{
	SymClass			theClass;
	SDDPI_Proc			theProcPtr;
	DTDPlugInParameters	theParams;
	
	UseResFile(mainResFile);
	GetIndString(theString, 0, 27);
	theError = FindSymbol(DTDPlugIns[samples[num].totalSndNb].connID, theString, (char**) &theProcPtr, &theClass);
	if(theError) {
		Do_Error(theError, 105);
		return;
	}
	theParams.channel = samples[num].chan;
	theParams.fileID = samples[num].fileID;
	theParams.bufferHandle = samples[num].snd;
	theError = (*theProcPtr)(&theParams);
	samples[num].time = TickCount();
	if(theError)
	Do_Error(theError, 701);
	SCD_PitchSample(num);
	
	thePrefs.SPlaying[num] = true;
}

typedef OSErr (*KDDPI_Proc)(DTDPlugInParametersPtr);

void SCD_StopDirectToDisk(short num)
{
	SymClass			theClass;
	KDDPI_Proc			theProcPtr;
	DTDPlugInParameters	theParams;
	
	UseResFile(mainResFile);
	GetIndString(theString, 0, 30);
	theError = FindSymbol(DTDPlugIns[samples[num].totalSndNb].connID, theString, (char**) &theProcPtr, &theClass);
	if(theError) {
		Do_Error(theError, 105);
		return;
	}
	theParams.channel = samples[num].chan;
	theParams.fileID = samples[num].fileID;
	theParams.bufferHandle = samples[num].snd;
	theError = (*theProcPtr)(&theParams);
	
	theCommand.cmd		= bufferCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *noSound;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	thePrefs.SPlaying[num] = false;
	
	if(thePrefs.flags & kDTDTime) {
		SetGWorld((CGrafPtr) mainWin, nil);
		Draw_Number(num);
	}
	
}

void SCD_AlignSample(short num)
{
	SoundHeaderPtr		stdHeader;
	ExtSoundHeaderPtr	extHeader;
	long				numFrames,
						length,
						patternLength,
						offset,
						globalOffset,
						maxOffset;
	UnsignedFixed		frequency;
	unsigned short 	temp;
	short				times;
	
	//R�cup�re les donn�es
	stdHeader = (SoundHeaderPtr) *samples[num].snd;
	extHeader = (ExtSoundHeaderPtr) *samples[num].snd;
	if(stdHeader->encode == stdSH)
	numFrames = stdHeader->length;
	else
	numFrames = extHeader->numFrames;
	
	//Calcule les dur�es
	length = 1000 * numFrames / (unsigned short) HiWord(stdHeader->sampleRate);
	frequency = UnsignedFixedMulDiv(UnsignedFixedMulDiv(stdHeader->sampleRate, 0x00010000, rate22khz) 
		+ kMinRelativeRate * kProgress, rate22khz, 0x00010000);
	maxOffset = 1000 * numFrames / (unsigned short) HiWord(frequency) - length;
	patternLength = 240000 / thePrefs.RBPM;
	
	//Calcule les offsets
	globalOffset = length % patternLength;
	times = globalOffset / maxOffset;
	if(length < (patternLength / 2) || !globalOffset)
	return;
	
	//D�cale le sample
	if(globalOffset < (patternLength / 2)) { //Acc�l�rer
		do {
			++times;
			offset = globalOffset / times;
		
			frequency = 0L;
			temp = 1000 * numFrames / (length - offset);
			BlockMove(&temp, &frequency, 2);
			
			frequency = UnsignedFixedMulDiv(frequency, 0x00010000, rate22khz) 
				- UnsignedFixedMulDiv(stdHeader->sampleRate, 0x00010000, rate22khz);
			thePrefs.SRelativeRate[num] = frequency / kProgress;
		}
		while(thePrefs.SRelativeRate[num] > kMaxRelativeRate);
	}
	else { //Ralentir
		globalOffset = patternLength - globalOffset;
		do {
			++times;
			offset = globalOffset / times;
		
			frequency = 0L;
			temp = 1000 * numFrames / (length + offset);
			BlockMove(&temp, &frequency, 2);
		
			frequency = UnsignedFixedMulDiv(stdHeader->sampleRate, 0x00010000, rate22khz) 
				- UnsignedFixedMulDiv(frequency, 0x00010000, rate22khz);
			thePrefs.SRelativeRate[num] = - (frequency / kProgress);
		}
		while(thePrefs.SRelativeRate[num] < kMinRelativeRate);
	}
}

void SCD_PitchSample(short num)
{
	theCommand.cmd		= rateCmd;
	theCommand.param1	= 0;
	theCommand.param2	= thePrefs.SRate[num] + thePrefs.SRelativeRate[num] * kProgress;
	SndDoImmediate(samples[num].chan, &theCommand);
}

void SCD_VolumeSample(short num)
{
	if(thePrefs.SPan[num] > 100)
	tempVol = Shorts_To_Long(thePrefs.SVol[num] * (200 - thePrefs.SPan[num]) / 100,
		thePrefs.SVol[num]);
	else
	tempVol = Shorts_To_Long(thePrefs.SVol[num],
		thePrefs.SVol[num] * (thePrefs.SPan[num]) / 100);
	theCommand.cmd		= volumeCmd;
	theCommand.param1	= 0;
	theCommand.param2	= tempVol;
	SndDoImmediate(samples[num].chan, &theCommand);
}

void SCD_EffectsSample(short num)
{
#if demo
	return;
#else
	thePrefs.localization.sourceMode = thePrefs.SSourceMode[num];
	SndSetInfo(samples[num].chan, siSSpLocalization, &thePrefs.localization);
#endif
}
	
void SCD_VolumeSynthetizer()
{
	long		j;
	
	tempVol = Shorts_To_Long(thePrefs.IVol, thePrefs.IVol);
	theCommand.cmd		= volumeCmd;
	theCommand.param1	= 0;
	theCommand.param2	= tempVol;
	for(j = 0; j < thePrefs.nbInsChannels; ++j)
	SndDoImmediate(insChannels[j], &theCommand);
}

void SCD_VolumeRythmsBox()
{
	long		j;
	
	tempVol = Shorts_To_Long(thePrefs.RVol, thePrefs.RVol);
	theCommand.cmd		= volumeCmd;
	theCommand.param1	= 0;
	theCommand.param2	= tempVol;
	for(j = 0; j < thePrefs.nbRytChannels; ++j)
	SndDoImmediate(rytChannels[j], &theCommand);
}

void SCD_SetSynthetizerMode(short mode)
{
	theCommand.cmd		= reInitCmd;
	theCommand.param1	= 0;
	if(mode == 1) { //mono
		theCommand.param2	= initStereo;
		for(i = 0; i < thePrefs.nbInsChannels; ++i)
		SndDoImmediate(insChannels[i], &theCommand);
	}
	else { //st�r�o
		theCommand.param2	= initChanLeft;
		for(i = 0; i < thePrefs.nbInsChannels; i = i + 2)
		SndDoImmediate(insChannels[i], &theCommand);
		theCommand.param2	= initChanRight;
		for(i = 1; i < thePrefs.nbInsChannels; i = i + 2)
		SndDoImmediate(insChannels[i], &theCommand);
	}
}
	
void SCD_CreateTrack(short num)
{
	SoundComponentLink	theLink;
	
	/*switch(thePrefs.SStatus[num]) {
		
		case initMono:
		theError = SndNewChannel(&samples[num].chan, sampledSynth, initMono, nil);
		break;
		
		case initChanLeft:
		theError = SndNewChannel(&samples[num].chan, sampledSynth, initChanLeft, nil);
		break;
		
		case initChanRight:
		theError = SndNewChannel(&samples[num].chan, sampledSynth, initChanRight, nil);
		break;
		
		case initStereo:
		theError = SndNewChannel(&samples[num].chan, sampledSynth, initStereo, nil);
		break;
		
	}*/
	theError = SndNewChannel(&samples[num].chan, sampledSynth, thePrefs.SStatus[num], nil);
	if(theError)
	Do_Error(theError, 504);
	if(samples[num].chan)
	SCD_VolumeSample(num);
	samples[num].chan->userInfo = SetCurrentA5();
	
#if demo
	return;
#else
	#if defined(powerc) || defined (__powerc)
	if(thePrefs.SReverb && samples[num].chan) {
		theLink.description.componentType			= kSoundEffectsType;
		theLink.description.componentSubType		= kSSpLocalizationSubType;
		theLink.description.componentManufacturer	= 0;
		theLink.description.componentFlags			= 0;
		theLink.description.componentFlagsMask		= 0;
		theLink.mixerID								= nil;
		theLink.linkID								= nil;
		theError = SndSetInfo(samples[num].chan, siPreMixerSoundComponent, &theLink);
		if(theError)
		Do_Error(theError, 507);
		SCD_EffectsSample(num);
	}
	#endif
#endif
}
	
void SCD_KillTrack(short num)
{
	SndDisposeChannel(samples[num].chan, true);
	chanFinished = nil;
	
	SF_CloseSample(num);
	
	thePrefs.SStatus[num]		= 0;
	thePrefs.SSndNb[num]		= 0;
	thePrefs.SVol[num]			= defaultMaxVolume;
	thePrefs.SLoop[num]			= 0;
	thePrefs.SPan[num]			= 100;
	thePrefs.SRate[num]			= 0;
	thePrefs.SRelativeRate[num]	= 0;
	thePrefs.SPlaying[num]		= false;
	
	thePrefs.SMaxVol[num]		= defaultMaxVolume;
	//Tuer  la r�verb?
	
	num = uc[num];
	if(num != -1) {
		SetGWorld((CGrafPtr) mainWin, nil);
		SetRect(&aRect, 8 + num * 98, 0, 98 + num * 98, 200);
		InvalRect(&aRect);
	}
}

void SCD_ReInitTrack(short num, long newInit)
{
	theCommand.cmd		= reInitCmd;
	theCommand.param1	= 0;
	theCommand.param2	= newInit;
	SndDoImmediate(samples[num].chan, &theCommand);
	thePrefs.SStatus[num] = newInit;
	
	//if(thePrefs.SPlaying[num] && samples[num].type == sampleType)
	//SCD_StartSample(num);
}

void SCD_PlayNote(short num, unsigned char theNote)
{
	++activeChan;
	if(activeChan > thePrefs.nbInsChannels - 1)
	activeChan = 0;
	
	/*SndChannelStatus(insChannels[activeChan], sizeof(channelStatus), &channelStatus);
	if(channelStatus.scChannelBusy) {*/
		theCommand.cmd		= quietCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 0;
		SndDoImmediate(insChannels[activeChan], &theCommand);
	//}
	
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *instruments[num].snd;
	SndDoImmediate(insChannels[activeChan], &theCommand);
	
	theCommand.cmd		= freqCmd;
	theCommand.param1	= 0;
	theCommand.param2	= theNote;
	SndDoImmediate(insChannels[activeChan], &theCommand);
	
	currentNotes[activeChan][0] = num;
	currentNotes[activeChan][1] = theNote;
}

void SCD_StopNote(short num, unsigned char theNote)
{
	short	chan,
			k;
	
	for(chan = 0; chan < thePrefs.nbInsChannels; ++chan)
	if(currentNotes[chan][0] == num && currentNotes[chan][1] == theNote) {
#if 0
		theCommand.cmd		= quietCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 0;
		SndDoImmediate(insChannels[chan], &theCommand);
#else
		theCommand.cmd		= volumeCmd;
		theCommand.param1	= 0;
		for(k = thePrefs.IVol; k > 10; k = k / 4) {
			theCommand.param2	= Shorts_To_Long(thePrefs.IVol, thePrefs.IVol);
			SndDoImmediate(insChannels[chan], &theCommand);
		}
		
		theCommand.cmd		= quietCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 0;
		SndDoImmediate(insChannels[chan], &theCommand);
		
		theCommand.cmd		= volumeCmd;
		theCommand.param1	= 0;
		theCommand.param2	= Shorts_To_Long(thePrefs.IVol, thePrefs.IVol);
		SndDoImmediate(insChannels[chan], &theCommand);
#endif
	}
}

typedef OSErr (*PDDPI_Proc)(DTDPlugInParametersPtr);
typedef OSErr (*RDDPI_Proc)(DTDPlugInParametersPtr);

void SCD_PauseResumeSampler()
{
	SymClass			theClass;
	PDDPI_Proc			theProcPtr;
	RDDPI_Proc			theProcPtr_2;
	DTDPlugInParameters	theParams;
	
	UseResFile(mainResFile);
	if(!samplerPaused) {
		for(i = 0; i < thePrefs.nbSamples; ++i) {
			if(thePrefs.SPlaying[i]) {
				if(samples[i].type == sampleType) {
					theCommand.cmd		= rateCmd;
					theCommand.param1	= 0;
					theCommand.param2	= 0;
					SndDoImmediate(samples[i].chan, &theCommand);
				}
				else {
					GetIndString(theString, 0, 28);
					theError = FindSymbol(DTDPlugIns[samples[i].totalSndNb].connID, theString, (char**) &theProcPtr, &theClass);
					if(theError) {
						Do_Error(theError, 105);
						continue;
					}
					theParams.channel = samples[i].chan;
					theParams.fileID = samples[i].fileID;
					theParams.bufferHandle = samples[i].snd;
					theError = (*theProcPtr)(&theParams);
				}
			}
		}
		GetIndString(theString, 0, 13);
		SetItem(menu[2], 2, theString);
	}
	else {
		for(i = 0; i < thePrefs.nbSamples; ++i) {
			if(thePrefs.SPlaying[i]) {
				if(samples[i].type == sampleType) {
					theCommand.cmd		= rateCmd;
					theCommand.param1	= 0;
					theCommand.param2	= thePrefs.SRate[i] + thePrefs.SRelativeRate[i] * kProgress;
					SndDoImmediate(samples[i].chan, &theCommand);
				}
				else {
					GetIndString(theString, 0, 29);
					theError = FindSymbol(DTDPlugIns[samples[i].totalSndNb].connID, theString, (char**) &theProcPtr_2, &theClass);
					if(theError) {
						Do_Error(theError, 105);
						continue;
					}
					theParams.channel = samples[i].chan;
					theParams.fileID = samples[i].fileID;
					theParams.bufferHandle = samples[i].snd;
					theError = (*theProcPtr_2)(&theParams);
				}
			}
		}
		GetIndString(theString, 0, 12);
		SetItem(menu[2], 2, theString);
	}
	samplerPaused = !samplerPaused;
}