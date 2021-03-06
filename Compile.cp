#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"
#include			"Record Codes.h"

//CONSTANTES LOCALES:

#define				kExtSHSize			64
#define				kStdSHSize			22

#define				kAutoPlay			128
#define				kAutoQuit			64
#define				kLoop				32

//STRUCTURES LOCALES:

typedef struct playerPrefs
{
	short				smpPoly;
	short				sytPoly;
	short				rytPoly;
	short				volume;
	unsigned char		flags;
};
typedef playerPrefs* playerPrefsPtr;

// FONCTIONS:
	
/*void UFixed2Ext80(UnsignedFixed num, extended80 ext)
{
	long double		temp;
	
	temp = Fix2X(num);
	ldtox80(&temp, &ext);
}*/

Boolean HasSoundManager3_2()
{
	NumVersion		version;
	
	version = SndSoundManagerVersion();
	if((version.majorRev > 3) || ((version.majorRev == 3) && (version.minorAndBugRev >= 0x20)))
	return true;
	else
	return false;
}

/*short GetRelativeRate(Handle sound)
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
	short				times,
						correction;
	
	//R�cup�re les donn�es
	stdHeader = (SoundHeaderPtr) *sound;
	extHeader = (ExtSoundHeaderPtr) *sound;
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
	return 0;
	
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
			correction = frequency / kProgress;
		}
		while(correction > kMaxRelativeRate);
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
			correction = - (frequency / kProgress);
		}
		while(correction < kMinRelativeRate);
	}
	
	return correction;
}*/

