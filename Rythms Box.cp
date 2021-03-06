#include			<Aliases.h>

#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"
#include			"Record Codes.h"

//PROTOTYPE:

pascal void		RB_QuantizeTaskFunction();

//VARIABLES LOCALES:

SndCommand			theCommand_2;
ExtTMTask			quantizeTask;
UniversalProcPtr	QuantizeTaskRoutine	= NewTimerProc(RB_QuantizeTaskFunction);

//FONCTIONS:

#if defined(powerc) || defined (__powerc)

#else
pascal ExtTMTaskPtr GetA1Value() = 0x2E89;
#endif

void RB_ClearPattern(short num)
{
	long		h,
				v;
	
	for(v = 0; v < kPatternParts; ++v)
		for(h = 0; h < kPatternLength; ++h)
		pattern[num][v][h] = 0;
}

void RB_CopyPattern(short current, short destination)
{
	short		j;
	
	for(j = 0; j < kPatternParts; ++j)
		for(i = 0; i < kPatternLength; ++i)
		pattern[destination][j][i] = pattern[current][j][i];
}

short RB_EditPart(short num)
{
	short		itemType,
				ID;
	Handle		textItem;
	Boolean	loop = true;
	long		temp;
	
	UseResFile(mainResFile);
	theDialog = GetNewDialog(7000, nil, (WindowPtr) -1);
	SetPort(theDialog);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	OutLine_Item(theDialog, 17);
	OutLine_Item(theDialog, 19);
	
	GetDItem(theDialog, 5, &itemType, &textItem, &aRect); //Part type
	if(rythms[num].fileID)
	SetIText(textItem, "\pSound file");
	else
		if(MIDIDriverEnabled)
		SetIText(textItem, "\pMidi");
		else
		SetIText(textItem, "\pNone");
	GetDItem(theDialog, 6, &itemType, &textItem, &aRect); //Part volume
	NumToString(thePrefs.RPartVol[num], theString);
	SetIText(textItem, theString);
	GetDItem(theDialog, 7, &itemType, &textItem, &aRect); //Part panoramic
	NumToString(thePrefs.RPartPan[num], theString);
	SetIText(textItem, theString);
	GetDItem(theDialog, 8, &itemType, &textItem, &aRect); //Part pitch
	NumToString(thePrefs.RPartPitch[num], theString);
	SetIText(textItem, theString);
	GetDItem(theDialog, 9, &itemType, &textItem, &aRect); //Sound name
	SetIText(textItem, rythms[num].name);
	SelIText(theDialog, 6, 0, 32767);
	SetDialogDefaultItem(theDialog, 1);
	
	while(loop) {
		ModalDialog(nil, &itemHit);
		switch(itemHit) {
			case 1:
			loop = false;
			break;
			
			case 2:
			RB_LoadSound(num);
			GetDItem(theDialog, 5, &itemType, &textItem, &aRect); //Part type
			if(rythms[num].fileID)
			SetIText(textItem, "\pSound file");
			else
				if(MIDIDriverEnabled)
				SetIText(textItem, "\pMidi");
				else
				SetIText(textItem, "\pNone");
			GetDItem(theDialog, 9, &itemType, &textItem, &aRect);
			SetIText(textItem, rythms[num].name);
			OutLine_Item(theDialog, 17);
			OutLine_Item(theDialog, 19);
			DrawDialog(theDialog);
			break;
			
			case 3:
			goto Get;
			Back:
			SelIText(theDialog, 6, 0, 32767);
			if(rythms[num].fileID)
			RB_PlaySound(num);
			else
			if(MIDIDriverEnabled)
			MIDI_SendNote(9, thePrefs.RPartPitch[num], thePrefs.RVol * thePrefs.RPartVol[num] / 810);
			break;
			
			case 4:
			RB_CloseSound(num);
			GetDItem(theDialog, 5, &itemType, &textItem, &aRect); //Part type
			if(MIDIDriverEnabled)
			SetIText(textItem, "\pMidi");
			else
			SetIText(textItem, "\pNone");
			GetDItem(theDialog, 9, &itemType, &textItem, &aRect);
			SetIText(textItem, rythms[num].name);
			break;
			
			case 23:
			HMSetBalloons(!HMGetBalloons());
			if(!HMGetBalloons()) {
				OutLine_Item(theDialog, 17);
				OutLine_Item(theDialog, 19);
				DrawDialog(theDialog);
			}
			break;
		}
	}
	
	Get:
	GetDItem(theDialog, 6, &itemType, &textItem, &aRect); //Part volume
	GetIText(textItem, theString);
	StringToNum(theString, &temp);
	if(temp < 0)
	temp = 0;
	if(temp > kDefaultPartVolume * 2)
	temp = kDefaultPartVolume * 2;
	thePrefs.RPartVol[num] = temp;
	
	GetDItem(theDialog, 7, &itemType, &textItem, &aRect); //Part panoramic
	GetIText(textItem, theString);
	StringToNum(theString, &temp);
	if(temp < 0)
	temp = 0;
	if(temp > 200)
	temp = 200;
	thePrefs.RPartPan[num] = temp;
	
	GetDItem(theDialog, 8, &itemType, &textItem, &aRect); //Part pitch
	GetIText(textItem, theString);
	StringToNum(theString, &temp);
	if(temp < 0)
	temp = 0;
	if(temp > 127)
	temp = 127;
	thePrefs.RPartPitch[num] = temp;
	
	if(loop)
	goto Back;
	
	DisposeDialog(theDialog);
	SetGWorld((CGrafPtr) rythmsWin, nil);
	return true;
}

