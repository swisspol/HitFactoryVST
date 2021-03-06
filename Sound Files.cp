#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"
#include			"Record Codes.h"

// FONCTIONS:

short SF_OpenBank(FSSpecPtr fileSpecPtr, short num)
{
	short				tempID;
	
	tempID = FSpOpenResFile(fileSpecPtr, fsRdPerm);
	if(tempID == -1) {
		Do_Error(ResError(), 102);
		return false;
	}
	
	UseResFile(tempID);
	samples[num].snd = Get1IndResource('snd ', 1);
	if(!samples[num].snd) {
		Do_Error(0, 502);
		CloseResFile(tempID);
		return false;
	}
	GetResInfo(samples[num].snd, &resID, &resType, samples[num].name);
	while(StringWidth(samples[num].name) > maxWidthSmpl)
	--samples[num].name[0];

	DetachResource(samples[num].snd);
	thePrefs.SRate[num] = SCD_CleanUpSound(samples[num].snd, true);
	thePrefs.SRelativeRate[num] = 0;
	HLock(samples[num].snd);
	
	samples[num].fileID = tempID;
	samples[num].sndNb = 1;
	samples[num].totalSndNb = Count1Resources('snd ');
	
	return true;
}

typedef OSErr (*LIPI_Proc)(FSSpecPtr, Handle*, long, Fixed*);

short SF_OpenSample(short num)
{
	StandardFileReply	theReply;
	openData			theData;
	long				j;
	SymClass			theClass;
	LIPI_Proc			theProcPtr;
	Boolean			loop = true;
	
	/*if(isCMusFile) {
		Do_Error(-30006, 409);
		return false;
	}*/
	
	//R残up俊e le fichier
	UseResFile(mainResFile);
	theData.reply = &theReply;
	theData.extendedDLOG = thePrefs.flags & kExtendedOpenDLG;
	theData.update = true;
	theData.fileListPtr = importFileTypes;
	theData.addBank = true;
	CustomGetFile(FileFilterRoutine, 0, nil, &theReply, 22000, whereToShow, DialogHookRoutine,
		nil, nil, nil, &theData);
	if(theData.extendedDLOG)
	thePrefs.flags = thePrefs.flags | kExtendedOpenDLG;
	else
	thePrefs.flags = thePrefs.flags & (~kExtendedOpenDLG);
	if(!theReply.sfGood)
	return false;
	
	SF_CloseSample(num);
	
	if(theReply.sfType != 'Bank') {
		//Trouve le Plug-In ad子uat
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
		theError = (*theProcPtr)(&theReply.sfFile, &samples[num].snd, kLoopSound + kNewHeader, &thePrefs.SRate[num]);
		else
		theError = (*theProcPtr)(&theReply.sfFile, &samples[num].snd, kLoopSound, &thePrefs.SRate[num]);
		if(theError) {
			Do_Error(theError, 106);
			return false;
		}
		thePrefs.SRelativeRate[num] = 0;
		HLock(samples[num].snd);
		UseResFile(mainResFile);
		
		BlockMove(theReply.sfFile.name, samples[num].name, sizeof(Str63));
		while(StringWidth(samples[num].name) > maxWidthSmpl)
		--samples[num].name[0];
		
		samples[num].fileID = -1;
		samples[num].sndNb = 1;
		samples[num].totalSndNb = 1;
	}
	else
	SF_OpenBank(&theReply.sfFile, num);
	
	samples[num].type = sampleType;
	samples[num].spec.vRefNum = theReply.sfFile.vRefNum;
	samples[num].spec.parID = theReply.sfFile.parID;
	BlockMove(theReply.sfFile.name, samples[num].spec.name, sizeof(Str63));
	
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *samples[num].snd;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	if(thePrefs.quantizeFlags & kQSmplPitchAlign)
	SCD_AlignSample(num);
	
#if(kCreateFileIDs)
	theBlock.fidParam.ioNamePtr = theReply.sfFile.name;
	theBlock.fidParam.ioVRefNum = theReply.sfFile.vRefNum;
	theBlock.fidParam.ioSrcDirID = theReply.sfFile.parID;
	PBCreateFileIDRef(&theBlock, false);
#endif
	return true;
}

