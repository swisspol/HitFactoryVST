#include			<Aliases.h>

#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"
#include			"Record Codes.h"

#define				kCreateFileIDs		true
#if(kCreateFileIDs)
extern HParamBlockRec		theBlock;
#endif

// FONCTIONS:
	
void Clear_Music() //Efface l'enregistrement
{
	for(i = 0; i < nbNotes; ++i) {
		tempRec[i].time = 0;
		tempRec[i].action = 0;
		tempRec[i].command = 0;
		tempRec[i].command_2 = 0;
	}
}

short Save_Music()
{
	Str255				defaultName,
						DLGText;
	StandardFileReply	theReply;
	
#if demo
	Do_Error(-30008, 904);
	return false;
#else
	if(isCMusFile) {
		Do_Error(-30006, 409);
		return false;
	}
			
	//Où sauver?
	UseResFile(mainResFile);
	GetIndString(defaultName, 0, 3);
	GetIndString(DLGText, 0, 4);
	CustomPutFile(DLGText, defaultName, &theReply, 30000, whereToShow, nil,
		nil, nil, nil, nil);
	if(!theReply.sfGood)
	return false;
	
	if(theReply.sfReplacing) { //Efface le fichier si existe déjà
		theError = FSpDelete(&theReply.sfFile);
		if(theError) {
			Do_Error(theError, 104);
			return false;
		}
	}
	theError = FSpCreate(&theReply.sfFile, 'HiFc', 'Msic', theReply.sfScript); //Crée le fichier
	if(theError) {
		Do_Error(theError, 101);
		return false;
	}
	
	//Remplit la structure du fichier par défaut
	musicFileSpec.vRefNum = theReply.sfFile.vRefNum;
	musicFileSpec.parID = theReply.sfFile.parID;
	BlockMove(theReply.sfFile.name, musicFileSpec.name, 63);
	
	Write_Music();
	
	return true;
#endif
}