short RB_OpenPatternSet()
{
	StandardFileReply	theReply;
	
	//R�cup�re le fichier
	UseResFile(mainResFile);
	CustomGetFile(nil, 1, typeList_4, &theReply, 20200, whereToShow, nil,
		nil, nil, nil, nil);
	if(theReply.sfGood)
	RB_LoadPatternSet(&theReply.sfFile);
	
	return true;
}

short RB_LoadPatternSet(FSSpec* patternSpec)
{
	short				fileNumber;
	
	//Ouvre le fichier
	theError = FSpOpenDF(patternSpec, fsRdPerm, &fileNumber); //fsRdWrPerm
	if(theError) {
		Do_Error(theError, 102);
		return false;
	}
	
	SetFPos(fileNumber, 1, 0);
	if(RB_ReadPatternSet(fileNumber)) {
		BlockMove(patternSpec->name, patternName, sizeof(Str63));
		SetGWorld((CGrafPtr) rythmsWin, nil);
		while(StringWidth(patternName) > kMaxWidthSet)
		--patternName[0];
	}
	else
	GetIndString(patternName, 0, 15);
	
	FSClose(fileNumber);
	if(thePrefs.quantizeFlags & kQSmplPitchAlign) {
		SetGWorld((CGrafPtr) mainWin, nil);
		for(i = 0; i < thePrefs.nbSamples; ++i)
		if(samples[i].fileID && samples[i].type == sampleType) {
			SCD_AlignSample(i);
			if(thePrefs.SPlaying[i])
			SCD_PitchSample(i);
		}
		SetGWorld((CGrafPtr) mainWin, nil);
		InvalRect(&pitchRect);
		SetGWorld((CGrafPtr) rythmsWin, nil);
	}
	
	if(thePrefs.rythmsWin)
	InvalRect(&rythmsRect);
	
	return true;
}
	