short SF_FastLoad(short num, Str63 fileName, long dirID)
{
	FSSpec			theSpec;
	Boolean		wasPlaying = thePrefs.SPlaying[num],
					loop = true;
	FInfo			theInfo;
	long				j;
	SymClass			theClass;
	LIPI_Proc			theProcPtr;
	
	theSpec.vRefNum = thePrefs.soundsFolder.vRefNum;
	theSpec.parID = dirID;
	BlockMove(fileName, theSpec.name, sizeof(Str63));
	
	SF_CloseSample(num);
	
	if(FSpGetFInfo(&theSpec, &theInfo))
	return false;
	
	if(theInfo.fdType != 'Bank') {
		//Trouve le Plug-In ad子uat
		i = 0; j = 0;
		while(loop) {
			for(j = 0; j < kTypesPerImport; ++j)
			if(importPlugIns[i].importFileTypes[j] == theInfo.fdType)
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
		theError = (*theProcPtr)(&theSpec, &samples[num].snd, kLoopSound + kNewHeader, &thePrefs.SRate[num]);
		else
		theError = (*theProcPtr)(&theSpec, &samples[num].snd, kLoopSound, &thePrefs.SRate[num]);
		if(theError) {
			Do_Error(theError, 106);
			return false;
		}
		thePrefs.SRelativeRate[num] = 0;
		HLock(samples[num].snd);
		UseResFile(mainResFile);
		
		BlockMove(theSpec.name, samples[num].name, sizeof(Str63));
		while(StringWidth(samples[num].name) > maxWidthSmpl)
		--samples[num].name[0];
		
		samples[num].fileID = -1;
		samples[num].sndNb = 1;
		samples[num].totalSndNb = 1;
	}
	else
	SF_OpenBank(&theSpec, num);
		
	samples[num].type = sampleType;
	samples[num].spec.vRefNum = theSpec.vRefNum;
	samples[num].spec.parID = theSpec.parID;
	BlockMove(theSpec.name, samples[num].spec.name, 64);
	
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *samples[num].snd;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	if(thePrefs.quantizeFlags & kQSmplPitchAlign)
	SCD_AlignSample(num);
	
#if(kCreateFileIDs)
	theBlock.fidParam.ioNamePtr = theSpec.name;
	theBlock.fidParam.ioVRefNum = theSpec.vRefNum;
	theBlock.fidParam.ioSrcDirID = theSpec.parID;
	PBCreateFileIDRef(&theBlock, false);
#endif

	if(wasPlaying) {
		if(thePrefs.quantizeFlags & kQSmplStart) {
			for(i = 0; i < nbSamplesToStart; ++i)
			if(num == samplesToStart[i])
			return true;
			
			samplesToStart[nbSamplesToStart] = num;
			++nbSamplesToStart;
		}
		else
		SCD_StartSample(num);
	}
	
	return true;
}
	
typedef OSErr (*ODDPI_Proc)(FSSpecPtr, long, Fixed*, DTDPlugInParametersPtr);

short SF_OpenDirectToDisk(short num)
{
	StandardFileReply	theReply;
	openData			theData;
	long				j;
	SymClass			theClass;
	ODDPI_Proc			theProcPtr;
	Boolean			loop = true;
	DTDPlugInParameters	theParams;
	
	/*if(isCMusFile) {
		Do_Error(-30006, 409);
		return false;
	}*/
	
	UseResFile(mainResFile);
	theData.reply = &theReply;
	theData.extendedDLOG = thePrefs.flags & kExtendedOpenDLG;
	theData.update = true;
	theData.fileListPtr = DTDFileTypes;
	theData.addBank = true;
	CustomGetFile(FileFilterRoutine, 0, nil, &theReply, 20100, whereToShow, nil,
		nil, nil, nil, &theData);
	if(theData.extendedDLOG)
	thePrefs.flags = thePrefs.flags | kExtendedOpenDLG;
	else
	thePrefs.flags = thePrefs.flags & (~kExtendedOpenDLG);
	if(!theReply.sfGood)
	return false;
	
	SF_CloseSample(num);
	
	//Trouve le Plug-In ad子uat
	i = 0; j = 0;
	while(loop) {
		for(j = 0; j < kTypesPerDTD; ++j)
		if(DTDPlugIns[i].DTDFileTypes[j] == theReply.sfType)
		loop = false;
		
		if(loop)
		++i;
	}
	
	//Appelle le Plug-In
	GetIndString(theString, 0, 26);
	theError = FindSymbol(DTDPlugIns[i].connID, theString, (char**) &theProcPtr, &theClass);
	if(theError) {
		Do_Error(theError, 105);
		return false;
	}
	samples[num].snd = NewHandle(thePrefs.bufferSize);
	if(!samples[num].snd) {
		Do_Error(0, 702);
		samples[num].sndNb = 0;
		samples[num].totalSndNb = 0;
		GetIndString(samples[num].name, 0, 10);
		return false;
	}
	HLock(samples[num].snd);
	theParams.channel = samples[num].chan;
	theParams.fileID = 0;
	theParams.bufferHandle = samples[num].snd;
	theError = (*theProcPtr)(&theReply.sfFile, 0L, &thePrefs.SRate[num], &theParams);
	if(theError) {
		Do_Error(theError, 106);
		return false;
	}
	thePrefs.SRelativeRate[num] = 0;
	UseResFile(mainResFile);
	
	BlockMove(theReply.sfFile.name, samples[num].name, sizeof(Str63));
	while(StringWidth(samples[num].name) > maxWidthSmpl)
	--samples[num].name[0];
	
	samples[num].fileID = theParams.fileID;
	samples[num].sndNb = DTDCode;
	samples[num].totalSndNb = i;
	
	samples[num].type = directToDiskType;
	samples[num].spec.vRefNum = theReply.sfFile.vRefNum;
	samples[num].spec.parID = theReply.sfFile.parID;
	BlockMove(theReply.sfFile.name, samples[num].spec.name, sizeof(Str63));
	
#if(kCreateFileIDs)
	theBlock.fidParam.ioNamePtr = theReply.sfFile.name;
	theBlock.fidParam.ioVRefNum = theReply.sfFile.vRefNum;
	theBlock.fidParam.ioSrcDirID = theReply.sfFile.parID;
	PBCreateFileIDRef(&theBlock, false);
#endif

	return true;
}

short SF_OpenInstrument(short num)
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
		
	SF_CloseInstrument(num);
	
	//Trouve le Plug-In ad子uat
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
	theError = (*theProcPtr)(&theReply.sfFile, &instruments[num].snd, kNewHeader, &rate);
	else
	theError = (*theProcPtr)(&theReply.sfFile, &instruments[num].snd, 0L, &rate);
	if(theError) {
		Do_Error(theError, 106);
		return false;
	}
	HLock(instruments[num].snd);
	UseResFile(mainResFile);
	
	BlockMove(theReply.sfFile.name, instruments[num].name, sizeof(Str63));
	
	instruments[num].fileID = -1;
	instruments[num].spec.vRefNum = theReply.sfFile.vRefNum;
	instruments[num].spec.parID = theReply.sfFile.parID;
	BlockMove(theReply.sfFile.name, instruments[num].spec.name, 64);
	
	theCommand.cmd		= soundCmd;
	theCommand.param1	= 0;
	theCommand.param2	= (long) *instruments[num].snd;
	for(i = 0; i < thePrefs.nbInsChannels; ++i)
	SndDoImmediate(insChannels[i], &theCommand);
	
#if(kCreateFileIDs)
	theBlock.fidParam.ioNamePtr = theReply.sfFile.name;
	theBlock.fidParam.ioVRefNum = theReply.sfFile.vRefNum;
	theBlock.fidParam.ioSrcDirID = theReply.sfFile.parID;
	PBCreateFileIDRef(&theBlock, false);
#endif
	return true;
}
	