short Write_Music()
{
	short				fileNumber,
						numberNotes = nbNotes;
	long				bytesNumber;
						
#if demo
	Do_Error(-30008, 904);
	return false;
#else
	theError = FSpOpenDF(&musicFileSpec, fsRdWrPerm, &fileNumber); //Ouvre le fichier
	if(theError) {
		Do_Error(theError, 102);
		return false;
	}
	
	SetFPos(fileNumber, 1, 0);
	//Ecrit le numéro de version de l'enregistrement
	bytesNumber = 2;
	FSWrite(fileNumber, &bytesNumber, &msicVersion);
	//Ecrit le nombre de notes de l'enregistrement
	bytesNumber = 2;
	FSWrite(fileNumber, &bytesNumber, &numberNotes);
	//Ecrit l'enregistrement
	bytesNumber = sizeof(tempRec);
	FSWrite(fileNumber, &bytesNumber, &tempRec);
	//Ecrit le numéro de version des préférences
	bytesNumber = 2;
	FSWrite(fileNumber, &bytesNumber, &prefVersion);
	//Ecrit les préférences
	bytesNumber = sizeof(thePrefs);
	FSWrite(fileNumber, &bytesNumber, &recordPrefs);
	//Ecrit les références des fichiers sons de l'enregistrement
	Write_Files(fileNumber);
	//Ecrit les données de la boîte à rythmes
	RB_WritePatternSet(fileNumber);
	
	FSClose(fileNumber);
	FlushVol(nil, musicFileSpec.vRefNum);
	
	//Detag l'enregistrement
	saved = true;
	DisableItem(menu[1], 4);
	
	return true;
#endif
}
short Open_Music(FSSpec* musicSpec)
{
	short				fileNumber,
						temp;
	long				bytesNumber;
	
	//Ouvre le fichier
	theError = FSpOpenDF(musicSpec, fsRdWrPerm, &fileNumber);
	if(theError) {
		Do_Error(theError, 102);
		return false;
	}
	Clear_Music();
	recorded = false;
	
	//Remplit la structure du fichier par défaut
	musicFileSpec.vRefNum = musicSpec->vRefNum;
	musicFileSpec.parID = musicSpec->parID;
	BlockMove(musicSpec->name, musicFileSpec.name, sizeof(Str63));
	
	SetFPos(fileNumber, 1, 0);
	//Contrôle le numéro de version
	bytesNumber = 2;
	FSRead(fileNumber, &bytesNumber, &temp);
	if(temp != msicVersion) {
		Do_Error(-30000, 401);
		FSClose(fileNumber);
		return false;
	}
	
	Anim_WaitCursor();
	
	//Contrôle le nombre de notes
	bytesNumber = 2;
	FSRead(fileNumber, &bytesNumber, &temp);
	if(temp > nbNotes) {
		Do_Error(-30002, 403); //Trop de notes donc fichier tronqué
		bytesNumber = nbNotes * sizeof(record);
	}
	else
		bytesNumber = temp * sizeof(record);
	//Charge l'enregistrement en mémoire
	FSRead(fileNumber, &bytesNumber, &tempRec);
	if(temp > nbNotes) { //Si fichier tronqué, rajoute command de fin d'enregistrement
		tempRec[nbNotes - 1].action = kEnd;
		SetFPos(fileNumber, 3, (temp - nbNotes) * sizeof(record));
	}
	//Contrôle la version des préférences
	bytesNumber = 2;
	FSRead(fileNumber, &bytesNumber, &temp);
	//Charge les préférences en mémoire
	bytesNumber = sizeof(thePrefs);
	FSRead(fileNumber, &bytesNumber, &recordPrefs);
	//Charge les fichiers
	Read_Files(fileNumber);
	//Charge les données de la boîte à rythmes
	RB_ReadPatternSet(fileNumber);
	UseResFile(mainResFile);
	GetIndString(patternName, 0, 15);
	//Ferme le fichier
	FSClose(fileNumber);
	//Installe la musique
	Install_Music();
	
	//Tag l'enregistrement 
	recorded = true;
	saved = true;
	isCMusFile = false;
	for(i = 0; i < editorPlugInsNumber; ++i)
	editorPlugIns[i].update = true;
	DisableItem(menu[1], 4);
	EnableItem(menu[1], 5);
	EnableItem(menu[1], 6);
	
	if(thePrefs.nbSamples < recordPrefs.nbSamples) { //Contrôle du nombre de pistes du sampler
		Do_Error(-30000, 402);
		recorded = false;
	}
	
	//Mise à jour des fenêtres
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

void Play_Music(long whichPosition)
{
	switch(tempRec[whichPosition].action) {
		
		case kSStart:
		if(samples[tempRec[whichPosition].command].type == directToDiskType)
		SCD_StartDirectToDisk(tempRec[whichPosition].command);
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1) {	
			SetRect(&sourceRect, 451, 100, 491, 118);
			SetRect(&destRect, 39 + 80 * tempValue, 251, 79 + 80 * tempValue, 269);
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			ForeColor(yellowColor);
			RGBBackColor(&theBackColor);
		}
		break;
					
		case kSStop:
		if(samples[tempRec[whichPosition].command].type == directToDiskType) {
			SCD_StopDirectToDisk(tempRec[whichPosition].command);
			chanFinished = nil;
		}
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1) {
			SetRect(&sourceRect, 1, 251, 79, 269);
			SetRect(&destRect, 1 + 80 * tempValue, 251, 79 + 80 * tempValue, 269);
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			ForeColor(yellowColor);
			RGBBackColor(&theBackColor);
		}
		break;
					
		case kSNext:
		SetGWorld((CGrafPtr) mainWin, nil);
		if(thePrefs.flags & kNoInterruptBank)
			SF_LoadNextSample(tempRec[whichPosition].command);
		else
			while(StringWidth(samples[tempRec[whichPosition].command].name) > maxWidthSmpl)
			--samples[tempRec[whichPosition].command].name[0];
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1)
		Draw_Name(tempValue);
		break;
					
		case kSPrev:
		SetGWorld((CGrafPtr) mainWin, nil);
		if(thePrefs.flags & kNoInterruptBank)
			SF_LoadPrevSample(tempRec[whichPosition].command);
		else
			while(StringWidth(samples[tempRec[whichPosition].command].name) > maxWidthSmpl)
			--samples[tempRec[whichPosition].command].name[0];
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1)
		Draw_Name(tempValue);
		break;
					
		case kSVolume:
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1) {
			SetRect(&sourceRect, 25, 82, 55, 208);
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &sourceRect, srcCopy, nil);
			i = volPaddleStart + (thePrefs.SMaxVol[tempRec[whichPosition].command] - thePrefs.SVol[tempRec[whichPosition].command])
				/ (thePrefs.SMaxVol[tempRec[whichPosition].command] / volPaddleLength);
			SetRect(&sourceRect, 451, 120, 471, 160);
			SetRect(&destRect, 30, i - 20, 50, i + 20);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &destRect, srcCopy, nil);
			SetRect(&sourceRect, 25, 82, 55, 208);
			SetRect(&destRect, 25 + 80 * tempValue, 82, 55 + 80 * tempValue, 208);
			CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			RGBBackColor(&theBackColor);
		}
		break;
					
		case kSPitch:
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1) {
			SetRect(&sourceRect, 0, 213, 80, 229);
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &sourceRect, srcCopy, nil);
			i = 40 + thePrefs.SRelativeRate[tempRec[whichPosition].command];
			SetRect(&sourceRect, 472, 120, 486, 136);
			SetRect(&destRect, i - 7, 213, i + 7, 229);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &destRect, srcCopy, nil);
			SetRect(&sourceRect, 0, 213, 80, 229);
			SetRect(&destRect, 80 * tempValue, 213, 80 + 80 * tempValue, 229);
			CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			//Draw_Pitch(tempValue);
			RGBBackColor(&theBackColor);
		}
		break;
					
		case kSPano:
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1) {
			SetRect(&sourceRect, 0, 231, 80, 247);
			SetRect(&destRect, 80 * tempValue, 231, 80 + 80 * tempValue, 247);
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &sourceRect, srcCopy, nil);
			i = 15 + (200 - thePrefs.SPan[tempRec[whichPosition].command]) / 4;
			SetRect(&sourceRect, 472, 120, 486, 136);
			SetRect(&destRect, i - 7, 231, i + 7, 247);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &destRect, srcCopy, nil);
			SetRect(&sourceRect, 0, 231, 80, 247);
			SetRect(&destRect, 80 * tempValue, 231, 80 + 80 * tempValue, 247);
			CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			//Draw_Panoramic(tempValue);
			RGBBackColor(&theBackColor);
		}
		break;
		
		//case kIPlay:
		//;
		//break;
					
		case kIVolume:
		if(thePrefs.instrumentsWin && thePrefs.insWinExt) {
			SetRect(&sourceRect, 162 + 230, 0 + 32, 162 + 260, 0 + 148);
			SetRect(&destRect, 230, 32, 260, 148);
			SetGWorld((CGrafPtr) instrumentsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &destRect, srcCopy, nil);
			i = insVolPaddleStart + (thePrefs.IMaxVol - thePrefs.IVol)
				/ (thePrefs.IMaxVol / insVolPaddleLength);
			SetRect(&sourceRect, 451, 120, 471, 160);
			SetRect(&destRect, 234, i - 20, 254, i + 20);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &destRect, srcCopy, nil);
			SetRect(&sourceRect, 230, 32, 260, 148);
			CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(instrumentsWin),
				&sourceRect, &sourceRect, srcCopy, nil);
			RGBBackColor(&theBackColor);
		}
		break;
					
		case kSLoop:
		if(thePrefs.SLoop[tempRec[whichPosition].command]) {
			tempValue = uc[tempRec[whichPosition].command];
			if(tempValue != -1) {
				SetRect(&sourceRect, 0, 231, 80, 247);
				SetRect(&destRect, 80 * tempValue, 231, 80 + 80 * tempValue, 247);
				SetGWorld((CGrafPtr) mainWin, nil);
				BackColor(whiteColor);
				ForeColor(blackColor);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
				SetRect(&sourceRect, 533, 134, 544, 145);
				SetRect(&destRect, 5 + 80 * tempValue, 54, 16 + 80 * tempValue, 65);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
				RGBBackColor(&theBackColor);
			}
		}
		else {
			tempValue = uc[tempRec[whichPosition].command];
			if(tempValue != -1) {
				SetGWorld((CGrafPtr) mainWin, nil);
				BackColor(whiteColor);
				ForeColor(blackColor);
				Draw_Panoramic(tempValue);
				SetRect(&sourceRect, 5, 54, 16, 65);
				SetRect(&destRect, 5 + 80 * tempValue, 54, 16 + 80 * tempValue, 65);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
				RGBBackColor(&theBackColor);
			}
		}
		break;
					
		case kSInit:
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1) {
			SetGWorld((CGrafPtr) mainWin, nil);
			SetRect(&aRect, 80 * tempValue, 0, 80 + 80 * tempValue, 270);
			InvalRect(&aRect);
		}
		break;
		
		//case kMPausRes:
		//;
		//break;
		
		case kMLoopSpeed:
		for(i = 1; i < 10; ++i)
		CheckItem(submenu[0], i, false);
		CheckItem(submenu[0], tempRec[whichPosition].command, true);
		break;
		
		case kMSynthMode:
		if(thePrefs.instrumentsWin && thePrefs.insWinExt) {
			SetGWorld((CGrafPtr) instrumentsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&destRect, 209, 6, 220, 17);
			if(thePrefs.synthMode == 2) {
				SetRect(&sourceRect, 533, 134, 544, 145);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(instrumentsWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			else {
				SetRect(&sourceRect, 162 + 209, 0 + 6, 162 + 220, 0 + 17);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(instrumentsWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			RGBBackColor(&theBackColor);
		}
		break;
		
		case kSTurbo:
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1) {
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&destRect, 5 + 80 * tempValue, 67, 16 + 80 * tempValue, 78);
			if(thePrefs.SMaxVol[tempRec[whichPosition].command] == thePrefs.STurbo) {
				SetRect(&sourceRect, 533, 112, 544, 133);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			else {
				SetRect(&sourceRect, 5, 67, 16, 78);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			RGBBackColor(&theBackColor);
		}
		break;
		
		case kSEffects:
#if defined(powerc) || defined (__powerc)
		if(!thePrefs.SReverb)
		break;
		tempValue = uc[tempRec[whichPosition].command];
		if(tempValue != -1) {
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 5, 41, 75, 52);
			SetRect(&destRect, 5 + 80 * tempValue, 41, 75 + 80 * tempValue, 52);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
			SetRect(&sourceRect, 533, 100, 544, 111);
			if(thePrefs.SSourceMode[tempRec[whichPosition].command] == kSSpSourceMode_Localized) {
				SetRect(&destRect, 5 + 80 * tempValue, 41, 16 + 80 * tempValue, 52);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			if(thePrefs.SSourceMode[tempRec[whichPosition].command] == kSSpSourceMode_Ambient) {
				SetRect(&destRect, 40 + 80 * tempValue, 41, 51 + 80 * tempValue, 52);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			RGBBackColor(&theBackColor);
		}
#endif
		break;
		
		case kITurbo:
		if(thePrefs.instrumentsWin && thePrefs.insWinExt) {
			SetGWorld((CGrafPtr) instrumentsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&destRect, 209, 19, 220, 30);
			if(thePrefs.IMaxVol == thePrefs.ITurbo) {
				SetRect(&sourceRect, 533, 112, 544, 133);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(instrumentsWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			else {
				SetRect(&sourceRect, 162 + 209, 0 + 19, 162 + 220, 0 + 30);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(instrumentsWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			RGBBackColor(&theBackColor);
		}
		break;
		
		case kRPlay:
		if(thePrefs.rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 451, 100, 490, 117);
			SetRect(&destRect, 39, 22, 78, 39);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect, &destRect, srcCopy, nil);
		}
		break;
		
		case kRStop:
		if(thePrefs.rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 611 + 39, 0 + 22, 611 + 78, 0 + 39);
			SetRect(&destRect, 39, 22, 78, 39);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect, &destRect, srcCopy, nil);
		}
		break;
		
		case kRVolume:
		if(thePrefs.rythmsWin && thePrefs.rytWinExt) {
			SetRect(&sourceRect, 611 + 262, 0 + 42, 611 + 295, 0 + 148);
			SetRect(&destRect, 262, 42, 295, 148);
			SetGWorld((CGrafPtr) rythmsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &destRect, srcCopy, nil);
			i = kRytVolPaddleStart + (thePrefs.RMaxVol - thePrefs.RVol)
					/ (thePrefs.RMaxVol / kRytVolPaddleLength);
			SetRect(&sourceRect, 451, 120, 471, 160);
			SetRect(&destRect, 269, i - 20, 289, i + 20);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &destRect, srcCopy, nil);
			SetRect(&sourceRect, 262, 42, 295, 148);
			CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect, &sourceRect, srcCopy, nil);
		}
		break;
		
		case kRBPM:
		if(thePrefs.rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			RGBBackColor(&theBackColor);
			Draw_Rythms_BPM();
		}
		break;
		
		/*case kRInstrument:
		if(rythms[tempRec[whichPosition].command].fileID)
		RB_PlaySound(tempRec[whichPosition].command);
		break;*/
		
		case kRTurbo:
		if(thePrefs.rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&destRect, 265, 27, 276, 38);
			if(thePrefs.RMaxVol == thePrefs.RTurbo) {
				SetRect(&sourceRect, 533, 112, 544, 133);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			else {
				SetRect(&sourceRect, 611 + 265, 0 + 27, 611 + 276, 0 + 38);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
		}
		break;
		
		case kRMute:
		if(thePrefs.rythmsWin && thePrefs.rytWinExt) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&destRect, 66, 47 + 10 * tempRec[whichPosition].command, 76, 
				57 + 10 * tempRec[whichPosition].command);
			if(thePrefs.RPartMute[tempRec[whichPosition].command])
			SetRect(&sourceRect, 611 + 66, 151 + 47, 611 + 76, 151 + 57);
			else
			SetRect(&sourceRect, 611 + 66, 0 + 47, 611 + 76, 0 + 57);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
					&sourceRect, &destRect, srcCopy, nil);
		}
		break;
		
		case kRNext:
		if(thePrefs.rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			RGBBackColor(&theBackColor);
			Draw_Rythms_Next();
		}
		break;
		
		case kRCurrent:
		if(thePrefs.rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			RGBBackColor(&theBackColor);
			Draw_Rythms_Current();
			BackColor(whiteColor);
			ForeColor(blackColor);
			Draw_Rythms_Pattern();
		}
		break;
		
		/*case kRPattern:
		if(thePrefs.rythmsWin && thePrefs.rytWinExt) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&destRect, 78 + 10 * tempRec[whichPosition].command_2, 47 + 10 * tempRec[whichPosition].command, 
				88 + 10 * tempRec[whichPosition].command_2, 57 + 10 * tempRec[whichPosition].command);
			if(pattern[thePrefs.RCurrent][tempRec[whichPosition].command][tempRec[whichPosition].command_2])
			SetRect(&sourceRect, 611 + 78, 151 + 47, 611 + 88, 151 + 57);
			else
			SetRect(&sourceRect, 611 + 78, 0 + 47, 611 + 88, 0 + 57);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect, &destRect, srcCopy, nil);
		}
		break;*/
		
		case kEnd:
		for(i = 0; i < thePrefs.nbSamples; ++i)
			if(thePrefs.SPlaying[i] && samples[i].type == directToDiskType)
			SCD_StopDirectToDisk(i);
		chanFinished = nil;
		
		TM_KillReadTask();
		position = 0;
		playing = false;
		
		EnableItem(menu[1], 1);
		EnableItem(menu[1], 2);
		EnableItem(menu[1], 8);
		if(musicFileSpec.parID && !saved)
		EnableItem(menu[1], 4);
		EnableItem(menu[1], 5);
		EnableItem(menu[1], 6);
		if(!(!(thePrefs.MIDIFlags & kUseHFClock) && (thePrefs.quantizeFlags & kQSmplPitchAlign)))
		EnableItem(menu[2], 3);
		EnableItem(menu[2], 5);
		EnableItem(menu[2], 8);
		EnableItem(popUpMenu, 1);
		EnableItem(popUpMenu, 2);
		EnableItem(popUpMenu, 3);
		EnableItem(popUpMenu, 5);
		EnableItem(popUpMenu2, 0);
		
		SetGWorld((CGrafPtr) mainWin, nil);
		InvalRect(&playRect);
		if(thePrefs.recordWin) {
			SetGWorld((CGrafPtr) recordWin, nil);
			InvalRect(&recordRect);
		}
		if(thePrefs.rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			InvalRect(&rythmsRect);
		}
		break;
	}
}