short RB_SavePatternSet()
{
	Str255				defaultName,
						DLGText;
	StandardFileReply	theReply;
	short				fileNumber;
	
#if demo
	Do_Error(-30008, 904);
	return false;
#else
	//O� sauver?
	UseResFile(mainResFile);
	GetIndString(defaultName, 0, 17);
	GetIndString(DLGText, 0, 18);
	CustomPutFile(DLGText, defaultName, &theReply, 30100, whereToShow, nil,
		nil, nil, nil, nil);
	if(!theReply.sfGood)
	return false;
	
	if(theReply.sfReplacing) { //Efface le fichier si existe d�j�
		theError = FSpDelete(&theReply.sfFile);
		if(theError) {
			Do_Error(theError, 104);
			return false;
		}
	}
	theError = FSpCreate(&theReply.sfFile, 'HiFc', 'Patt', theReply.sfScript); //Cr�e le fichier
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
	RB_WritePatternSet(fileNumber);
	
	FSClose(fileNumber);
	FlushVol(nil, musicFileSpec.vRefNum);
	
	BlockMove(theReply.sfFile.name, patternName, sizeof(Str63));
	while(StringWidth(patternName) > kMaxWidthSet)
	--patternName[0];
	
	return true;
#endif
}

pascal void RB_RythmeTaskFonction()
{
	ExtTMTaskPtr	rythmeTaskPtr = (ExtTMTaskPtr) 0x2E89;
#if kA5savy
	long			oldA5 = SetA5(rythmeTaskPtr->appA5);
#endif
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
		
#if 0
		theCommand_2.cmd		= quietCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= 0;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
		
		theCommand_2.cmd		= bufferCmd; //ou SoundCmd?
		theCommand_2.param1		= 0;
		theCommand_2.param2		= (long) *rythms[snd].snd;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
#else
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
#endif
	}
	else
	if(MIDIDriverEnabled) {
		if(snd > 1)
		MIDI_SendNote(9, thePrefs.RPartPitch[snd - 1], 0); //Note Off
		else
		MIDI_SendNote(9, thePrefs.RPartPitch[kPatternParts - 1], 0); //Note Off
		
		MIDI_SendNote(9, thePrefs.RPartPitch[snd], thePrefs.RVol * thePrefs.RPartVol[snd] / 810);
	}
	
	PrimeTime((QElemPtr) &rythmeTask, interTime);
	
#if kA5savy
	oldA5 = SetA5(oldA5);
#endif
}

pascal void RB_QuantizeTaskFunction()
{
	ExtTMTaskPtr	quantizeTaskPtr = (ExtTMTaskPtr) 0x2E89;
#if kA5savy
	long			oldA5 = SetA5(quantizeTaskPtr->appA5);
#endif
	short			temp;
	long			j;
	long			time = ActualTime;
	
	if(thePrefs.quantizeFlags & kQSmplStart) {
		for(j = 0; j < nbSamplesToStart; ++j) {
			temp = samplesToStart[j];
			if(!thePrefs.SPlaying[temp]) {
				if(recording) {
					++position;
					++time;
					tempRec[position].time = time;
					tempRec[position].action = kSStart;
					tempRec[position].command = temp;
					tempRec[position].command_2 = 0;
				}
				if(samples[temp].type == sampleType)
				SCD_StartSample(temp);
			}
		}
		nbSamplesToStart = 0;
	}
	
	if(thePrefs.quantizeFlags & kQSmplStop) {
		for(j = 0; j < nbSamplesToStop; ++j) {
			temp = samplesToStop[j];
			if(thePrefs.SPlaying[temp]) {
				if(recording) {
					++position;
					++time;
					tempRec[position].time = time;
					tempRec[position].action = kSStop;
					tempRec[position].command = temp;
					tempRec[position].command_2 = 0;
				}
				if(samples[temp].type == sampleType)
				SCD_StopSample(temp);
			}
		}
		nbSamplesToStop = 0;
	}
	
	if(thePrefs.quantizeFlags & kQRB) {
		for(temp = 0; temp < nbRBMutes; ++temp) {
			thePrefs.RPartMute[RBMutes[temp]] = !thePrefs.RPartMute[RBMutes[temp]];
			
			if(recording) {
				++position;
				++time;
				tempRec[position].time = time;
				tempRec[position].action = kRMute;
				tempRec[position].command = RBMutes[temp];
				tempRec[position].command_2 = 0;
			}
		}
		nbRBMutes = 0;
	}
		
#if kA5savy
	oldA5 = SetA5(oldA5);
#endif
}

