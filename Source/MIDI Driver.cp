#include			<MIDI.h>
#include			<OMS.h>

#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"
#include			"Record Codes.h"

//CONSTANTES LOCALES:

#define				kHitFactoryClientID	'HiFc'
#define				kInputPortID		'Inpt'
#define				kOutputPortID		'Oupt'
#define				kMIDIBufferSize		5000
#define				kMIDIVConnectMade	-255
#define				kManagerClientID	'amdr'
#define				kManagerAInID		'Ain '
#define				kManagerAOutID		'Aout'
#define				kManagerBInID		'Bin '
#define				kManagerBOutID		'Bout'

#define				kStatusBit			128
#define				kDataBit			0
#define				kChannelMask		15
#define				kCommandMask		112

#define				kNoteOn				16
#define				kNoteOff			0
#define				kPrgmChange			64
#define				kCtrlChange			48

#define				kClock				248
#define				kStart				250
#define				kContinue			251
#define				kStop				252

#define				kTwoPower32			(4294967296.0)      /* 2^32 */
#define				kProgress			600
#define				kMax				60

//VARIABLES LOCALES:

Boolean			isForeGround = true;
short				MIDIInputRefNum,
					MIDIOutputRefNum,
					OMSDestNode;
OMSUniqueID			OMSUniqueSourceID;
UnsignedWide		clockTime,
					oldClockTime;
GrafPtr				oldPort_2;

//VARIABLES EXTERNES:

extern SndCommand	theCommand_2;
extern ExtTMTask	quantizeTask;
extern UniversalProcPtr	QuantizeTaskRoutine;
		
//FONCTIONS:

void AlignSample2(short num, long patternLength)
{
	SoundHeaderPtr		stdHeader;
	ExtSoundHeaderPtr	extHeader;
	long				numFrames,
						length,
						offset,
						globalOffset,
						maxOffset;
	UnsignedFixed		frequency;
	unsigned short 	temp;
	short				times;
	
	//Récupère les données
	stdHeader = (SoundHeaderPtr) *samples[num].snd;
	extHeader = (ExtSoundHeaderPtr) *samples[num].snd;
	if(stdHeader->encode == stdSH)
	numFrames = stdHeader->length;
	else
	numFrames = extHeader->numFrames;
	
	//Calcule les durées
	length = 1000 * numFrames / (unsigned short) HiWord(stdHeader->sampleRate);
	frequency = UnsignedFixedMulDiv(UnsignedFixedMulDiv(stdHeader->sampleRate, 0x00010000, rate22khz) 
		- kMax * kProgress, rate22khz, 0x00010000);
	maxOffset = 1000 * numFrames / (unsigned short) HiWord(frequency) - length;
	
	//Calcule les offsets
	globalOffset = length % patternLength;
	times = globalOffset / maxOffset;
	if(length < (patternLength / 2) || !globalOffset)
	return;
	
	//Décale le sample
	if(globalOffset < (patternLength / 2)) { //Accélérer
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
		while(thePrefs.SRelativeRate[num] > kMax);
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
		while(thePrefs.SRelativeRate[num] < - kMax);
	}
}

double MicrosecondToDouble(register const UnsignedWide *epochPtr)
{
   register double    result;

   result = (((double) epochPtr->hi) * kTwoPower32) + epochPtr->lo;
   return (result);
}

void RB_Fonction()
{
	short			snd;
	long			partVol,
					volume;
					
	++patternPos;
	if(patternPos > kPatternLength - 1) {
		patternPos = 0;
		thePrefs.RCurrent = thePrefs.RNext;
		updatePattern = true;
		
		PrimeTime((QElemPtr) &quantizeTask, 0);
	}
	
	for(snd = 0; snd < kPatternParts; ++snd)
	if(pattern[thePrefs.RCurrent][snd][patternPos] && !thePrefs.RPartMute[snd])
	if(rythms[snd].fileID) {
		++activeRythmChan;
		if(activeRythmChan > thePrefs.nbRytChannels - 1)
		activeRythmChan = 0;

		theCommand_2.cmd		= quietCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= 0;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
		
		partVol = thePrefs.RVol * thePrefs.RPartVol[snd] / 200;
		if(thePrefs.RPartPan[snd] > 100)
		volume = Shorts_To_Long(partVol * (200 - thePrefs.RPartPan[snd]) / 100, partVol);
		else
		volume = Shorts_To_Long(partVol, partVol * (thePrefs.RPartPan[snd]) / 100);
		theCommand_2.cmd		= volumeCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= volume;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
	
		theCommand_2.cmd		= soundCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= (long) *rythms[snd].snd;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
		
		theCommand_2.cmd		= freqCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= thePrefs.RPartPitch[snd];
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
	}
	else
	if(MIDIDriverEnabled) {
		if(snd > 1)
		MIDI_SendNote(9, thePrefs.RPartPitch[snd - 1], 0); //Note Off
		else
		MIDI_SendNote(9, thePrefs.RPartPitch[kPatternParts - 1], 0); //Note Off
		
		MIDI_SendNote(9, thePrefs.RPartPitch[snd], thePrefs.RVol * thePrefs.RPartVol[snd] / 810);
	}
}

