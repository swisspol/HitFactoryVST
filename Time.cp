#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"
#include			"Record Codes.h"

//VARIABLES LOCALES:

short			k;

//CONSTANTES LOCALES:

#define				num			tempRec[position].command

//FONCTIONS:

pascal void TM_LoopTask()
{
	ExtTMTaskPtr	loopTaskPtr = (ExtTMTaskPtr) 0x2E89;
#if kA5savy
	long oldA5 = SetA5(loopTaskPtr->appA5);
#endif
	
	for(k = 0; k < thePrefs.nbSamples; ++k)
	if(thePrefs.SLoop[k] && thePrefs.SPlaying[k]) {
		thePrefs.SPan[k] = thePrefs.SPan[k] + thePrefs.SLoop[k];
		if(thePrefs.SPan[k] > 200 || thePrefs.SPan[k] < 0) {
			thePrefs.SLoop[k] = - thePrefs.SLoop[k];
			thePrefs.SPan[k] = thePrefs.SPan[k] + 2 * thePrefs.SLoop[k];
		}
		SCD_VolumeSample(k);
	}
		
	PrimeTime((QElemPtr) &loopTask, thePrefs.loopSpeed);
	
#if kA5savy
	oldA5 = SetA5(oldA5);
#endif
}

void TM_InstallLoopTask()
{
	loopTask.theTask.tmAddr		= LoopTaskRoutine;
	loopTask.theTask.tmWakeUp	= 0;
	loopTask.theTask.tmReserved	= 0;
	loopTask.appA5				= SetCurrentA5();
	
	InsXTime((QElemPtr) &loopTask);
	PrimeTime((QElemPtr) &loopTask, thePrefs.loopSpeed);
}
	
void TM_KillLoopTask()
{
	RmvTime((QElemPtr) &loopTask);
}