short SF_LoadNextSample(short num)
{
	if(samples[num].totalSndNb == 1)
	return false;
	
	theCommand.cmd		= quietCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	HUnlock(samples[num].snd);
	DisposeHandle(samples[num].snd);
	
	UseResFile(samples[num].fileID);
	++samples[num].sndNb;
	if(samples[num].sndNb > samples[num].totalSndNb)
	samples[num].sndNb = 1;
	
	samples[num].snd = Get1IndResource('snd ', samples[num].sndNb);
	if(!samples[num].snd) {
		Do_Error(0, 502);
		return false;
	}
	
	GetResInfo(samples[num].snd, &resID, &resType, samples[num].name);
	while(StringWidth(samples[num].name) > maxWidthSmpl)
	--samples[num].name[0];
	
	DetachResource(samples[num].snd);
	thePrefs.SRate[num] = SCD_CleanUpSound(samples[num].snd, true);
	HLock(samples[num].snd);
	
	if(thePrefs.quantizeFlags & kQSmplPitchAlign && !(event.modifiers & optionKey)) {
		SCD_AlignSample(num);
		if(uc[num] != -1) {
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 0, 213, 80, 229);
			SetRect(&destRect, 80 * uc[num], 213, 80 + 80 * uc[num], 229);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			SetRect(&sourceRect, 472, 120, 486, 136);
			itemHit = 40 + 80 * uc[num] + thePrefs.SRelativeRate[num];
			SetRect(&destRect, itemHit - 7, 213, itemHit + 7, 229);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			RGBBackColor(&theBackColor);
		}
	}
	
	if(thePrefs.SPlaying[num])
	SCD_StartSample(num);
	
	return true;
}