void RB_Start()
{
	if(!thePrefs.RPlaying) {
		rythmeTask.theTask.tmAddr		= RythmeTaskRoutine;
		rythmeTask.theTask.tmWakeUp		= 0;
		rythmeTask.theTask.tmReserved	= 0;
		rythmeTask.appA5				= SetCurrentA5();
		
		//patternPos = -1; // au lieu de 0
		
		if(thePrefs.MIDIFlags & kUseHFClock) {
			InsXTime((QElemPtr) &rythmeTask);
			//PrimeTime((QElemPtr) &rythmeTask, interTime);
			RB_RythmeTaskFonction();
		}
		
		quantizeTask.theTask.tmAddr		= QuantizeTaskRoutine;
		quantizeTask.theTask.tmWakeUp	= 0;
		quantizeTask.theTask.tmReserved	= 0;
		quantizeTask.appA5				= SetCurrentA5();
		InsXTime((QElemPtr) &quantizeTask);
		
		thePrefs.RPlaying = true;
		DisableItem(popUpMenu2, 0);
	}
}

void RB_Stop()
{
	if(thePrefs.RPlaying) {
		if(thePrefs.MIDIFlags & kUseHFClock)
		RmvTime((QElemPtr) &rythmeTask);
	
		RmvTime((QElemPtr) &quantizeTask);
		
		/*theCommand_2.cmd		= quietCmd;
		theCommand_2.param1	= 0;
		theCommand_2.param2	= 0;
		for(i = 0; i < thePrefs.nbRytChannels; ++i)
		SndDoImmediate(rytChannels[i], &theCommand_2);*/
		
		thePrefs.RPlaying = false;
		if(!recording && !playing)
		EnableItem(popUpMenu2, 0);
	}
}

void RB_PlaySound(short num)
{
	long			partVol,
					volume;
					
	++activeRythmChan;
	if(activeRythmChan > thePrefs.nbRytChannels - 1)
	activeRythmChan = 0;
	
#if 0
		theCommand_2.cmd		= quietCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= 0;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
		
		theCommand_2.cmd		= bufferCmd; //ou SoundCmd?
		theCommand_2.param1		= 0;
		theCommand_2.param2		= (long) *rythms[num].snd;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
#else
		theCommand_2.cmd		= quietCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= 0;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
		
		partVol = thePrefs.RVol * thePrefs.RPartVol[num] / 200;
		if(thePrefs.RPartPan[num] > 100)
		volume = Shorts_To_Long(partVol * (200 - thePrefs.RPartPan[num]) / 100, partVol);
		else
		volume = Shorts_To_Long(partVol, partVol * (thePrefs.RPartPan[num]) / 100);
		theCommand_2.cmd		= volumeCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= volume;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
		
		theCommand_2.cmd		= soundCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= (long) *rythms[num].snd;
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
		
		theCommand_2.cmd		= freqCmd;
		theCommand_2.param1		= 0;
		theCommand_2.param2		= thePrefs.RPartPitch[num];
		SndDoImmediate(rytChannels[activeRythmChan], &theCommand_2);
#endif
}

typedef OSErr (*LIPI_Proc)(FSSpecPtr, Handle*, long, Fixed*);