pascal short MIDI_ReadHook(MIDIPacketPtr thePacket, long appA5)
{
#if kA5savy
	long			oldA5 = SetA5(appA5);
#endif
	unsigned char	channel,
					command,
					velocity;
	short			pitch,
					pos;
	GrafPtr			oldPort_2;
	Rect			updateRect;
	long			j,
					patLength;
	
	if(!isForeGround && (thePrefs.MIDIFlags & kMIDIForeGround))
	goto End;
	if(thePacket->flags && thePacket->flags != 1)
	goto End;
	
	if(!(thePrefs.MIDIFlags & kUseHFClock)) {
		switch(thePacket->data[0]) {
			
			case kClock:
			++clockCount;
			if(clockCount == 6) {
				if(thePrefs.RPlaying)
				RB_Fonction();
				clockCount = 0;
				
				Microseconds(&clockTime);
				patLength = 16 * (MicrosecondToDouble(&clockTime) - MicrosecondToDouble(&oldClockTime)) / 1000;
				oldClockTime = clockTime;
				
				if(thePrefs.quantizeFlags & kQSmplPitchAlign)
					for(long k = 0; k < thePrefs.nbSamples; ++k)
					if(samples[k].fileID && samples[k].type == sampleType) {
						pitch = thePrefs.SRelativeRate[k];
						AlignSample2(k, patLength);
						if(thePrefs.SPlaying[k] && pitch != thePrefs.SRelativeRate[k])
						SCD_PitchSample(k);
					}
			}
			goto End;
			break;
			
			case kStart:
			patternPos = -1;
			clockCount = 5;
			RB_Start();
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRPlay;
				tempRec[position].command = 0;
				tempRec[position].command_2 = 0;
			}
			
			if(thePrefs.rythmsWin) {
				GetPort(&oldPort_2);
				SetGWorld((CGrafPtr) rythmsWin, nil);
				InvalRect(&rytPlayRect);
				SetGWorld((CGrafPtr) oldPort_2, nil);
			}
			goto End;
			break;
			
			case kContinue:
			RB_Start();
			clockCount = 5;
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRPlay;
				tempRec[position].command = 0;
				tempRec[position].command_2 = 0;
			}
			
			if(thePrefs.rythmsWin) {
				GetPort(&oldPort_2);
				SetGWorld((CGrafPtr) rythmsWin, nil);
				InvalRect(&rytPlayRect);
				SetGWorld((CGrafPtr) oldPort_2, nil);
			}
			goto End;
			break;
			
			case kStop:
			RB_Stop();
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRStop;
				tempRec[position].command = 0;
				tempRec[position].command_2 = 0;
			}
			
			if(thePrefs.rythmsWin) {
				GetPort(&oldPort_2);
				SetGWorld((CGrafPtr) rythmsWin, nil);
				InvalRect(&rytPlayRect);
				SetGWorld((CGrafPtr) oldPort_2, nil);
			}
			goto End;
			break;
			
		}
	}

	channel = thePacket->data[0] & kChannelMask;
	command = thePacket->data[0] & kCommandMask;
	pitch = thePacket->data[1];
	velocity = thePacket->data[2];
	
	if(command == kNoteOn && !velocity)
	command = kNoteOff;
	
	if(command == kNoteOn) {
		if(thePrefs.MIDIDriverMode == kMIDIAllToSmpl 
			|| thePrefs.MIDIDriverMode == kMIDIAllToSmplChan
			|| (thePrefs.MIDIDriverMode == kMIDISmplAndSynth && channel == thePrefs.MIDISamplerChannel)) {
			
			if(thePrefs.MIDIDriverMode == kMIDIAllToSmpl 
				|| thePrefs.MIDIDriverMode == kMIDISmplAndSynth) {
				channel = pitch - (pitch / thePrefs.nbSamples) * thePrefs.nbSamples;
				if(!samples[channel].fileID)
				goto End;
				pitch = thePrefs.SRelativeRate[channel];
			}
			if(thePrefs.MIDIDriverMode == kMIDIAllToSmplChan) {
				if(!samples[channel].fileID)
				goto End;
				pitch = pitch - baseKey;
				if(pitch > kMaxRelativeRate)
				pitch = kMaxRelativeRate;
				if(pitch < kMinRelativeRate)
				pitch = kMinRelativeRate;
			}
			
			if(samples[channel].type != sampleType)
			goto End;
			
			if(!thePrefs.SPlaying[channel]) {
				if(pitch != thePrefs.SRelativeRate[channel]) {
					if(recording) {
						++position;
						tempRec[position].time = ActualTime;
						tempRec[position].action = kSPitch;
						tempRec[position].command = channel;
						tempRec[position].command_2 = pitch;
					}
					
					thePrefs.SRelativeRate[channel] = pitch;
				}
				
				if(thePrefs.quantizeFlags & kQSmplStart) {
					for(j = 0; j < nbSamplesToStart; ++j)
					if(trueButton == samplesToStart[j])
					goto End;
					for(j = 0; j < nbSamplesToStop; ++j)
					if(trueButton == samplesToStop[j])
					goto End;
					
					samplesToStart[nbSamplesToStart] = channel;
					++nbSamplesToStart;
				}
				else {
					if(recording) {
						++position;
						tempRec[position].time = ActualTime + 1;
						tempRec[position].action = kSStart;
						tempRec[position].command = channel;
						tempRec[position].command_2 = 0;
					}
					
					SCD_StartSample(channel);
				}
			}
			else {
				if(pitch == thePrefs.SRelativeRate[channel]) {
					if(thePrefs.quantizeFlags & kQSmplStop) {
						for(j = 0; j < nbSamplesToStart; ++j)
						if(trueButton == samplesToStart[j])
						goto End;
						for(j = 0; j < nbSamplesToStop; ++j)
						if(trueButton == samplesToStop[j])
						goto End;
						
						samplesToStop[nbSamplesToStop] = channel;
						++nbSamplesToStop;
					}
					else {
						if(recording) {
							++position;
							tempRec[position].time = ActualTime;
							tempRec[position].action = kSStop;
							tempRec[position].command = channel;
							tempRec[position].command_2 = 0;
						}
						
						SCD_StopSample(channel);
					}
				}
				else {
					if(recording) {
						++position;
						tempRec[position].time = ActualTime;
						tempRec[position].action = kSPitch;
						tempRec[position].command = channel;
						tempRec[position].command_2 = pitch;
					}
					
					thePrefs.SRelativeRate[channel] = pitch;
					SCD_PitchSample(channel);
				}
			}
			
			if(uc[channel] != -1) {
				GetPort(&oldPort_2);
				SetGWorld((CGrafPtr) mainWin, nil);
				SetRect(&updateRect, uc[channel] * 80, 213, 80 + uc[channel] * 80, 268);
				InvalRect(&updateRect);
				SetGWorld((CGrafPtr) oldPort_2, nil);
			}
		}
			
		channel = thePacket->data[0] & kChannelMask;
		pitch = thePacket->data[1];
		if(thePrefs.MIDIDriverMode == kMIDIAllToSynth 
			|| thePrefs.MIDIDriverMode == kMIDIAllToSynthChan 
			|| (thePrefs.MIDIDriverMode == kMIDISmplAndSynth && channel == thePrefs.MIDISynthChannel)) {
			
			if(thePrefs.MIDIDriverMode == kMIDIAllToSynth)
			channel = activeInstrument;
			if(thePrefs.MIDIDriverMode == kMIDIAllToSynthChan)
			;
			if(thePrefs.MIDIDriverMode == kMIDISmplAndSynth)
			channel = activeInstrument;
			
			if(instruments[channel].fileID) {
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kIPlay;
					tempRec[position].command = channel;
					tempRec[position].command_2 = pitch;
				}
				
				SCD_PlayNote(channel, pitch);
			}
		}
		
		channel = thePacket->data[0] & kChannelMask;
		pitch = thePacket->data[1];
		if(thePrefs.MIDIDriverMode == kMIDIAllToRBox
			|| (thePrefs.MIDIDriverMode == kMIDISmplAndSynth && channel == thePrefs.MIDIRBChannel)) {
			pitch = pitch - (pitch / 10) * 10;
			
			if(rythms[pitch].fileID) {
				if(!thePrefs.RPlaying) {
					if(recording) {
						++position;
						tempRec[position].time = ActualTime;
						tempRec[position].action = kRInstrument;
						tempRec[position].command = pitch;
						tempRec[position].command_2 = 0;
					}
					RB_PlaySound(pitch);
				}
				else {
					if(!playing && !recording) {
						pos = patternPos;
						pattern[thePrefs.RCurrent][pitch][pos] = 255;
						if(thePrefs.rytWinExt) {
							GetPort(&oldPort_2);
							SetGWorld((CGrafPtr) rythmsWin, nil);
							InvalRect(&rytPatternRect);
							SetGWorld((CGrafPtr) oldPort_2, nil);
						}
					}
					else if(recording) {
						++position;
						tempRec[position].time = ActualTime;
						tempRec[position].action = kRInstrument;
						tempRec[position].command = pitch;
						tempRec[position].command_2 = 0;
					}
					RB_PlaySound(pitch);
				}
			}
		}
	}
	
	if(command == kNoteOff && (thePrefs.flags & kStopWhenKeyUp)) {
		channel = thePacket->data[0] & kChannelMask;
		pitch = thePacket->data[1];
		if(thePrefs.MIDIDriverMode == kMIDIAllToSynth 
			|| thePrefs.MIDIDriverMode == kMIDIAllToSynthChan 
			|| (thePrefs.MIDIDriverMode == kMIDISmplAndSynth && channel == thePrefs.MIDISynthChannel)) {
			
			if(thePrefs.MIDIDriverMode == kMIDIAllToSynth)
			channel = activeInstrument;
			if(thePrefs.MIDIDriverMode == kMIDIAllToSynthChan)
			;
			if(thePrefs.MIDIDriverMode == kMIDISmplAndSynth)
			channel = activeInstrument;
			
			if(instruments[channel].fileID) {
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kIStop;
					tempRec[position].command = channel;
					tempRec[position].command_2 = pitch;
				}
				
				SCD_StopNote(channel, pitch);
			}
		}
	}
	
	/*if(command == kPrgmChange) {
		;
	}
	
	if(command == kCtrlChange) {
		;
	}*/
	
	End:
#if kA5savy
	oldA5 = SetA5(oldA5);
#endif	
	return midiMorePacket;
}

pascal short AMM_ReadHook(MIDIPacketPtr thePacketPtr, long appA5)
{
	MIDI_ReadHook(thePacketPtr, appA5);
	
	return midiMorePacket;
}
UniversalProcPtr AMMReadHookRoutine = NewMIDIReadHookProc(AMM_ReadHook);

pascal void OMS_ReadHook(OMSPacket* thePacketPtr, long appA5)
{
	MIDI_ReadHook((MIDIPacketPtr) thePacketPtr, appA5);
}
UniversalProcPtr OMSReadHookRoutine = NewOMSReadHook(OMS_ReadHook);

pascal void OMS_ApplicationHook(OMSAppHookMsg* message, long appA5)
{
	;
}
UniversalProcPtr OMSApplicationHookRoutine = NewOMSAppHook(OMS_ApplicationHook);

Boolean MIDI_SendNote(char channel, char note, char velocity)
{
	MIDIPacket		theMIDIPacket;
	OMSPacket		theOMSPacket;
	
	//if(!MIDIDriverEnabled)
	//return false;
	
	if(thePrefs.MIDIFlags & kUseMidiMgr) {
		theMIDIPacket.flags = 0;
		theMIDIPacket.len = 6 + 3;
		theMIDIPacket.tStamp = 0L;
		theMIDIPacket.data[0] = kStatusBit | kNoteOn | channel;
		theMIDIPacket.data[1] = kDataBit | note;
		theMIDIPacket.data[2] = kDataBit | velocity;
		
		MIDIWritePacket(MIDIOutputRefNum, &theMIDIPacket);
	}
	else {
		theOMSPacket.flags = 0;
		theOMSPacket.len = 6 + 3;
		theOMSPacket.srcIORefNum = 0;
		theOMSPacket.appConnRefCon = 0;
		theOMSPacket.data[0] = kStatusBit | kNoteOn | channel;
		theOMSPacket.data[1] = kDataBit | note;
		theOMSPacket.data[2] = kDataBit | velocity;
	
		OMSWritePacket(&theOMSPacket, OMSDestNode, MIDIOutputRefNum);
	}
	
	return true;
}
	