short SF_LoadPrevSample(short num)
{
	if(samples[num].totalSndNb == 1)
	return false;
	
	theCommand.cmd		= quietCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	SndDoImmediate(samples[num].chan, &theCommand);
	
	HUnlock(samples[num].snd);
	DisposeHandle(samples[num].snd);
	
	UseResFile(samples[num].fileID);
	--samples[num].sndNb;
	if(samples[num].sndNb < 1)
	samples[num].sndNb = samples[num].totalSndNb;
	
	samples[num].snd = Get1IndResource('snd ', samples[num].sndNb);
	if(!samples[num].snd) {
		Do_Error(0, 502);
		return false;
	}
	
	GetResInfo(samples[num].snd, &resID, &resType, samples[num].name);
	while(StringWidth(samples[num].name) > maxWidthSmpl)
	--samples[num].name[0];
	
	DetachResource(samples[num].snd);
	thePrefs.SRate[num] = SCD_CleanUpSound(samples[num].snd, true);
	HLock(samples[num].snd);
	
	if(thePrefs.quantizeFlags & kQSmplPitchAlign && !(event.modifiers & optionKey)) {
		SCD_AlignSample(num);
		if(uc[num] != -1) {
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 0, 213, 80, 229);
			SetRect(&destRect, 80 * uc[num], 213, 80 + 80 * uc[num], 229);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			SetRect(&sourceRect, 472, 120, 486, 136);
			itemHit = 40 + 80 * uc[num] + thePrefs.SRelativeRate[num];
			SetRect(&destRect, itemHit - 7, 213, itemHit + 7, 229);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			RGBBackColor(&theBackColor);
		}
	}
	
	if(thePrefs.SPlaying[num])
	SCD_StartSample(num);
	
	return true;
}

typedef OSErr (*CDDPI_Proc)(DTDPlugInParametersPtr);

short SF_CloseSample(short num)
{
	SymClass			theClass;
	CDDPI_Proc			theProcPtr;
	DTDPlugInParameters	theParams;
	
	samples[num].spec.vRefNum	= 0;
	samples[num].spec.parID		= 0;
	samples[num].spec.name[0]	= 0;
	if(!samples[num].fileID)
	return false;
	
	if(thePrefs.SPlaying[num]) {
		if(samples[num].type == sampleType)
		SCD_StopSample(num);
		else
		SCD_StopDirectToDisk(num);
	}
		
	if(samples[num].type == sampleType && samples[num].fileID != -1) //Cas Bank
	CloseResFile(samples[num].fileID);
	if(samples[num].type == directToDiskType) {
		GetIndString(theString, 0, 31);
		theError = FindSymbol(DTDPlugIns[samples[num].totalSndNb].connID, theString, (char**) &theProcPtr, &theClass);
		if(theError) {
			Do_Error(theError, 105);
			return false;
		}
		theParams.channel = samples[num].chan;
		theParams.fileID = samples[num].fileID;
		theParams.bufferHandle = samples[num].snd;
		theError = (*theProcPtr)(&theParams);
		if(theError) {
			Do_Error(theError, 106);
			return false;
		}
	}

	samples[num].fileID = 0;
	samples[num].type = sampleType;
	samples[num].sndNb = 0;
	samples[num].totalSndNb = 0;
	HUnlock(samples[num].snd);
	DisposeHandle(samples[num].snd);
	GetIndString(samples[num].name, 0, 10);
	
	thePrefs.SPlaying[num] = false;
	
	return true;
}

short SF_CloseInstrument(short num)
{
	instruments[num].spec.vRefNum	= 0;
	instruments[num].spec.parID 	= 0;
	instruments[num].spec.name[0]	= 0;
	if(!instruments[num].fileID)
	return false;
	
	instruments[num].fileID = 0;
	HUnlock(instruments[num].snd);
	DisposeHandle(instruments[num].snd);
	GetIndString(instruments[num].name, 0, 11);
	
	return true;
}