short RB_LoadSound(short num)
{
	StandardFileReply	theReply;
	openData			theData;
	long				j;
	SymClass			theClass;
	LIPI_Proc			theProcPtr;
	Boolean			loop = true;
	Fixed				rate;
	
	/*if(isCMusFile) {
		Do_Error(-30006, 409);
		return false;
	}*/
	
	UseResFile(mainResFile);
	theData.reply = &theReply;
	theData.extendedDLOG = thePrefs.flags & kExtendedOpenDLG;
	theData.update = true;
	theData.fileListPtr = importFileTypes;
	theData.addBank = false;
	CustomGetFile(FileFilterRoutine, 0, nil, &theReply, 22000, whereToShow, DialogHookRoutine,
		nil, nil, nil, &theData);
	if(theData.extendedDLOG)
	thePrefs.flags = thePrefs.flags | kExtendedOpenDLG;
	else
	thePrefs.flags = thePrefs.flags & (~kExtendedOpenDLG);
	if(!theReply.sfGood)
	return false;
	
	RB_CloseSound(num);
	
	//Trouve le Plug-In ad�quat
	i = 0; j = 0;
	while(loop) {
		for(j = 0; j < kTypesPerImport; ++j)
		if(importPlugIns[i].importFileTypes[j] == theReply.sfType)
		loop = false;
		
		if(loop)
		++i;
	}

	//Appelle le Plug-In
	GetIndString(theString, 0, 23);
	theError = FindSymbol(importPlugIns[i].connID, theString, (char**) &theProcPtr, &theClass);
	if(theError) {
		Do_Error(theError, 105);
		return false;
	}
	if(thePrefs.flags & kByPassHeaders)
	theError = (*theProcPtr)(&theReply.sfFile, &rythms[num].snd, kNewHeader, &rate);
	else
	theError = (*theProcPtr)(&theReply.sfFile, &rythms[num].snd, 0L, &rate);
	if(theError) {
		Do_Error(theError, 106);
		return false;
	}
	HLock(rythms[num].snd);
	UseResFile(mainResFile);
	
	//TextFont(3);
	TextSize(9);
	TextFace(0);
	BlockMove(theReply.sfFile.name, rythms[num].name, sizeof(Str63));
	while(StringWidth(rythms[num].name) > kMaxWidthRyth)
	--rythms[num].name[0];
	//TextFont(3);
	TextSize(10);
	TextFace(1);
		
	rythms[num].fileID = -1;
	rythms[num].spec.vRefNum = theReply.sfFile.vRefNum;
	rythms[num].spec.parID = theReply.sfFile.parID;
	BlockMove(theReply.sfFile.name, rythms[num].spec.name, 64);
	
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *rythms[num].snd;
	for(i = 0; i < thePrefs.nbRytChannels; ++i)
	SndDoImmediate(rytChannels[i], &theCommand);
	
#if(kCreateFileIDs)
	theBlock.fidParam.ioNamePtr = theReply.sfFile.name;
	theBlock.fidParam.ioVRefNum = theReply.sfFile.vRefNum;
	theBlock.fidParam.ioSrcDirID = theReply.sfFile.parID;
	PBCreateFileIDRef(&theBlock, false);
#endif
	return true;
}

short RB_CloseSound(short num)
{
	rythms[num].spec.vRefNum	= 0;
	rythms[num].spec.parID		= 0;
	rythms[num].spec.name[0]	= 0;
	if(!rythms[num].fileID)
	return false;
	
	rythms[num].fileID = 0;
	HUnlock(rythms[num].snd);
	DisposeHandle(rythms[num].snd);
	GetIndString(rythms[num].name, 0, 16);
	
	return true;
}