Boolean AMM_InstallDriver()
{
	Handle			icon;
	MIDIPortParams	params;
	
	if(!MIDIVersion()) { //MIDI Manager?
		Do_Error(0, 604);
		return false;
	}
	
	UseResFile(mainResFile); //SignIn
	icon = GetIcon(500);
	GetIndString(theString, 600, 1);
	theError = MIDISignIn(kHitFactoryClientID, (long) SetCurrentA5(), icon, theString);
	if(theError) {
		Do_Error(theError, 605);
		return false;
	}
	
	params.portID = kInputPortID; //Input Port
	params.portType = midiPortTypeInput;
	params.timeBase = 0;
	params.offsetTime = midiGetCurrent;
	params.readHook = AMMReadHookRoutine;
	params.refCon = (long) SetCurrentA5();
	params.initClock.syncType = 0;
	params.initClock.curTime = 0;
	params.initClock.format = 0;
	GetIndString(params.name, 600, 2);
	theError = MIDIAddPort(kHitFactoryClientID, kMIDIBufferSize, &MIDIInputRefNum, &params);
	if(theError && theError != kMIDIVConnectMade) {
		Do_Error(theError, 606);
		MIDISignOut(kHitFactoryClientID);
		return false;
	}
	
	params.portID = kOutputPortID; //Output Port
	params.portType = midiPortTypeOutput;
	params.timeBase = 0;
	params.offsetTime = midiGetCurrent;
	params.readHook = nil;
	params.refCon = (long) SetCurrentA5();
	params.initClock.syncType = 0;
	params.initClock.curTime = 0;
	params.initClock.format = 0;
	GetIndString(params.name, 600, 3);
	theError = MIDIAddPort(kHitFactoryClientID, kMIDIBufferSize, &MIDIOutputRefNum, &params);
	if(theError && theError != kMIDIVConnectMade) {
		Do_Error(theError, 607);
		MIDISignOut(kHitFactoryClientID);
		return false;
	}
	
	if(thePrefs.MIDIFlags & kMIDIAutoConnect) { //Autoconnection port modem
		MIDIConnectData(kHitFactoryClientID, kInputPortID, kManagerClientID, kManagerAInID);
		MIDIConnectData(kHitFactoryClientID, kOutputPortID, kManagerClientID, kManagerAOutID);
	}
	
	return true;
}