Boolean Write_Files(short fileID)
{
	long		bytesNumber,
				aliasSize;
	AliasHandle	theAlias = (AliasHandle) NewHandle(1000);
	
	bytesNumber = 4;
	//Ecrit les références des fichiers sons du sampler
	for(i = 0; i < maxNbSamples; ++i) {
		NewAliasMinimal(&samples[i].spec, &theAlias);
		HLock((Handle) theAlias);
		aliasSize = GetHandleSize((Handle) theAlias);
		if(!samples[i].fileID)
		aliasSize = 0;
		FSWrite(fileID, &bytesNumber, &aliasSize);
		FSWrite(fileID, &aliasSize, *theAlias);
	}
	//Ecrit les références des fichiers sons du synthétiseur
	for(i = 0; i < maxNbInstruments; ++i) {
		NewAliasMinimal(&instruments[i].spec, &theAlias);
		HLock((Handle) theAlias);
		aliasSize = GetHandleSize((Handle) theAlias);
		if(!instruments[i].fileID)
		aliasSize = 0;
		FSWrite(fileID, &bytesNumber, &aliasSize);
		FSWrite(fileID, &aliasSize, *theAlias);
	}
	
	return true;
}

typedef OSErr (*LIPI_Proc)(FSSpecPtr, Handle*, long, Fixed*);
typedef OSErr (*ODDPI_Proc)(FSSpecPtr, long, Fixed*, DTDPlugInParametersPtr);