OSErr Convert_Sound(Handle srcHandle, Handle* destHandle, short sampleSize, UnsignedFixed sampleRate, short numChannels)
{
	SoundConverter		sc;
	SoundComponentData	inputFormat,
						outputFormat;
	unsigned long		inputFrames,
						inputBytes;
	unsigned long		outputFrames,
						outputBytes;
	unsigned long		soundSize,
						soundFrames;
	Ptr					inputPtr,
						outputPtr;
	OSErr				err;
	ExtSoundHeaderPtr	extHeaderPtr;
	ExtSoundHeader		theHeader;
	Boolean			doConversion = false;
	
	extHeaderPtr = (ExtSoundHeaderPtr) *srcHandle;
	
	//D�finit Input Format
	inputFormat.flags = 0;
	if(extHeaderPtr->encode == extSH) {
		if(extHeaderPtr->sampleSize == 16)
		inputFormat.format = kTwosComplement;
		else
		inputFormat.format = kOffsetBinary;
		inputFormat.numChannels = extHeaderPtr->numChannels;
		inputFormat.sampleSize = extHeaderPtr->sampleSize;
	}
	else {
		inputFormat.format = kOffsetBinary;
		inputFormat.numChannels = 1;
		inputFormat.sampleSize = 8;
	}
	inputFormat.sampleRate = extHeaderPtr->sampleRate;
	inputFormat.sampleCount = 0;
	inputFormat.buffer = nil;
	inputFormat.reserved = 0;
	
	//Pas de conversion:
	if((inputFormat.sampleSize == sampleSize || sampleSize == 0)
		&& (inputFormat.sampleRate == sampleRate || sampleRate == 0)
		&& (inputFormat.numChannels == numChannels || numChannels == 0)) {
		*destHandle = NewHandle(GetHandleSize(srcHandle));
		if(!(*destHandle))
		return -108;
		HLock(*destHandle);
		BlockMove(*srcHandle, **destHandle, GetHandleSize(srcHandle));
		HUnlock(*destHandle);
		
		return 0;
	}
	
	//Definit Output Format
	outputFormat.flags = 0;
	switch(sampleSize) {
		case 8:
		outputFormat.format = kOffsetBinary;
		outputFormat.sampleSize = 8;
		break;
		
		case 16:
		outputFormat.format = kTwosComplement;
		outputFormat.sampleSize = 16;
		break;
		
		default:
		outputFormat.format = inputFormat.format;
		outputFormat.sampleSize = inputFormat.sampleSize;
		sampleSize = inputFormat.sampleSize;
		break;
	}
	if(sampleRate)
	outputFormat.sampleRate = sampleRate;
	else {
		outputFormat.sampleRate = inputFormat.sampleRate;
		sampleRate = inputFormat.sampleRate;
	}
	if(numChannels)
	outputFormat.numChannels = numChannels;
	else {
		outputFormat.numChannels = inputFormat.numChannels;
		numChannels = inputFormat.numChannels;
	}
	outputFormat.sampleCount = 0;
	outputFormat.buffer = nil;
	outputFormat.reserved = 0;
	
	//Cr�e le Header
	BlockMove(extHeaderPtr, &theHeader, kExtSHSize);
	theHeader.samplePtr = nil;
	if(extHeaderPtr->encode == stdSH) {
		theHeader.numChannels = 1;
		//theHeader.AIFFSampleRate = 0;
		theHeader.markerChunk = nil;
		theHeader.instrumentChunks = nil;
		theHeader.AESRecording = nil;
		theHeader.futureUse1 = 0;
		theHeader.futureUse2 = 0;
		theHeader.futureUse3 = 0;
		theHeader.futureUse4 = 0;
	}
	theHeader.sampleSize = sampleSize;
	theHeader.sampleRate = sampleRate;
	theHeader.numChannels = numChannels;
	
	if(extHeaderPtr->encode == stdSH)
	soundSize = GetHandleSize(srcHandle) - kStdSHSize;
	else
	soundSize = GetHandleSize(srcHandle) - kExtSHSize;
	
	//Pr�pare la conversion
	err = SoundConverterOpen(&inputFormat, &outputFormat, &sc);
	if(err)
	Do_Error(err, 511);
	err = SoundConverterGetBufferSizes(sc, soundSize, &inputFrames, &inputBytes, &outputBytes);
	if(err)
	Do_Error(err, 511);
	
	//Cr�e destHandle
	*destHandle = NewHandle(outputBytes + kExtSHSize);
	if(!(*destHandle))
	return -108;
	HLock(*destHandle);
	BlockMove(&theHeader, **destHandle, kExtSHSize);
	inputPtr = *srcHandle + kExtSHSize;
	outputPtr = **destHandle + kExtSHSize;
	extHeaderPtr = (ExtSoundHeaderPtr) **destHandle;
	
	//Convertit
	err = SoundConverterBeginConversion(sc);
	if(err)
	Do_Error(err, 511);
	err = SoundConverterConvertBuffer(sc, inputPtr, inputFrames, outputPtr, &outputFrames, &outputBytes);
	if(err)
	Do_Error(err, 511);
	soundFrames = outputFrames;
	err = SoundConverterEndConversion(sc, outputPtr,&outputFrames, &outputBytes);
	if(err)
	Do_Error(err, 511);
	soundFrames += outputFrames;
	extHeaderPtr->numFrames = soundFrames; //Danger
	extHeaderPtr->loopStart = 0;
	if(extHeaderPtr->loopEnd)
	extHeaderPtr->loopEnd = soundFrames;
	
	//Fin
	err = SoundConverterClose(sc);
	if(err)
	Do_Error(err, 511);
	HUnlock(*destHandle);
	
	return 0;
}
	
void Install_Resource(Handle theResource, ResType resType, short resID, Str255 resName)
{
	AddResource(theResource, resType, resID, resName);
	if(ResError())
	Do_Error(ResError(), 505);
	else {
		WriteResource(theResource);
		if(ResError())
		Do_Error(ResError(), 506);
	}
	DetachResource(theResource);
}
	