Boolean OMS_InstallDriver()
{
	//OMSNodeInfoListH		nodesListHandle;
	//Boolean				hasInput = false,
	//						hasOutput = false;
	OMSIDListH				nodesListHandle;
	short					mode;
	OMSConnectionParams		connectionParams;
	
	if(!(OMSVersion() > 0x01200000)) { //OMS?
		Do_Error(0, 608);
		return false;
	}
	
	UseResFile(mainResFile); //SignIn
	GetIndString(theString, 600, 1);
	theError = OMSSignIn(kHitFactoryClientID, (long) SetCurrentA5(), theString, OMSApplicationHookRoutine, &mode);
	if(theError) {
		Do_Error(theError, 609);
		return false;
	}
	
	//Input port
	theError = OMSAddPort(kHitFactoryClientID, kInputPortID, omsPortTypeInput, OMSReadHookRoutine, (long) SetCurrentA5(), &MIDIInputRefNum);
	if(theError) {
		Do_Error(theError, 606);
		OMSSignOut(kHitFactoryClientID);
		return false;
	}
	
	//Output port
	theError = OMSAddPort(kHitFactoryClientID, kOutputPortID, omsPortTypeOutput , nil, 0L, &MIDIOutputRefNum);
	if(theError) {
		Do_Error(theError, 607);
		OMSSignOut(kHitFactoryClientID);
		return false;
	}
	
	//Nodes
#if 0
	nodesListHandle = OMSGetNodeInfo(omsIncludeInputs + omsIncludeOutputs);
	if(!nodesListHandle || !(*nodesListHandle)->numNodes) {
		Do_Error(0, 610);
		OMSSignOut(kHitFactoryClientID);
		return false;
	}
	for(i = 0; i < (*nodesListHandle)->numNodes; ++i) {
		if(((*nodesListHandle)->info[i].flags & kIsOutput) && !hasOutput) {
			OMSDestNode = (*nodesListHandle)->info[i].ioRefNum;
			hasOutput = true;
		}
		if(((*nodesListHandle)->info[i].flags & !kIsOutput) && !hasInput) {
			OMSUniqueSourceID = (*nodesListHandle)->info[i].uniqueID;
			hasInput = true;
		}
	}
	if(!hasInput || !hasOutput) {
		Do_Error(0, 611);
		OMSSignOut(kHitFactoryClientID);
		return false;
	}
	OMSDisposeHandle(nodesListHandle);
#else
	GetIndString(theString, 600, 10);
	nodesListHandle = OMSChooseNodes(nil, theString, false, omsIncludeReal + omsIncludeInputs, nil);
	if(!nodesListHandle || !(*nodesListHandle)->numIDs) {
		OMSSignOut(kHitFactoryClientID);
		return false;
	}
	OMSUniqueSourceID = (*nodesListHandle)->id[0];
	OMSDisposeHandle(nodesListHandle);
	
	GetIndString(theString, 600, 11);
	nodesListHandle = OMSChooseNodes(nil, theString, false, omsIncludeReal + omsIncludeOutputs, nil);
	if(!nodesListHandle || !(*nodesListHandle)->numIDs) {
		OMSSignOut(kHitFactoryClientID);
		return false;
	}
	OMSDestNode = OMSUniqueIDToRefNum((*nodesListHandle)->id[0]);
	OMSDisposeHandle(nodesListHandle);
#endif
	
	//Connect to input:
	connectionParams.nodeUniqueID	= OMSUniqueSourceID;
	connectionParams.appRefCon		= 0;
	theError = OMSOpenConnections(kHitFactoryClientID, kInputPortID, 1, &connectionParams, false);
	if(theError) {
		Do_Error(theError, 612);
		OMSSignOut(kHitFactoryClientID);
		return false;
	}
	
	return true;
}

Boolean MIDI_InstallDriver()
{
	Boolean	result;
	
#if	demo
	return false;
#else
	if(thePrefs.MIDIFlags & kUseMidiMgr)
	result = AMM_InstallDriver();
	else
	result = OMS_InstallDriver();
	
	if(result) {
		MIDIDriverEnabled = true;
		return true;
	}
	else
	return false;
#endif
}

void AMM_KillDriver()
{
	MIDIRemovePort(MIDIInputRefNum); //Input Port
	MIDIRemovePort(MIDIOutputRefNum); //Output port
	MIDISignOut(kHitFactoryClientID); //MIDI Manager
}

void OMS_KillDriver()
{
#if 0
	OMSConnectionParams		connectionParams;
	
	//Deconnect from input:
	connectionParams.nodeUniqueID	= OMSUniqueSourceID;
	connectionParams.appRefCon		= 0;
	OMSCloseConnections(kHitFactoryClientID, kInputPortID, 1, &connectionParams);
#endif	
	OMSSignOut(kHitFactoryClientID); //OMS
}

Boolean MIDI_KillDriver()
{
#if demo
	return false;
#else
	if(!MIDIDriverEnabled)
	return false;
	
	if(thePrefs.MIDIFlags & kUseMidiMgr)
	AMM_KillDriver();
	else
	OMS_KillDriver();
	
	MIDIDriverEnabled = false;
	
	return true;
#endif
}

void MIDI_Suspend()
{
	if(!MIDIDriverEnabled)
	return;
	
	if(thePrefs.MIDIFlags & kUseMidiMgr)
	isForeGround = false;
	else
	OMSSuspend(kHitFactoryClientID);
}
	