short RB_WritePatternSet(short fileID)
{
	long		bytesNumber,
				aliasSize;
	AliasHandle	theAlias = (AliasHandle) NewHandle(1000);
	
	//�crit le num�ro de version du set
	bytesNumber = 2;
	FSWrite(fileID, &bytesNumber, &pattVersion);
	
	//Ecrit les patterns
	bytesNumber = sizeof(pattern);
	FSWrite(fileID, &bytesNumber, pattern);
	
	//�crit le num�ro de version des pr�f�rences
	bytesNumber = 2;
	FSWrite(fileID, &bytesNumber, &prefVersion);
	
	//Ecrit les pr�f�rences
	bytesNumber = sizeof(prefs);
	FSWrite(fileID, &bytesNumber, &thePrefs);
	
	//Ecrit les r�f�rences des fichiers sons de la bo�te � rythmes
	bytesNumber = 4;
	for(i = 0; i < kPatternParts; ++i) {
		NewAliasMinimal(&rythms[i].spec, &theAlias);
		HLock((Handle) theAlias);
		aliasSize = GetHandleSize((Handle) theAlias);
		if(!rythms[i].fileID)
		aliasSize = 0;
		FSWrite(fileID, &bytesNumber, &aliasSize);
		FSWrite(fileID, &aliasSize, *theAlias);
	}
	HUnlock((Handle) theAlias);
	
	return true;
}