pascal void TM_ReadTask()
{
	ExtTMTaskPtr	readTaskPtr = (ExtTMTaskPtr) 0x2E89;
#if kA5savy
	long	oldA5 = SetA5(readTaskPtr->appA5);
#endif
	short	oldResFile = CurResFile();
	
	switch(tempRec[position].action) {
		
		case kSStart:
		//SCD_StartSample(tempRec[position].command);
		if(samples[num].type == sampleType)
		SCD_StartSample(num);
		else
		;//SCD_StartDirectToDisk(num);
		//thePrefs.SPlaying[num] = true;
		break;
		
		case kSStop:
		if(samples[tempRec[position].command].type == sampleType)
		SCD_StopSample(tempRec[position].command);
		else
		;//SCD_StopDirectToDisk(tempRec[position].command);
		//directToDiskFinished = 0;
		break;
		
		case kSNext:
		if(samples[num].totalSndNb == 1 || (thePrefs.flags & kNoInterruptBank))
		break;
		theCommand.cmd		= quietCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 0;
		SndDoImmediate(samples[num].chan, &theCommand);
		HUnlock(samples[num].snd);
		DisposeHandle(samples[num].snd);
		
		++samples[num].sndNb;
		if(samples[num].sndNb > samples[num].totalSndNb)
		samples[num].sndNb = 1;
		
		UseResFile(samples[num].fileID);
		samples[num].snd = Get1IndResource('snd ', samples[num].sndNb);
		
		GetResInfo(samples[num].snd, &resID, &resType, samples[num].name);
		DetachResource(samples[num].snd);
		thePrefs.SRate[num] = SCD_CleanUpSound(samples[num].snd, true);
		HLock(samples[num].snd);
		
		/*if(thePrefs.SPlaying[num]) {
			theCommand.cmd		= soundCmd;
			theCommand.param1	= 0;
			theCommand.param2	= (long) *samples[num].snd;
			SndDoImmediate(samples[num].chan, &theCommand);
			
			theCommand.cmd		= freqCmd;
			theCommand.param1	= 0;
			theCommand.param2	= thePrefs.RNote[num];
			SndDoImmediate(samples[num].chan, &theCommand);
		}*/
		if(thePrefs.SPlaying[num])
		SCD_StartSample(num);
		UseResFile(oldResFile);
		break;
		
		case kSPrev:
		if(samples[num].totalSndNb == 1 || (thePrefs.flags & kNoInterruptBank))
		break;
		theCommand.cmd		= quietCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 0;
		SndDoImmediate(samples[num].chan, &theCommand);
		HUnlock(samples[num].snd);
		DisposeHandle(samples[num].snd);
		
		--samples[num].sndNb;
		if(samples[num].sndNb < 1)
		samples[num].sndNb = samples[num].totalSndNb;
		
		UseResFile(samples[num].fileID);
		samples[num].snd = Get1IndResource('snd ', samples[num].sndNb);
		
		GetResInfo(samples[num].snd, &resID, &resType, samples[num].name);
		DetachResource(samples[num].snd);
		thePrefs.SRate[num] = SCD_CleanUpSound(samples[num].snd, true);
		HLock(samples[num].snd);
		
		/*if(thePrefs.SPlaying[num]) {
			theCommand.cmd		= soundCmd;
			theCommand.param1	= 0;
			theCommand.param2	= (long) *samples[num].snd;
			SndDoImmediate(samples[num].chan, &theCommand);
			
			theCommand.cmd		= freqCmd;
			theCommand.param1	= 0;
			theCommand.param2	= thePrefs.RNote[num];
			SndDoImmediate(samples[num].chan, &theCommand);
		}*/
		if(thePrefs.SPlaying[num])
		SCD_StartSample(num);
		UseResFile(oldResFile);
		break;
		
		case kSVolume:
		thePrefs.SVol[tempRec[position].command] = tempRec[position].command_2;
		SCD_VolumeSample(tempRec[position].command);
		break;
		
		case kSPitch:
		thePrefs.SRelativeRate[tempRec[position].command] = tempRec[position].command_2;
		SCD_PitchSample(tempRec[position].command);
		break;
		
		case kIPlay:
		SCD_PlayNote(tempRec[position].command, tempRec[position].command_2);
		break;
		
		case kIStop:
		SCD_StopNote(tempRec[position].command, tempRec[position].command_2);
		break;
		
		case kIVolume:
		thePrefs.IVol = tempRec[position].command;
		SCD_VolumeSynthetizer();
		break;
		
		case kSPano:
		thePrefs.SPan[tempRec[position].command] = tempRec[position].command_2;
		SCD_VolumeSample(tempRec[position].command);
		break;
		
		case kSLoop:
		if(!thePrefs.SLoop[tempRec[position].command])
			thePrefs.SLoop[tempRec[position].command] = loopValue;
		else
			thePrefs.SLoop[tempRec[position].command] = 0;
		break;
		
		case kSInit:
		SCD_ReInitTrack(tempRec[position].command, tempRec[position].command_2);
		break;
		
		case kMPausRes:
		SCD_PauseResumeSampler();
		break;
		
		case kMLoopSpeed:
		thePrefs.loopSpeed = (1000 - tempRec[position].command * 100) / 4;
		break;
		
		case kMSynthMode:
		thePrefs.synthMode = tempRec[position].command;
		SCD_SetSynthetizerMode(thePrefs.synthMode);
		break;
		
		case kSTurbo:
		if(thePrefs.SMaxVol[tempRec[position].command] == thePrefs.STurbo) {
			thePrefs.SMaxVol[tempRec[position].command] = defaultMaxVolume;
			thePrefs.SVol[tempRec[position].command] = thePrefs.SVol[tempRec[position].command]
				/ (thePrefs.STurbo / defaultMaxVolume);
		}
		else {
			thePrefs.SMaxVol[tempRec[position].command] = thePrefs.STurbo;
			thePrefs.SVol[tempRec[position].command] = thePrefs.SVol[tempRec[position].command]
				* (thePrefs.STurbo / defaultMaxVolume);
		}
		SCD_VolumeSample(tempRec[position].command);
		break;
		
		case kSEffects:
		if(!thePrefs.SReverb)
		break;
		thePrefs.SSourceMode[tempRec[position].command] = tempRec[position].command_2;
		SCD_EffectsSample(tempRec[position].command);
		break;
		
		case kITurbo:
		if(thePrefs.IMaxVol == thePrefs.ITurbo) {
			thePrefs.IMaxVol = defaultMaxVolume;
			thePrefs.IVol = thePrefs.IVol / (thePrefs.ITurbo / defaultMaxVolume);
		}
		else {
			thePrefs.IMaxVol = thePrefs.ITurbo;
			thePrefs.IVol = thePrefs.IVol * (thePrefs.ITurbo / defaultMaxVolume);
		}
		SCD_VolumeSynthetizer();
		break;
		
		case kRPlay:
		RB_Start();
		break;
		
		case kRStop:
		RB_Stop();
		break;
		
		case kRVolume:
		thePrefs.RVol = tempRec[position].command;
		//SCD_VolumeRythmsBox();
		break;
		
		case kRBPM:
		thePrefs.RBPM = tempRec[position].command;
		interTime = -(15000000 / thePrefs.RBPM);
		break;
		
		case kRInstrument:
		if(rythms[tempRec[position].command].fileID)
		RB_PlaySound(tempRec[position].command);
		break;
		
		case kRTurbo:
		if(thePrefs.RMaxVol == thePrefs.RTurbo) {
			thePrefs.RMaxVol = defaultMaxVolume;
			thePrefs.RVol = thePrefs.RVol / (thePrefs.RTurbo / defaultMaxVolume);
		}
		else {
			thePrefs.RMaxVol = thePrefs.RTurbo;
			thePrefs.RVol = thePrefs.RVol * (thePrefs.RTurbo / defaultMaxVolume);
		}
		//SCD_VolumeRythmsBox();
		break;
		
		case kRMute:
		thePrefs.RPartMute[tempRec[position].command] = !thePrefs.RPartMute[tempRec[position].command];
		break;
		
		case kRNext:
		thePrefs.RNext = tempRec[position].command;
		break;
		
		case kRCurrent:
		thePrefs.RCurrent = tempRec[position].command;
		patternPos = -1;
		clockCount = 5;
		break;
		
		/*case kRPattern:
		if(pattern[thePrefs.RCurrent][tempRec[position].command][tempRec[position].command_2])
		pattern[thePrefs.RCurrent][tempRec[position].command][tempRec[position].command_2] = 0;
		else
		pattern[thePrefs.RCurrent][tempRec[position].command][tempRec[position].command_2] = 255;
		break;*/
		
		//case kEnd:
	}
	
	posArea[writePos] = position;
	++writePos;
	if(writePos >= kCycleSize)
	writePos = 0;
				
	if(tempRec[position].action != kEnd) {
		PrimeTime((QElemPtr) &readTask, (tempRec[position + 1].time
			 - tempRec[position].time) * 10.4); //!10
		++position;
	}
	else {
		//position = 0;
		//playing = false;
#if 1
		for(k = 0; k < thePrefs.nbSamples; ++k)
			if(samples[k].type == sampleType)
			SCD_StopSample(k);
		theCommand.cmd		= quietCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 0;
		for(k = 0; k < thePrefs.nbInsChannels; ++k)
		SndDoImmediate(insChannels[k], &theCommand);
		RB_Stop();
		for(k = 0; k < thePrefs.nbRytChannels; ++k)
		SndDoImmediate(rytChannels[k], &theCommand);
#endif
	}
	
#if kA5savy
	oldA5 = SetA5(oldA5);
#endif
}

void TM_InstallReadTask()
{
	readTask.theTask.tmAddr		= ReadTaskRoutine;
	readTask.theTask.tmWakeUp	= 0;
	readTask.theTask.tmReserved	= 0;
	readTask.appA5				= SetCurrentA5();
	
	InsXTime((QElemPtr) &readTask);
/*	if(position)
	PrimeTime((QElemPtr) &readTask, tempRec[position].time * 10 
		- tempRec[position - 1].time * 10);
	else*/
	startTime = TickCount();
	PrimeTime((QElemPtr) &readTask, tempRec[position].time * 10.4); //! 10 avant
}
	
void TM_KillReadTask()
{
	RmvTime((QElemPtr) &readTask);
}