void MIDI_Resume()
{
	if(!MIDIDriverEnabled)
	return;
	
	if(thePrefs.MIDIFlags & kUseMidiMgr)
	isForeGround = true;
	else
	OMSResume(kHitFactoryClientID);
}

void Init_MIDIDialog()
{
	Str255		tempString;
	long		tempValue;
	
#if demo
	return;
#else
	//Récupère le dialogue
	UseResFile(mainResFile);
	theDialog = GetNewDialog(3500, nil, (WindowPtr) -1);
	GetDItem(theDialog, 3, nil, &dialogItem[0], &aRect);
	GetDItem(theDialog, 4, nil, &dialogItem[1], &aRect);
	GetDItem(theDialog, 5, nil, &dialogItem[2], &aRect);
	GetDItem(theDialog, 6, nil, &dialogItem[3], &aRect);
	GetDItem(theDialog, 7, nil, &dialogItem[4], &aRect);
	GetDItem(theDialog, 8, nil, &dialogItem[5], &aRect);
	GetDItem(theDialog, 9, nil, &dialogItem[6], &aRect);
	GetDItem(theDialog, 10, nil, &dialogItem[7], &aRect);
	GetDItem(theDialog, 11, nil, &dialogItem[8], &aRect);
	GetDItem(theDialog, 12, nil, &dialogItem[9], &aRect);
	GetDItem(theDialog, 1, nil, &dialogItem[10], &aRect);
	GetDItem(theDialog, 13, nil, &dialogItem[11], &aRect);
	GetDItem(theDialog, 14, nil, &dialogItem[12], &aRect);
	GetDItem(theDialog, 15, nil, &dialogItem[13], &aRect);
	GetDItem(theDialog, 16, nil, &dialogItem[14], &aRect);
	GetDItem(theDialog, 17, nil, &dialogItem[15], &aRect);
	
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	SetGWorld((CGrafPtr) savePort, nil);
	Hide_Menus();
	
	//MIDI on?
	if(!thePrefs.enableMIDIDriver || (thePrefs.enableMIDIDriver && !MIDIVersion() && !OMSVersion())) {
		HiliteControl((ControlHandle) dialogItem[1], 255);
		HiliteControl((ControlHandle) dialogItem[2], 255);
	}
	else {
		SetCtlValue((ControlHandle) dialogItem[0], true);
		if((thePrefs.MIDIFlags & kUseMidiMgr) && MIDIVersion())
		SetCtlValue((ControlHandle) dialogItem[1], true);
		if(!(thePrefs.MIDIFlags & kUseMidiMgr) && OMSVersion())
		SetCtlValue((ControlHandle) dialogItem[2], true);
		if(!MIDIVersion()) {
			SetCtlValue((ControlHandle) dialogItem[1], false);
			SetCtlValue((ControlHandle) dialogItem[2], true);
			HiliteControl((ControlHandle) dialogItem[1], 255);
		}
		if(!OMSVersion()) {
			SetCtlValue((ControlHandle) dialogItem[1], true);
			SetCtlValue((ControlHandle) dialogItem[2], false);
			HiliteControl((ControlHandle) dialogItem[2], 255);
		}
	}
	if(!MIDIVersion() && !OMSVersion()) {
		SetCtlValue((ControlHandle) dialogItem[0], false);
		HiliteControl((ControlHandle) dialogItem[0], 255);
	}
	SetCtlValue((ControlHandle) dialogItem[3], thePrefs.MIDIFlags & kMIDIForeGround); //FG only
	SetCtlValue((ControlHandle) dialogItem[4], thePrefs.MIDIFlags & kMIDIAutoConnect); //Autoconnect
#if pro
	SetCtlValue((ControlHandle) dialogItem[5], !(thePrefs.MIDIFlags & kUseHFClock)); //Extern clock
#else
	HiliteControl((ControlHandle) dialogItem[5], 255);
#endif
	if(thePrefs.MIDIDriverMode == kMIDIAllToSmpl //Smpl
		|| thePrefs.MIDIDriverMode == kMIDIAllToSmplChan) {
		SetCtlValue((ControlHandle) dialogItem[6], true);
		if(thePrefs.MIDIDriverMode == kMIDIAllToSmplChan)
		SetCtlValue((ControlHandle) dialogItem[7], true);
	}
	else
	HiliteControl((ControlHandle) dialogItem[7], 255);
	if(thePrefs.MIDIDriverMode == kMIDIAllToSynth //Syth
		|| thePrefs.MIDIDriverMode == kMIDIAllToSynthChan) {
		SetCtlValue((ControlHandle) dialogItem[8], true);
		if(thePrefs.MIDIDriverMode == kMIDIAllToSynthChan)
		SetCtlValue((ControlHandle) dialogItem[9], true);
	}
	else
	HiliteControl((ControlHandle) dialogItem[9], 255);
	if(thePrefs.MIDIDriverMode == kMIDIAllToRBox) //RB
	SetCtlValue((ControlHandle) dialogItem[11], true);
	if(thePrefs.MIDIDriverMode == kMIDISmplAndSynth) //All
	SetCtlValue((ControlHandle) dialogItem[12], true);
	tempValue = thePrefs.MIDISamplerChannel + 1; //Channel pour sampler
	NumToString(tempValue, tempString);
	SetIText(dialogItem[13], tempString);
	tempValue = thePrefs.MIDISynthChannel + 1; //Channel pour synth
	NumToString(tempValue, tempString);
	SetIText(dialogItem[14], tempString);
	tempValue = thePrefs.MIDIRBChannel + 1; //Channel pour RB
	NumToString(tempValue, tempString);
	SetIText(dialogItem[15], tempString);
	
	SelIText(theDialog, 15, 0, 32767); //Misc
	
	dialogRunning = 4;
#endif
}