short RB_ReadPatternSet(short fileID)
{
	short				item,
						tempID;
	long				bytesNumber,
						aliasSize;
	AliasHandle			theAlias = (AliasHandle) NewHandle(1000);
	Boolean			isDifferent,
						forceLocate = false,
						loop;
	StandardFileReply	theReply;
	prefs				tempPrefs;
	FInfo				theInfo;
	long				j,
						k;
	SymClass			theClass;
	LIPI_Proc			theProcPtr;
	Fixed				rate;
	
	//Arr�te la bo�te � rythmes
	if(thePrefs.RPlaying) {
		RB_Stop();
		theCommand.cmd		= quietCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 0;
		for(i = 0; i < thePrefs.nbRytChannels; ++i)
		SndDoImmediate(rytChannels[i], &theCommand);
	}
	nbRBMutes = 0;
	
	if(event.modifiers & optionKey)
	forceLocate = true;
	
	//Contr�le le num�ro de version du set
	bytesNumber = 2;
	FSRead(fileID, &bytesNumber, &tempID);
	if(tempID != pattVersion) {
		Do_Error(-30000, 407);
		return false;
	}
	
	//Lit les patterns
	bytesNumber = sizeof(pattern);
	FSRead(fileID, &bytesNumber, pattern);
	
	//Contr�le le num�ro de version des pr�f�rences
	bytesNumber = 2;
	FSRead(fileID, &bytesNumber, &tempID);
	/*if(tempID != prefVersion) {
		Do_Error(-30000, 407);
		return false;
	}*/
	
	//Lit les pr�f�rences
	bytesNumber = sizeof(prefs);
	FSRead(fileID, &bytesNumber, &tempPrefs);
	//Met � jour les prefs
	thePrefs.RBPM = tempPrefs.RBPM;
	interTime = -(15000000 / thePrefs.RBPM);
	thePrefs.RCurrent = tempPrefs.RCurrent;
	thePrefs.RNext = tempPrefs.RNext;
	thePrefs.RVol = tempPrefs.RVol;
	thePrefs.RMaxVol = tempPrefs.RMaxVol;
	//SCD_VolumeRythmsBox();
	for(i = 0; i < kPatternParts; ++i) {
		thePrefs.RPartVol[i] = tempPrefs.RPartVol[i];
		thePrefs.RPartPan[i] = tempPrefs.RPartPan[i];
		thePrefs.RPartPitch[i] = tempPrefs.RPartPitch[i];
		thePrefs.RPartMute[i] = tempPrefs.RPartMute[i];
	}
	
	//Ferme tous les fichiers sons
	for(i = 0; i < kPatternParts; ++i)
	RB_CloseSound(i);
	
	bytesNumber = 4;
	//Charge les fichiers sons de la bo�te � rythmes
	for(i = 0; i < kPatternParts; ++i) {
		FSRead(fileID, &bytesNumber, &aliasSize);
		if(aliasSize > 0) {
			SetHandleSize((Handle) theAlias, aliasSize);
			HLock((Handle) theAlias);
			FSRead(fileID, &aliasSize, *theAlias);
			GetAliasInfo(theAlias, asiAliasName, rythms[i].spec.name);
			rythms[i].spec.vRefNum = musicFileSpec.vRefNum;
			rythms[i].spec.parID = musicFileSpec.parID;
			theError = 0;
			if(HGetFInfo(rythms[i].spec.vRefNum, rythms[i].spec.parID, rythms[i].spec.name, nil))
			theError = ResolveAlias(nil, theAlias, &rythms[i].spec, &isDifferent);
			if(theError || forceLocate) { //Impossible de trouver le fichier son
				GetAliasInfo(theAlias, asiAliasName, rythms[i].spec.name);
				ParamText(rythms[i].spec.name, nil, nil, nil);
				UseResFile(mainResFile);
				CustomGetFile(nil, -1, nil, &theReply, 10000, whereToShow, nil, nil, nil, nil, nil);
				if(!theReply.sfGood)
				continue;
				
				rythms[i].spec.vRefNum = theReply.sfFile.vRefNum;
				rythms[i].spec.parID = theReply.sfFile.parID;
				BlockMove(theReply.sfFile.name, rythms[i].spec.name, 63);
#if(kCreateFileIDs)
				theBlock.fidParam.ioNamePtr = theReply.sfFile.name;
				theBlock.fidParam.ioVRefNum = theReply.sfFile.vRefNum;
				theBlock.fidParam.ioSrcDirID = theReply.sfFile.parID;
				PBCreateFileIDRef(&theBlock, false);
#endif
			}
		}
	}
	HUnlock((Handle) theAlias);
	
	//Charge les sons
	SetGWorld((CGrafPtr) rythmsWin, nil);
	TextFont(3);
	for(i = 0; i < kPatternParts; ++i)
	if(rythms[i].spec.parID) {
		if(FSpGetFInfo(&rythms[i].spec, &theInfo)) {
			Do_Error(ResError(), 102);
			return false;
		}
		
		//Trouve le Plug-In ad�quat
		j = 0; k = 0; loop = true;
		while(loop) {
			for(k = 0; k < kTypesPerImport; ++k)
			if(importPlugIns[j].importFileTypes[k] == theInfo.fdType)
			loop = false;
			
			if(loop)
			++j;
			if(j >= importPlugInsNumber) {
				Do_Error(-30007, 107);
				continue;
			}
		}
	
		//Appelle le Plug-In
		GetIndString(theString, 0, 23);
		theError = FindSymbol(importPlugIns[j].connID, theString, (char**) &theProcPtr, &theClass);
		if(theError) {
			Do_Error(theError, 105);
			return false;
		}
		if(thePrefs.flags & kByPassHeaders)
		theError = (*theProcPtr)(&rythms[i].spec, &rythms[i].snd, kNewHeader, &rate);
		else
		theError = (*theProcPtr)(&rythms[i].spec, &rythms[i].snd, 0L, &rate);
		if(theError) {
			Do_Error(theError, 106);
			return false;
		}
		HLock(rythms[i].snd);
		UseResFile(mainResFile);
		
		//TextFont(3);
		//TextSize(9);
		//TextFace(0);
		BlockMove(rythms[i].spec.name, rythms[i].name, sizeof(Str63));
		while(StringWidth(rythms[i].name) > kMaxWidthRyth)
		--rythms[i].name[0];
		//TextFont(3);
		//TextSize(10);
		//TextFace(1);
			
		rythms[i].fileID = -1;
		
		theCommand.cmd		= soundCmd; //Joue le son
		theCommand.param1	= 0;
		theCommand.param2	= (long) *rythms[i].snd;
		for(k = 0; k < thePrefs.nbRytChannels; ++k)
		SndDoImmediate(rytChannels[k], &theCommand);
	}
	TextFont(200);
	
	return true;
}