short Compile_Music()
{
	Str255				defaultName,
						DLGText;
	StandardFileReply	theReply;
	short				fileNumber,
						numberNotes = nbNotes;
	long				bytesNumber;
	Boolean			loop = true,
						convert = false,
						addPlayer = false;
	Handle				convertedHandle;
	Ptr					writePtr;
	OSType				compType = 'NONE';
	playerPrefs			thePlayerPrefs;
	long				tempValue;
	short				size,
						chan;
	UnsignedFixed		rate;
	
#if demo
	Do_Error(-30008, 904);
	return false;
#else	
	//ANALYSE LA MUSIQUE:
	for(i = 0; i < nbNotes; ++i)
	if(tempRec[i].action == kSPrev || tempRec[i].action == kSNext) {
		Do_Error(-30006, 408);
		return false;
	}
	for(i = 0; i < thePrefs.nbSamples; ++i)
	if(samples[i].type == directToDiskType) {
		Do_Error(-30006, 408);
		return false;
	}
	
	//OPTIONS:
	theDialog = GetNewDialog(9000, nil, (WindowPtr) -1);
	GetDItem(theDialog, 3, nil, &dialogItem[0], &aRect);
	GetDItem(theDialog, 4, nil, &dialogItem[1], &aRect);
	GetDItem(theDialog, 5, nil, &dialogItem[2], &aRect);
	GetDItem(theDialog, 6, nil, &dialogItem[3], &aRect);
	GetDItem(theDialog, 7, nil, &dialogItem[4], &aRect);
	GetDItem(theDialog, 8, nil, &dialogItem[5], &aRect);
	GetDItem(theDialog, 9, nil, &dialogItem[6], &aRect);
	GetDItem(theDialog, 10, nil, &dialogItem[7], &aRect);
	GetDItem(theDialog, 11, nil, &dialogItem[8], &aRect);
	
	if(!HasSoundManager3_2()) {
		HiliteControl((ControlHandle) dialogItem[0], 255);
		HiliteControl((ControlHandle) dialogItem[1], 255);
		HiliteControl((ControlHandle) dialogItem[2], 255);
	}
	
	tempValue = -1; //Poly smpl
	for(i = 0; i < thePrefs.nbSamples; ++i)
	if(samples[i].fileID)
	tempValue = i;
	NumToString(tempValue + 1, theString);
	SetIText(dialogItem[4], theString);
	tempValue = 0; //Poly syth
	for(i = 0; i < maxNbInstruments; ++i)
	if(instruments[i].fileID)
	++tempValue;
	if(tempValue > 4)
	tempValue = 4;
	NumToString(tempValue, theString);
	SetIText(dialogItem[5], theString);
	tempValue = 0; //Poly RB
	for(i = 0; i < kPatternParts; ++i)
	if(rythms[i].fileID)
	tempValue += 2;
	if(tempValue > 6)
	tempValue = 6;
	NumToString(tempValue, theString);
	SetIText(dialogItem[6], theString);
	NumToString(100, theString); //Volume
	SetIText(dialogItem[7], theString);
	
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	OutLine_Item(theDialog, 17);
	OutLine_Item(theDialog, 19);
	SelIText(theDialog, 7, 0, 32767); //Misc
	SetDialogDefaultItem(theDialog, 1);
	
	while(loop) {
		ModalDialog(nil, &itemHit);
		switch(itemHit) {
			case 1:
			case 2:
			loop = false;
			break;
			
			case 6:
			i = GetCtlValue((ControlHandle) dialogItem[3]);
			SetCtlValue((ControlHandle) dialogItem[3], !i);
			break;
			
			case 11:
			i = GetCtlValue((ControlHandle) dialogItem[8]);
			SetCtlValue((ControlHandle) dialogItem[8], !i);
			break;
			
			case 26:
			HMSetBalloons(!HMGetBalloons());
			if(!HMGetBalloons()) {
				OutLine_Item(theDialog, 17);
				OutLine_Item(theDialog, 19);
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
	
	tempValue = GetCtlValue((ControlHandle) dialogItem[0]); //Format
	switch(tempValue) {
		case 1: size = 0; break;
		case 2: size = 8; break;
		case 3: size = 16; break;
	}
	tempValue = GetCtlValue((ControlHandle) dialogItem[1]); //Rate
	switch(tempValue) {
		case 1: rate = 0; break;
		case 2: rate = rate11025hz; break;
		case 3: rate = rate22050hz; break;
		case 4: rate = rate44khz; break;
	}
	tempValue = GetCtlValue((ControlHandle) dialogItem[2]); //Channels
	switch(tempValue) {
		case 1: chan = 0; break;
		case 2: chan = 1; break;
		case 3: chan = 2; break;
	}
	if(size + rate + chan)
	convert = true;
	
	if(GetCtlValue((ControlHandle) dialogItem[3])) {
		addPlayer = true;
		GetIText(dialogItem[4], theString); //Poly smpl
		StringToNum(theString, &tempValue);
		if(tempValue < 0) tempValue = 0;
		if(tempValue > 30) tempValue = 30;
		thePlayerPrefs.smpPoly = tempValue;
		GetIText(dialogItem[5], theString); //Poly syth
		StringToNum(theString, &tempValue);
		if(tempValue < 0) tempValue = 0;
		if(tempValue > 10) tempValue = 10;
		thePlayerPrefs.sytPoly = tempValue;
		GetIText(dialogItem[6], theString); //Poly RB
		StringToNum(theString, &tempValue);
		if(tempValue < 0) tempValue = 0;
		if(tempValue > 10) tempValue = 10;
		thePlayerPrefs.rytPoly = tempValue;
		GetIText(dialogItem[7], theString); //Volume
		StringToNum(theString, &tempValue);
		if(tempValue < 0) tempValue = 0;
		if(tempValue > 256) tempValue = 256;
		thePlayerPrefs.volume = tempValue;
		if(GetCtlValue((ControlHandle) dialogItem[8])) //Autoplay
		thePlayerPrefs.flags = kAutoPlay;
	}
	DisposeDialog(theDialog);
	for(i = 0; i < 20; ++i)
	DisposeHandle(dialogItem[i]);
	
	//O� sauver?
	UseResFile(mainResFile);
	GetIndString(defaultName, 0, 3);
	GetIndString(DLGText, 0, 4);
	CustomPutFile(DLGText, defaultName, &theReply, 30000, whereToShow, nil,
		nil, nil, nil, nil);
	if(!theReply.sfGood)
	return false;
	
	//CREE FICHIER
	if(theReply.sfReplacing) { //Efface le fichier si existe d�j�
		theError = FSpDelete(&theReply.sfFile);
		if(theError) {
			Do_Error(theError, 104);
			return false;
		}
	}
	if(addPlayer) //Cr�e le fichier
	FSpCreateResFile(&theReply.sfFile, 'HFPl', 'APPL', theReply.sfScript);
	else
	FSpCreateResFile(&theReply.sfFile, 'HiFc', 'CMus', theReply.sfScript);
	if(ResError()) {
		Do_Error(ResError(), 101);
		return false;
	}
	
	Anim_WaitCursor();
	
	//COPIE LE PLAYER:
	if(addPlayer) {
		UseResFile(mainResFile);
		convertedHandle = Get1Resource('Plyr', 128);
		if(!convertedHandle) {
			Do_Error(ResError(), 513);
			return false;
		}
		DetachResource(convertedHandle);
		HLock(convertedHandle);
		bytesNumber = GetHandleSize(convertedHandle);
		theError = FSpOpenDF(&theReply.sfFile, fsRdWrPerm, &fileNumber);
		if(theError) {
			Do_Error(theError, 102);
			return false;
		}
		SetFPos(fileNumber, 1, 0);
		FSWrite(fileNumber, &bytesNumber, *convertedHandle);
		if(bytesNumber != GetHandleSize(convertedHandle)) {
			Do_Error(theError, 103);
			return false;
		}
		SetEOF(fileNumber, bytesNumber);
		FSClose(fileNumber);
		HUnlock(convertedHandle);
		DisposeHandle(convertedHandle);
		
		UseResFile(mainResFile);
		convertedHandle = Get1Resource('Plyr', 129);
		if(!convertedHandle) {
			Do_Error(ResError(), 513);
			return false;
		}
		DetachResource(convertedHandle);
		HLock(convertedHandle);
		bytesNumber = GetHandleSize(convertedHandle);
		theError = FSpOpenRF(&theReply.sfFile, fsRdWrPerm, &fileNumber);
		if(theError) {
			Do_Error(theError, 102);
			return false;
		}
		SetFPos(fileNumber, 1, 0);
		FSWrite(fileNumber, &bytesNumber, *convertedHandle);
		if(bytesNumber != GetHandleSize(convertedHandle)) {
			Do_Error(theError, 103);
			return false;
		}
		SetEOF(fileNumber, bytesNumber);
		FSClose(fileNumber);
		HUnlock(convertedHandle);
		ReleaseResource(convertedHandle);
		
		FlushVol(nil, theReply.sfFile.vRefNum);
	}
	
	/*theError = FSpOpenDF(&theReply.sfFile, fsRdWrPerm, &fileNumber); //Ouvre le fichier
	if(theError) {
		Do_Error(theError, 102);
		return false;
	}
	SetFPos(fileNumber, 1, 0);
	
	//Ecrit le num�ro de version de l'enregistrement
	bytesNumber = 2;
	FSWrite(fileNumber, &bytesNumber, &cmusVersion);
	//Ecrit le nombre de notes de l'enregistrement
	bytesNumber = 2;
	FSWrite(fileNumber, &bytesNumber, &numberNotes);
	//Ecrit l'enregistrement
	bytesNumber = sizeof(tempRec);
	FSWrite(fileNumber, &bytesNumber, &tempRec);
	//Ecrit le num�ro de version des pr�f�rences
	bytesNumber = 2;
	FSWrite(fileNumber, &bytesNumber, &prefVersion);
	//Ecrit les pr�f�rences
	for(i = 0; i < thePrefs.nbSamples; ++i)
	recordPrefs.SSndNb[i] = 1;
	bytesNumber = sizeof(thePrefs);
	FSWrite(fileNumber, &bytesNumber, &recordPrefs);
	//Ecrit les patterns de la bo�te � rythmes
	bytesNumber = sizeof(pattern);
	FSWrite(fileNumber, &bytesNumber, pattern);
	//Ecrit o� sont les sons:
	bytesNumber = sizeof(Boolean);
	FSWrite(fileNumber, &bytesNumber, &soundsInRF);
	
	FSClose(fileNumber);*/
	
	//OUVRE LE FICHIER:
	fileNumber = FSpOpenResFile(&theReply.sfFile, fsRdWrPerm);
	if(fileNumber == -1) {
		Do_Error(ResError(), 102);
		return false;
	}
	UseResFile(fileNumber);

	//AJOUTE LES PREFS DU PLAYER:
	if(addPlayer) {
		convertedHandle = NewHandle(sizeof(playerPrefs));
		HLock(convertedHandle);
		BlockMove(&thePlayerPrefs, *convertedHandle, sizeof(playerPrefs));
		Install_Resource(convertedHandle, 'Pref', 128, "\pPlayer options");
		HUnlock(convertedHandle);
		DisposeHandle(convertedHandle);
	}
	
	//ECRIT LA MUSIQUE:
	convertedHandle = NewHandle(2 + 2 + sizeof(tempRec) + 2 + sizeof(thePrefs) + sizeof(pattern));
	HLock(convertedHandle);
	writePtr = *convertedHandle;
	
	//Ecrit le num�ro de version de l'enregistrement
	BlockMove(&cmusVersion, writePtr, 2);
	writePtr += 2;
	//Ecrit le nombre de notes de l'enregistrement
	BlockMove(&numberNotes, writePtr, 2);
	writePtr += 2;
	//Ecrit l'enregistrement
	BlockMove(&tempRec, writePtr, sizeof(tempRec));
	writePtr += sizeof(tempRec);
	//Note la compression des sons
	BlockMove(&compType, writePtr, 4);
	writePtr += 4;
	//Ecrit le num�ro de version des pr�f�rences
	BlockMove(&prefVersion, writePtr, 2);
	writePtr += 2;
	//Ecrit les pr�f�rences
	for(i = 0; i < thePrefs.nbSamples; ++i)
	recordPrefs.SSndNb[i] = 1;
	BlockMove(&recordPrefs, writePtr, sizeof(thePrefs));
	writePtr += sizeof(thePrefs);
	//Ecrit les patterns de la bo�te � rythmes
	BlockMove(pattern, writePtr, sizeof(pattern));
	writePtr += sizeof(pattern);
	
	Install_Resource(convertedHandle, 'Msic', 128, "\pMusic score");
	HUnlock(convertedHandle);
	DisposeHandle(convertedHandle);
	
	//COPIE LES SONS:
	for(i = 0; i < thePrefs.nbSamples; ++i) //Sampler
	if(samples[i].fileID) {
		Anim_WaitCursor();
		if(convert) {
			Convert_Sound(samples[i].snd, &convertedHandle, size, rate, chan);
			Install_Resource(convertedHandle, 'Smpl', i, samples[i].name);
			DisposeHandle(convertedHandle);
		}
		else
		Install_Resource(samples[i].snd, 'Smpl', i, samples[i].name);
	}
	for(i = 0; i < maxNbInstruments; ++i) //Synthetizer
	if(instruments[i].fileID) {
		Anim_WaitCursor();
		if(convert) {
			Convert_Sound(instruments[i].snd, &convertedHandle, size, rate, chan);
			Install_Resource(convertedHandle, 'Syth', i, instruments[i].name);
			DisposeHandle(convertedHandle);
		}
		else
		Install_Resource(instruments[i].snd, 'Syth', i, instruments[i].name);
	}
	for(i = 0; i < kPatternParts; ++i) //RBox
	if(rythms[i].fileID) {
		Anim_WaitCursor();
		if(convert) {
			Convert_Sound(rythms[i].snd, &convertedHandle, size, rate, chan);
			Install_Resource(convertedHandle, 'RBox', i, rythms[i].name);
			DisposeHandle(convertedHandle);
		}
		else
		Install_Resource(rythms[i].snd, 'RBox', i, rythms[i].name);
	}
	
	CloseResFile(fileNumber);
	
	UseResFile(mainResFile);
	FlushVol(nil, musicFileSpec.vRefNum);
	SetCCursor(mouseCursor);
	
	if(isCMusFile) //New!
	saved = true;
	
	return true;
#endif
}

short Decompile_Music(FSSpec* musicSpec)
{
	short				fileNumber,
						temp;
	ResType			type;
	long				bytesNumber,
						k;
	SoundHeaderPtr		stdHeader;
	Handle				convertedHandle;
	Ptr					readPtr;
	OSType				compType;
	
	Anim_WaitCursor();
	
	/*theError = FSpOpenDF(musicSpec, fsRdWrPerm, &fileNumber);
	if(theError) {
		Do_Error(theError, 102);
		return false;
	}
	Clear_Music();
	recorded = false;
	
	SetFPos(fileNumber, 1, 0);
	//Contr�le le num�ro de version
	bytesNumber = 2;
	FSRead(fileNumber, &bytesNumber, &temp);
	if(temp != cmusVersion) {
		Do_Error(-30000, 401);
		FSClose(fileNumber);
		return false;
	}
	
	//Contr�le le nombre de notes
	bytesNumber = 2;
	FSRead(fileNumber, &bytesNumber, &temp);	
	//Charge l'enregistrement en m�moire
	bytesNumber = temp * sizeof(record);
	FSRead(fileNumber, &bytesNumber, &tempRec);
	//Contr�le la version des pr�f�rences
	bytesNumber = 2;
	FSRead(fileNumber, &bytesNumber, &temp);
	//Charge les pr�f�rences en m�moire
	bytesNumber = sizeof(thePrefs);
	FSRead(fileNumber, &bytesNumber, &recordPrefs);
	//Charge les patterns de la bo�te � rythmes
	bytesNumber = sizeof(pattern);
	FSRead(fileNumber, &bytesNumber, pattern);
	UseResFile(mainResFile);
	GetIndString(patternName, 0, 15);
	//D�termine o� sont les sons
	bytesNumber = sizeof(Boolean);
	FSRead(fileNumber, &bytesNumber, &soundsInRF);
	//Ferme le fichier
	FSClose(fileNumber);*/
	
	//Ferme tous les fichiers sons
	GetIndString(theString, 0, 11);
	theCell.h = 0;
	for(i = 0; i < thePrefs.nbSamples; ++i)
	SF_CloseSample(i);
	for(i = 0; i < maxNbInstruments; ++i) {
		SF_CloseInstrument(i);
		theCell.v = i;
		LSetCell(&theString[1], theString[0], theCell, instrumentsList);
	}
	for(i = 0; i < kPatternParts; ++i)
	RB_CloseSound(i);
	
	//Ouvre le fichier
	fileNumber = FSpOpenResFile(musicSpec, fsRdWrPerm);
	if(fileNumber == -1) {
		Do_Error(ResError(), 102);
		return false;
	}
	UseResFile(fileNumber);
	
	//CHARGE LA PARTITION:
	convertedHandle = Get1IndResource('Msic', 1);
	if(!convertedHandle) {
		Do_Error(theError, 512);
		return false;
	}
	DetachResource(convertedHandle);
	HLock(convertedHandle);
	readPtr = *convertedHandle;
	Clear_Music();
	recorded = false;
	
	//Contr�le le num�ro de version
	temp = *((short*) readPtr);
	if(temp != cmusVersion) {
		Do_Error(-30000, 401);
		CloseResFile(fileNumber);
		return false;
	}
	readPtr += 2;
	//Contr�le le nombre de notes
	temp = *((short*) readPtr);
	readPtr += 2;
	//Charge l'enregistrement en m�moire
	BlockMove(readPtr, &tempRec, temp * sizeof(record));
	readPtr += temp * sizeof(record);
	//Note la compression des sons
	BlockMove(readPtr, &compType, 4);
	readPtr += 4;
	//Contr�le la version des pr�f�rences
	temp = *((short*) readPtr);
	readPtr += 2;
	//Charge les pr�f�rences en m�moire
	BlockMove(readPtr, &recordPrefs, sizeof(thePrefs));
	readPtr += sizeof(thePrefs);
	//Charge les patterns de la bo�te � rythmes
	BlockMove(readPtr, pattern, sizeof(pattern));
	readPtr += sizeof(pattern);
	UseResFile(mainResFile);
	GetIndString(patternName, 0, 15);
	UseResFile(fileNumber);
	
	HUnlock(convertedHandle);
	DisposeHandle(convertedHandle);

	//CHARGE LES SONS:
	//Sampler
	for(i = 0; i < thePrefs.nbSamples; ++i) {
		samples[i].snd = Get1Resource('Smpl', i);
		if(samples[i].snd) {
			Anim_WaitCursor();
			GetResInfo(samples[i].snd, &temp, &type, samples[i].name);
			DetachResource(samples[i].snd);
			thePrefs.SRelativeRate[i] = 0;
			HLock(samples[i].snd);
			
			stdHeader = (SoundHeaderPtr) *samples[i].snd;
			thePrefs.SRate[i] = UnsignedFixedMulDiv(stdHeader->sampleRate, 0x00010000, rate22khz);
			
			samples[i].fileID = -1;
			samples[i].type = sampleType;
			samples[i].sndNb = 1;
			samples[i].totalSndNb = 1;
			
			samples[i].spec.vRefNum = 0;
			samples[i].spec.parID = 0;
			samples[i].spec.name[0] = 0;
			
			theCommand.cmd		= soundCmd; //Joue le son
			theCommand.param1	= 0;
			theCommand.param2	= (long) *samples[i].snd;
			SndDoImmediate(samples[i].chan, &theCommand);
		}
	}
	
	//Synthetizer
	theCell.h = 0;
	for(i = 0; i < maxNbInstruments; ++i) {
		instruments[i].snd = Get1Resource('Syth', i);
		if(instruments[i].snd) {
			Anim_WaitCursor();
			GetResInfo(instruments[i].snd, &temp, &type, instruments[i].name);
			DetachResource(instruments[i].snd);
			HLock(instruments[i].snd);
			
			instruments[i].fileID = -1;
			
			instruments[i].spec.vRefNum = 0;
			instruments[i].spec.parID = 0;
			instruments[i].spec.name[0] = 0;
			
			theCell.v = i;
			LSetCell(&instruments[i].name[1], instruments[i].name[0], theCell, instrumentsList);
			
			theCommand.cmd		= soundCmd; //Joue le son
			theCommand.param1	= 0;
			theCommand.param2	= (long) *instruments[i].snd;
			for(k = 0; k < thePrefs.nbInsChannels; ++k)
			SndDoImmediate(insChannels[k], &theCommand);
		}
	}
	theCell.v = activeInstrument;
	LSetSelect(false, theCell, instrumentsList);
	for(i = 0; i < maxNbInstruments; ++i)
	if(instruments[i].fileID) {
		activeInstrument = i;
		break;
	}
	theCell.v = activeInstrument;
	LSetSelect(true, theCell, instrumentsList);
	
	//RBox
	for(i = 0; i < kPatternParts; ++i) {
		rythms[i].snd = Get1Resource('RBox', i);
		if(rythms[i].snd) {
			Anim_WaitCursor();
			GetResInfo(rythms[i].snd, &temp, &type, rythms[i].name);
			DetachResource(rythms[i].snd);
			HLock(rythms[i].snd);
			
			rythms[i].fileID = -1;
			
			rythms[i].spec.vRefNum = 0;
			rythms[i].spec.parID = 0;
			rythms[i].spec.name[0] = 0;
			
			theCommand.cmd		= soundCmd; //Joue le son
			theCommand.param1	= 0;
			theCommand.param2	= (long) *rythms[i].snd;
			for(k = 0; k < thePrefs.nbRytChannels; ++k)
			SndDoImmediate(rytChannels[k], &theCommand);
		}
	}
	
	CloseResFile(fileNumber);
	UseResFile(mainResFile);
	
	//Installe la musique
	Install_Music();
	
	if(thePrefs.nbSamples < recordPrefs.nbSamples) { //Contr�le du nombre de pistes du sampler
		Do_Error(-30000, 402);
		return false;
	}
	
	//Tag l'enregistrement 
	recorded = true;
	saved = true;
	isCMusFile = true;
	for(i = 0; i < editorPlugInsNumber; ++i)
	editorPlugIns[i].update = true;
	DisableItem(menu[1], 4);
	EnableItem(menu[1], 5);
	EnableItem(menu[1], 6);
	
	//Mise � jour des fen�tres
	SetGWorld((CGrafPtr) mainWin, nil);
	InvalRect(&shellRect);
	if(thePrefs.instrumentsWin) {
		SetGWorld((CGrafPtr) instrumentsWin, nil);
		InvalRect(&instrumentsRect);
	}
	if(thePrefs.recordWin) {
		SetGWorld((CGrafPtr) recordWin, nil);
		InvalRect(&recordRect);
	}
	if(thePrefs.rythmsWin) {
		SetGWorld((CGrafPtr) rythmsWin, nil);
		InvalRect(&rythmsRect);
	}
	SetCCursor(mouseCursor);
	
	return true;
}