void Update_MIDIDialog()
{
#if demo
	return;
#else
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	BeginUpdate(theDialog);
		OutLine_Item(theDialog, 27);
		OutLine_Item(theDialog, 29);
		OutLine_Item(theDialog, 31);
		Draw_DefaultItem(theDialog, 1);
		DrawDialog(theDialog);
	EndUpdate(theDialog);
	SetGWorld((CGrafPtr) savePort, nil);
#endif
}

void Act_MIDIDialog(short item)
{
#if demo
	return;	
#else
	if(item == 1) //OK
	Dispose_MIDIDialog();
	if(item == 2) { //Cancel
		DisposeDialog(theDialog);
		for(i = 0; i < 20; ++i)
			//if(dialogItem[i])
			DisposeHandle(dialogItem[i]);
		dialogRunning = 0;
		Show_Menus();
	}
	if(item == 3) { //On
		i = GetCtlValue((ControlHandle) dialogItem[0]);
		if(i) {
			SetCtlValue((ControlHandle) dialogItem[0], false);
			HiliteControl((ControlHandle) dialogItem[1], 255);
			HiliteControl((ControlHandle) dialogItem[2], 255);	
		}
		else {
			SetCtlValue((ControlHandle) dialogItem[0], true);
			if(MIDIVersion()) {
				HiliteControl((ControlHandle) dialogItem[1], 0);
				if(!OMSVersion())
				SetCtlValue((ControlHandle) dialogItem[1], true);
			}
			if(OMSVersion()) {
				HiliteControl((ControlHandle) dialogItem[2], 0);
				if(!MIDIVersion())
				SetCtlValue((ControlHandle) dialogItem[2], true);
			}
			if(MIDIVersion() && OMSVersion())
			SetCtlValue((ControlHandle) dialogItem[1], true);
		}
	}
	if(item == 4) { //AMM
		SetCtlValue((ControlHandle) dialogItem[1], true);
		SetCtlValue((ControlHandle) dialogItem[2], false);
	}
	if(item == 5) { //OMS
		SetCtlValue((ControlHandle) dialogItem[1], false);
		SetCtlValue((ControlHandle) dialogItem[2], true);
	}
	if(item == 6) { //FG only
		i = GetCtlValue((ControlHandle) dialogItem[3]);
		SetCtlValue((ControlHandle) dialogItem[3], !i);
	}
	if(item == 7) { //Autoconnect
		i = GetCtlValue((ControlHandle) dialogItem[4]);
		SetCtlValue((ControlHandle) dialogItem[4], !i);
	}
	if(item == 8) {
		i = GetCtlValue((ControlHandle) dialogItem[5]);
		SetCtlValue((ControlHandle) dialogItem[5], !i);
	}
	if(item == 9) { //Smpl
		SetCtlValue((ControlHandle) dialogItem[6], true);
		HiliteControl((ControlHandle) dialogItem[7], 0);
		SetCtlValue((ControlHandle) dialogItem[8], false);
		HiliteControl((ControlHandle) dialogItem[9], 255);
		SetCtlValue((ControlHandle) dialogItem[11], false);
		SetCtlValue((ControlHandle) dialogItem[12], false);
	}
	if(item == 10) { //Smpl chan
		i = GetCtlValue((ControlHandle) dialogItem[7]);
		SetCtlValue((ControlHandle) dialogItem[7], !i);
	}
	if(item == 11) { //Syth
		SetCtlValue((ControlHandle) dialogItem[6], false);
		HiliteControl((ControlHandle) dialogItem[7], 255);
		SetCtlValue((ControlHandle) dialogItem[8], true);
		HiliteControl((ControlHandle) dialogItem[9], 0);
		SetCtlValue((ControlHandle) dialogItem[11], false);
		SetCtlValue((ControlHandle) dialogItem[12], false);
	}
	if(item == 12) { //Syth chan
		i = GetCtlValue((ControlHandle) dialogItem[9]);
		SetCtlValue((ControlHandle) dialogItem[9], !i);
	}
	if(item == 13) { //RB
		SetCtlValue((ControlHandle) dialogItem[6], false);
		HiliteControl((ControlHandle) dialogItem[7], 255);
		SetCtlValue((ControlHandle) dialogItem[8], false);
		HiliteControl((ControlHandle) dialogItem[9], 255);
		SetCtlValue((ControlHandle) dialogItem[11], true);
		SetCtlValue((ControlHandle) dialogItem[12], false);
	}
	if(item == 14) { //Smpl + Syth + RB
		SetCtlValue((ControlHandle) dialogItem[6], false);
		HiliteControl((ControlHandle) dialogItem[7], 255);
		SetCtlValue((ControlHandle) dialogItem[8], false);
		HiliteControl((ControlHandle) dialogItem[9], 255);
		SetCtlValue((ControlHandle) dialogItem[11], false);
		SetCtlValue((ControlHandle) dialogItem[12], true);
	}
	if(item == 24)
	HMSetBalloons(!HMGetBalloons());
	if(item == -1 && (theKey == kReturnKey || theKey == kEnterKey)) { //Touche clavier
		HiliteControl((ControlHandle) dialogItem[10], inButton);
		Delay(kEnterDelay, &ticks);
		//Wait(kEnterDelay);
		Dispose_MIDIDialog();
	}
#endif
}