Boolean Read_Files(short fileID)
{
	long				bytesNumber,
						aliasSize;
	short				tempID,
						item;
	AliasHandle			theAlias = (AliasHandle) NewHandle(1000);
	Boolean			isDifferent,
						forceLocate = false;
	StandardFileReply	theReply;
	long				j,
						k;
	SymClass			theClass;
	LIPI_Proc			theProcPtr;
	ODDPI_Proc			theProcPtr_2;
	Boolean			loop;
	FInfo				theInfo;
	Fixed				rate;
	DTDPlugInParameters	theParams;
	
	if(event.modifiers & optionKey)
	forceLocate = true;
	
	//Ferme tous les fichiers sons
	UseResFile(mainResFile);
	theCell.h = 0;
	for(i = 0; i < thePrefs.nbSamples; ++i)
	SF_CloseSample(i);
	for(i = 0; i < maxNbInstruments; ++i) {
		SF_CloseInstrument(i);
		theCell.v = i;
		GetIndString(theString, 0, 11);
		LSetCell(&theString[1], theString[0], theCell, instrumentsList);
	}
	//activeInstrument = 0;
	
	bytesNumber = 4;
	//Charge les fichiers sons du sampler
	for(i = 0; i < maxNbSamples; ++i) {
		
		Anim_WaitCursor();
		
		FSRead(fileID, &bytesNumber, &aliasSize);
		if(aliasSize > 0) {
			SetHandleSize((Handle) theAlias, aliasSize);
			HLock((Handle) theAlias);
			FSRead(fileID, &aliasSize, *theAlias);
			GetAliasInfo(theAlias, asiAliasName, samples[i].spec.name);
			samples[i].spec.vRefNum = musicFileSpec.vRefNum;
			samples[i].spec.parID = musicFileSpec.parID;
			theError = 0;
			if(HGetFInfo(samples[i].spec.vRefNum, samples[i].spec.parID, samples[i].spec.name, nil))
			theError = ResolveAlias(nil, theAlias, &samples[i].spec, &isDifferent);
			if(theError || forceLocate) { //Impossible de trouver le fichier son
				GetAliasInfo(theAlias, asiAliasName, samples[i].spec.name);
				ParamText(samples[i].spec.name, nil, nil, nil);
				UseResFile(mainResFile);
				CustomGetFile(nil, -1, nil, &theReply, 10000, whereToShow, nil, nil, nil, nil, nil);
				if(!theReply.sfGood)
				continue;
				
				samples[i].spec.vRefNum = theReply.sfFile.vRefNum;
				samples[i].spec.parID = theReply.sfFile.parID;
				BlockMove(theReply.sfFile.name, samples[i].spec.name, 63);
#if(kCreateFileIDs)
				theBlock.fidParam.ioNamePtr = theReply.sfFile.name;
				theBlock.fidParam.ioVRefNum = theReply.sfFile.vRefNum;
				theBlock.fidParam.ioSrcDirID = theReply.sfFile.parID;
				PBCreateFileIDRef(&theBlock, false);
#endif
			}
		}
	}
	
	bytesNumber = 4;
	//Charge les fichiers sons du synthétiseur
	for(i = 0; i < maxNbInstruments; ++i) {
		
		Anim_WaitCursor();
		
		FSRead(fileID, &bytesNumber, &aliasSize);
		if(aliasSize > 0) {
			SetHandleSize((Handle) theAlias, aliasSize);
			HLock((Handle) theAlias);
			FSRead(fileID, &aliasSize, *theAlias);
			GetAliasInfo(theAlias, asiAliasName, instruments[i].spec.name);
			instruments[i].spec.vRefNum = musicFileSpec.vRefNum;
			instruments[i].spec.parID = musicFileSpec.parID;
			theError = 0;
			if(HGetFInfo(instruments[i].spec.vRefNum, instruments[i].spec.parID, instruments[i].spec.name, nil))
			theError = ResolveAlias(nil, theAlias, &instruments[i].spec, &isDifferent);
			if(theError || forceLocate) { //Impossible de trouver le fichier son
				GetAliasInfo(theAlias, asiAliasName, instruments[i].spec.name);
				ParamText(instruments[i].spec.name, nil, nil, nil);
				UseResFile(mainResFile);
				CustomGetFile(nil, -1, nil, &theReply, 10000, whereToShow, nil, nil, nil, nil, nil);
				if(!theReply.sfGood)
				continue;
				
				instruments[i].spec.vRefNum = theReply.sfFile.vRefNum;
				instruments[i].spec.parID = theReply.sfFile.parID;
				BlockMove(theReply.sfFile.name, instruments[i].spec.name, 63);
#if(kCreateFileIDs)
				theBlock.fidParam.ioNamePtr = theReply.sfFile.name;
				theBlock.fidParam.ioVRefNum = theReply.sfFile.vRefNum;
				theBlock.fidParam.ioSrcDirID = theReply.sfFile.parID;
				PBCreateFileIDRef(&theBlock, false);
#endif
			}
		}
	}
	
	//Charge les sons du sampler
	SetGWorld((CGrafPtr) mainWin, nil);
	for(i = 0; i < thePrefs.nbSamples; ++i) {
		if(!recordPrefs.SSndNb[i])
		continue;
		
		Anim_WaitCursor();
		
		FSpGetFInfo(&samples[i].spec, &theInfo);
		if(recordPrefs.SSndNb[i] != DTDCode) { //Sample
			if(theInfo.fdType != 'Bank') {
				//Trouve le Plug-In adéquat
				j = 0; k = 0; loop = true;
				while(loop) {
					for(k = 0; k < kTypesPerImport; ++k)
					if(importPlugIns[j].importFileTypes[k] == theInfo.fdType)
					loop = false;
					
					if(loop)
					++j;
					if(j >= importPlugInsNumber)
					loop = false;
				}
				
				if(j >= importPlugInsNumber) {
					Do_Error(-30007, 107);
					continue;
				}
				
				//Appelle le Plug-In
				GetIndString(theString, 0, 23);
				theError = FindSymbol(importPlugIns[j].connID, theString, (char**) &theProcPtr, &theClass);
				if(theError) {
					Do_Error(theError, 105);
					continue;
				}
				if(thePrefs.flags & kByPassHeaders)
				theError = (*theProcPtr)(&samples[i].spec, &samples[i].snd, kLoopSound + kNewHeader, &thePrefs.SRate[i]);
				else
				theError = (*theProcPtr)(&samples[i].spec, &samples[i].snd, kLoopSound, &thePrefs.SRate[i]);
				if(theError) {
					Do_Error(theError, 106);
					continue;
				}
				thePrefs.SRelativeRate[i] = 0;
				HLock(samples[i].snd);
				UseResFile(mainResFile);
				
				BlockMove(samples[i].spec.name, samples[i].name, sizeof(Str63));
				while(StringWidth(samples[i].name) > maxWidthSmpl)
				--samples[i].name[0];
				
				samples[i].fileID = -1;
				samples[i].sndNb = 1;
				samples[i].totalSndNb = 1;
			}
			else
			SF_OpenBank(&samples[i].spec, i);
				
			samples[i].type = sampleType;
			
			theCommand.cmd		= soundCmd; //Joue le son
			theCommand.param1	= 0;
			theCommand.param2	= (long) *samples[i].snd;
			SndDoImmediate(samples[i].chan, &theCommand);
		}
		else { //DTD
			//Trouve le Plug-In adéquat
			j = 0; k = 0; loop = true;
			while(loop) {
				for(k = 0; k < kTypesPerDTD; ++k)
				if(DTDPlugIns[j].DTDFileTypes[k] == theInfo.fdType)
				loop = false;
				
				if(loop)
				++j;
				if(j >= DTDPlugInsNumber)
				loop = false;
			}
			
			if(j >= DTDPlugInsNumber) {
				Do_Error(-30007, 107);
				continue;
			}
				
			//Appelle le Plug-In
			GetIndString(theString, 0, 26);
			theError = FindSymbol(DTDPlugIns[j].connID, theString, (char**) &theProcPtr_2, &theClass);
			if(theError) {
				Do_Error(theError, 105);
				continue;
			}
			samples[i].snd = NewHandle(thePrefs.bufferSize);
			if(!samples[i].snd) {
				Do_Error(0, 702);
				samples[i].sndNb = 0;
				samples[i].totalSndNb = 0;
				GetIndString(samples[i].name, 0, 10);
				continue;
			}
			HLock(samples[i].snd);
			theParams.channel = samples[i].chan;
			theParams.fileID = 0;
			theParams.bufferHandle = samples[i].snd;
			theError = (*theProcPtr_2)(&samples[i].spec, 0L, &thePrefs.SRate[i], &theParams);
			if(theError) {
				Do_Error(theError, 106);
				continue;
			}
			thePrefs.SRelativeRate[i] = 0;
			UseResFile(mainResFile);
			
			BlockMove(samples[i].spec.name, samples[i].name, sizeof(Str63));
			while(StringWidth(samples[i].name) > maxWidthSmpl)
			--samples[i].name[0];
			
			samples[i].fileID = theParams.fileID;
			samples[i].sndNb = DTDCode;
			samples[i].totalSndNb = j;
			
			samples[i].type = directToDiskType;
		}
	}
	
	//Charge les sons du synthétiseur
	theCell.h = 0;
	//if(thePrefs.instrumentsWin)
	SetGWorld((CGrafPtr) instrumentsWin, nil);
	for(i = 0; i < maxNbInstruments; ++i)
	if(instruments[i].spec.parID) {
		
		Anim_WaitCursor();
		
		FSpGetFInfo(&instruments[i].spec, &theInfo);
		
		//Trouve le Plug-In adéquat
		j = 0; k = 0; loop = true;
		while(loop) {
			for(k = 0; k < kTypesPerImport; ++k)
			if(importPlugIns[j].importFileTypes[k] == theInfo.fdType)
			loop = false;
			
			if(loop)
			++j;
			if(j >= importPlugInsNumber)
			loop = false;
		}
		
		if(j >= importPlugInsNumber) {
			Do_Error(-30007, 107);
			continue;
		}
		
		//Appelle le Plug-In
		GetIndString(theString, 0, 23);
		theError = FindSymbol(importPlugIns[j].connID, theString, (char**) &theProcPtr, &theClass);
		if(theError) {
			Do_Error(theError, 105);
			continue;
		}
		if(thePrefs.flags & kByPassHeaders)
		theError = (*theProcPtr)(&instruments[i].spec, &instruments[i].snd, kNewHeader, &rate);
		else
		theError = (*theProcPtr)(&instruments[i].spec, &instruments[i].snd, 0L, &rate);
		if(theError) {
			Do_Error(theError, 106);
			continue;
		}
		HLock(instruments[i].snd);
		UseResFile(mainResFile);
		
		BlockMove(instruments[i].spec.name, instruments[i].name, sizeof(Str63));
		theCell.v = i;
		LSetCell(&instruments[i].name[1], instruments[i].name[0], theCell, instrumentsList);
		
		instruments[i].fileID = -1;
		
		theCommand.cmd		= soundCmd;
		theCommand.param1	= 0;
		theCommand.param2	= (long) *instruments[i].snd;
		for(k = 0; k < thePrefs.nbInsChannels; ++k)
		SndDoImmediate(insChannels[k], &theCommand);
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
	
	return true;
}

short Install_Music()
{
	//Mise à jour du sampler d'après les préférences de l'enregistrement
	SetGWorld((CGrafPtr) mainWin, nil);
	nbSamplesToStart = 0;
	nbSamplesToStop = 0;
	for(i = 0; i < thePrefs.nbSamples; ++i) {
		if(thePrefs.SPlaying[i]) {
			if(samples[i].type == sampleType)
			SCD_StopSample(i);
			else
			SCD_StopDirectToDisk(i);
		}
		chanFinished = nil;
		if(samples[i].type == sampleType && samples[i].sndNb
			!= recordPrefs.SSndNb[i] && samples[i].fileID && recordPrefs.SSndNb[i]) {
			HUnlock(samples[i].snd);
			DisposeHandle(samples[i].snd);
			
			UseResFile(samples[i].fileID);
			samples[i].sndNb = recordPrefs.SSndNb[i];
			
			samples[i].snd = Get1IndResource('snd ', samples[i].sndNb);
			if(!samples[i].snd) {
				Do_Error(0, 502);
				continue;
			}
			
			GetResInfo(samples[i].snd, &resID, &resType, samples[i].name);
			while(StringWidth(samples[i].name) > maxWidthSmpl)
			--samples[i].name[0];

			DetachResource(samples[i].snd);
			thePrefs.SRate[i] = SCD_CleanUpSound(samples[i].snd, true);
			thePrefs.SRelativeRate[i] = recordPrefs.SRelativeRate[i];
			HLock(samples[i].snd);
		}
		if(thePrefs.SStatus[i] != recordPrefs.SStatus[i])
		SCD_ReInitTrack(i, recordPrefs.SStatus[i]);
		thePrefs.SPan[i] = recordPrefs.SPan[i];
		thePrefs.SVol[i] = recordPrefs.SVol[i];
		thePrefs.SMaxVol[i] = recordPrefs.SMaxVol[i];
		thePrefs.SRelativeRate[i] = recordPrefs.SRelativeRate[i];
		SCD_VolumeSample(i);
		thePrefs.SLoop[i] = recordPrefs.SLoop[i];
		thePrefs.SSourceMode[i] = recordPrefs.SSourceMode[i];
		if(thePrefs.SReverb)
		SCD_EffectsSample(i);
		if(playing && recordPrefs.SPlaying[i]) {
			if(samples[i].type == sampleType)
			SCD_StartSample(i);
			else
			SCD_StartDirectToDisk(i);
		}
	}
	
	//Mise à jour du synthétiseur
	theCommand.cmd		= quietCmd;
	theCommand.param1	= 0;
	theCommand.param2	= 0;
	for(i = 0; i < thePrefs.nbInsChannels; ++i)
	SndDoImmediate(insChannels[i], &theCommand);
	if(thePrefs.synthMode != recordPrefs.synthMode) {
		thePrefs.synthMode = recordPrefs.synthMode;
		SCD_SetSynthetizerMode(thePrefs.synthMode);
	}
	thePrefs.IVol = recordPrefs.IVol;
	thePrefs.IMaxVol = recordPrefs.IMaxVol;
	SCD_VolumeSynthetizer();
	
	//Mise à jour de la boîte à rythmes
	if(thePrefs.RPlaying) {
		RB_Stop();
		theCommand.cmd		= quietCmd;
		theCommand.param1	= 0;
		theCommand.param2	= 0;
		for(i = 0; i < thePrefs.nbRytChannels; ++i)
		SndDoImmediate(rytChannels[i], &theCommand);
	}
	nbRBMutes = 0;
	patternPos = -1;
	thePrefs.RBPM = recordPrefs.RBPM;
	interTime = -(15000000 / thePrefs.RBPM);
	thePrefs.RCurrent = recordPrefs.RCurrent;
	thePrefs.RNext = recordPrefs.RNext;
	thePrefs.RVol = recordPrefs.RVol;
	thePrefs.RMaxVol = recordPrefs.RMaxVol;
	//SCD_VolumeRythmsBox();
	for(i = 0; i < kPatternParts; ++i) {
		thePrefs.RPartVol[i] = recordPrefs.RPartVol[i];
		thePrefs.RPartPan[i] = recordPrefs.RPartPan[i];
		thePrefs.RPartPitch[i] = recordPrefs.RPartPitch[i];
		thePrefs.RPartMute[i] = recordPrefs.RPartMute[i];
	}
	if(recordPrefs.RPlaying)
	RB_Start();
	
	//Mises à jour diverses
#if 0
	thePrefs.STurbo = recordPrefs.STurbo;
	thePrefs.ITurbo = recordPrefs.ITurbo;
	thePrefs.RTurbo = recordPrefs.RTurbo;
#endif
	
	thePrefs.loopSpeed = recordPrefs.loopSpeed;
	for(i = 1; i < 10; ++i)
	CheckItem(submenu[0], i, false);
	CheckItem(submenu[0], (1000 - thePrefs.loopSpeed * 4) / 100, true);
	
	//Déplace le sampler au max vers la gauche
	if(!playing) {
		for(i = 0; i < maxNbSamples; ++i)
		uc[i] = -1;
		for(i = 0; i < nbShowSamples; ++i) {
			c[i] = i;
			uc[i] = i;
		}
		SetCtlValue(samplerScrollBar, 0);
	}
	
	return nil;
}