void Dispose_MIDIDialog()
{
	Str255		tempString;
	long		tempValue;
	
#if demo
	return;
#else
	thePrefs.enableMIDIDriver = GetCtlValue((ControlHandle) dialogItem[0]);
	MIDI_KillDriver();
	if(thePrefs.enableMIDIDriver) {
		if(GetCtlValue((ControlHandle) dialogItem[1]))
		thePrefs.MIDIFlags = thePrefs.MIDIFlags | kUseMidiMgr;
		else
		thePrefs.MIDIFlags = thePrefs.MIDIFlags & (~kUseMidiMgr);
		
		if(MIDI_InstallDriver())
		thePrefs.enableMIDIDriver = true;
		else
		thePrefs.enableMIDIDriver = false;
	}
	
	//Flags
	if(GetCtlValue((ControlHandle) dialogItem[3])) //FG only
	thePrefs.MIDIFlags = thePrefs.MIDIFlags | kMIDIForeGround;
	else
	thePrefs.MIDIFlags = thePrefs.MIDIFlags & (~kMIDIForeGround);
	if(GetCtlValue((ControlHandle) dialogItem[4])) //Autoconnect
	thePrefs.MIDIFlags = thePrefs.MIDIFlags | kMIDIAutoConnect;
	else
	thePrefs.MIDIFlags = thePrefs.MIDIFlags & (~kMIDIAutoConnect);
	if(GetCtlValue((ControlHandle) dialogItem[5])) //extern clock
	thePrefs.MIDIFlags = thePrefs.MIDIFlags & (~kUseHFClock);
	else
	thePrefs.MIDIFlags = thePrefs.MIDIFlags | kUseHFClock;
	
	//Settings
	if(GetCtlValue((ControlHandle) dialogItem[6])) {
		if(GetCtlValue((ControlHandle) dialogItem[7]))
		thePrefs.MIDIDriverMode = kMIDIAllToSmplChan;
		else
		thePrefs.MIDIDriverMode = kMIDIAllToSmpl;
	}
	if(GetCtlValue((ControlHandle) dialogItem[8])) {
		if(GetCtlValue((ControlHandle) dialogItem[9]))
		thePrefs.MIDIDriverMode = kMIDIAllToSynthChan;
		else
		thePrefs.MIDIDriverMode = kMIDIAllToSynth;
	}
	if(GetCtlValue((ControlHandle) dialogItem[11])) {
		thePrefs.MIDIDriverMode = kMIDIAllToRBox;
	}
	if(GetCtlValue((ControlHandle) dialogItem[12])) {
		thePrefs.MIDIDriverMode = kMIDISmplAndSynth;
		GetIText(dialogItem[13], tempString); //smpl
		StringToNum(tempString, &tempValue);
		if(tempValue < 1)
		tempValue = 1;
		if(tempValue > 16)
		tempValue = 16;
		thePrefs.MIDISamplerChannel = tempValue - 1;
		GetIText(dialogItem[14], tempString); //synth
		StringToNum(tempString, &tempValue);
		if(tempValue < 1)
		tempValue = 1;
		if(tempValue > 16)
		tempValue = 16;
		thePrefs.MIDISynthChannel = tempValue - 1;
		GetIText(dialogItem[15], tempString); //RB
		StringToNum(tempString, &tempValue);
		if(tempValue < 1)
		tempValue = 1;
		if(tempValue > 16)
		tempValue = 16;
		thePrefs.MIDIRBChannel = tempValue - 1;
	}
	
	SetGWorld((CGrafPtr) mainWin, nil);
	InvalRect(&shellRect);
	if(thePrefs.rythmsWin) {
		SetGWorld((CGrafPtr) rythmsWin, nil);
		InvalRect(&rythmsRect);
	}
	
	Update_Pref();
	
	DisposeDialog(theDialog);
	for(i = 0; i < 20; ++i)
		//if(dialogItem[i])
		DisposeHandle(dialogItem[i]);
	dialogRunning = 0;
	Show_Menus();
	
	if(!(thePrefs.MIDIFlags & kUseHFClock) && (thePrefs.quantizeFlags & kQSmplPitchAlign))
	DisableItem(menu[2], 3);
	else
	EnableItem(menu[2], 3);
#endif
}