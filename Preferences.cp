#include			<Folders.h>

#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Record Codes.h"
#include			"Variables.h"

//FONCTIONS:

short Create_Pref()
{
	long		prefFolderNum;
	short		volumeID;
	Str255		tempString;
	
	GetIndString(tempString, 900, 4);
	
	//Initialise la structure
	thePrefs.registed[0]		= 0;
	BlockMove(tempString, thePrefs.userName, sizeof(Str255));
	thePrefs.soundsFolder.vRefNum	= 0;
	thePrefs.soundsFolder.parID		= 0;
	thePrefs.soundsFolder.name[0]	= 0;
	
	thePrefs.loopDTD			= false;
	thePrefs.flags				= 0xFF;
	thePrefs.quantizeFlags		= 0x00;
	
#if demo

#else
	AskForSoundsFolder();
#endif
	
	thePrefs.samplesWin			= true;
	thePrefs.smpWinExt			= true;
	thePrefs.recordWin			= true;
	thePrefs.recWinExt			= true;
	thePrefs.instrumentsWin		= true;
	thePrefs.insWinExt			= true;
	thePrefs.rythmsWin			= true;
	thePrefs.rytWinExt			= true;
	thePrefs.smpWinPos.h		= 40;
	thePrefs.smpWinPos.v		= 40;
	thePrefs.recWinPos.h		= 40;
	thePrefs.recWinPos.v		= 40;
	thePrefs.insWinPos.h		= 40;
	thePrefs.insWinPos.v		= 40;
	thePrefs.rytWinPos.h		= 40;
	thePrefs.rytWinPos.v		= 40;
	
	thePrefs.nbSamples			= 16;
	thePrefs.nbInsChannels		= 2;
	thePrefs.nbRytChannels		= 2;
	thePrefs.reserved1			= nil;
	thePrefs.reserved2			= nil;
	thePrefs.bufferSize			= 250000;
	thePrefs.outputVol			= Shorts_To_Long(200, 200);
	thePrefs.loopSpeed			= 100;
	
	thePrefs.enableMIDIDriver	= false;
	thePrefs.MIDIFlags			= 0xFF;
	thePrefs.MIDIDriverMode		= kMIDIAllToSynth;
	thePrefs.MIDISamplerChannel	= 0;
	thePrefs.MIDISynthChannel	= 1;
	thePrefs.MIDIRBChannel		= 2;
	thePrefs.reserved3			= nil;
	
	thePrefs.SReverb			= false;
	thePrefs.STurbo				= 2 * defaultMaxVolume;
	for(i = 0; i < maxNbSamples; ++i) {
#if defined(powerc) || defined (__powerc)
		thePrefs.SSourceMode[i]	= kSSpSourceMode_Unfiltered;
#else
		thePrefs.SSourceMode[i]	= nil;
#endif
		thePrefs.SStatus[i]		= initStereo;
		thePrefs.SGroup[i]		= -1;
		thePrefs.SSndNb[i]		= 0;
		thePrefs.SVol[i]		= defaultMaxVolume;
		thePrefs.SPan[i]		= 100;
		thePrefs.SRate[i]		= 0;
		thePrefs.SRelativeRate[i] = 0;
		thePrefs.SLoop[i]		= 0;
		thePrefs.SPlaying[i]	= false;
		thePrefs.SMaxVol[i]		= defaultMaxVolume;
	}
	
	thePrefs.synthMode			= 2;
	thePrefs.IReverb			= false;
#if defined(powerc) || defined (__powerc)
	thePrefs.ISourceMode		= kSSpSourceMode_Unfiltered;
#else
	thePrefs.ISourceMode		= nil;

#endif
	thePrefs.ITurbo				= 2 * defaultMaxVolume;
	thePrefs.IVol				= defaultMaxVolume;
	thePrefs.IMaxVol			= defaultMaxVolume;
	
	thePrefs.RReverb			= false;
#if defined(powerc) || defined (__powerc)
	thePrefs.RSourceMode		= kSSpSourceMode_Unfiltered;
#else
	thePrefs.RSourceMode		= nil;
#endif
	thePrefs.RTurbo				= 2 * defaultMaxVolume;
	thePrefs.RPlaying			= false;
	thePrefs.RBPM				= 120;
	thePrefs.RCurrent			= 0;
	thePrefs.RNext				= 0;
	thePrefs.RVol				= defaultMaxVolume;
	thePrefs.RMaxVol			= defaultMaxVolume;
	for(i = 0; i < kPatternParts; ++i) {
		thePrefs.RPartMute[i]	= false;
		thePrefs.RPartVol[i]	= kDefaultPartVolume;
		thePrefs.RPartPan[i]	= 100;
		thePrefs.RPartPitch[i]	= baseKey;
	}
	
	thePrefs.enableGroups		= false;
	thePrefs.GRelative			= false;
	thePrefs.GPlay				= false;
	thePrefs.GVol				= false;
	thePrefs.GPan				= false;
	thePrefs.GPitch				= false;
	thePrefs.GOptions			= false;
	thePrefs.GInit				= false;
	
	thePrefs.sampleColor.red	= 65535;
	thePrefs.sampleColor.green	= 65535;
	thePrefs.sampleColor.blue	= 39321;
	thePrefs.DTDColor.red		= 65535;
	thePrefs.DTDColor.green		= 39321;
	thePrefs.DTDColor.blue		= 39321;
	thePrefs.displayColor.red	= 39321;
	thePrefs.displayColor.green	= 65535;
	thePrefs.displayColor.blue	= 65535;
	
#if defined(powerc) || defined (__powerc)
	thePrefs.localization.medium							= kSSpMedium_Air; //Air ou Water
	thePrefs.localization.humidity							= 0; //%
	thePrefs.localization.roomSize							= 10; //distance to walls in meters
	thePrefs.localization.roomReflectivity					= -15; //reflectivity in dB
	thePrefs.localization.reverbAttenuation					= 4; //reverberation in dB
	thePrefs.localization.sourceMode						= kSSpSourceMode_Unfiltered;
																//Localized ou Binaural ou Ambient ou Unfiltered
	thePrefs.localization.referenceDistance					= 4;
	thePrefs.localization.coneAngleCos						= 0; //cone
	thePrefs.localization.coneAttenuation					= 0; //attenuation in dB when outside the cone
	thePrefs.localization.currentLocation.elevation			= 0;
	thePrefs.localization.currentLocation.azimuth			= 0;
	thePrefs.localization.currentLocation.distance			= 4; //distance to sound in meters
	thePrefs.localization.currentLocation.projectionAngle	= 1;
	thePrefs.localization.currentLocation.sourceVelocity	= 0;
	thePrefs.localization.currentLocation.listenerVelocity	= 0;
	thePrefs.localization.reserved0							= 0;
	thePrefs.localization.reserved1							= 0;
	thePrefs.localization.reserved2							= 0;
	thePrefs.localization.reserved3							= 0;
	thePrefs.localization.virtualSourceCount				= 0;
#else
	for(i = 0; i < 196; ++i)
	thePrefs.localization[i] = nil;
#endif
	
	//Place les fen�tres
	thePrefs.smpWinPos.h = (dragRect.right - samplerWide) / 2;
	thePrefs.smpWinPos.v = 40;
	thePrefs.recWinPos.h = thePrefs.smpWinPos.h + 288 + (samplerWide - 288 - 159) / 2;
	thePrefs.recWinPos.v = 360;
	thePrefs.insWinPos.h = thePrefs.smpWinPos.h;
	thePrefs.insWinPos.v = 330;
	thePrefs.rytWinPos.h = thePrefs.smpWinPos.h + samplerWide - 320;
	thePrefs.rytWinPos.v = 330;
	
	//Cr�e le fichier dans le dossier Pr�f�rences
	FindFolder(kOnSystemDisk, 'pref', kDontCreateFolder, &volumeID, &prefFolderNum);
	theError = HCreate(volumeID, prefFolderNum, prefName, 'HiFc', 'Data');
	if(theError) {
		Do_Error(theError, 201);
		return false;
	}
	Update_Pref(); //Ecrit le fichier
	
	return true;
}

short Read_Pref()
{
	short		fileNumber,
				temp,
				volumeID;
	long		bytesNumber,
				prefFolderNum;
				
	FindFolder(kOnSystemDisk, 'pref', kDontCreateFolder, &volumeID, &prefFolderNum);
	if(HOpen(volumeID, prefFolderNum, prefName, fsRdWrPerm, &fileNumber)) //existe pas
	return false;
	
	Anim_WaitCursor();
	
	SetFPos(fileNumber, 1, 0);
	bytesNumber = 2;
	theError = FSRead(fileNumber, &bytesNumber, &temp);
	if(temp != prefVersion) { //Contr�le le num�ro de version
		FSClose(fileNumber); //Si mauvais: efface le fichier
		theError = HDelete(volumeID, prefFolderNum, prefName);
		if(theError)
		Do_Error(theError, 204);
		return false;
	}
	
	Anim_WaitCursor();
	
	bytesNumber = sizeof(thePrefs); //charge la structure en m�moire
	theError = FSRead(fileNumber, &bytesNumber, &thePrefs);
	if(theError) {
		Do_Error(theError, 202);
		FSClose(fileNumber);
		return false;
	}
	FSClose(fileNumber);
	
	return true;
}

short Update_Pref()
{
	short		fileNumber,
				volumeID;
	long		bytesNumber,
				prefFolderNum;
	
	FindFolder(kOnSystemDisk, 'pref', kDontCreateFolder, &volumeID, &prefFolderNum);
	//Ouvre le fichier
	theError = HOpen(volumeID, prefFolderNum, prefName, fsRdWrPerm, &fileNumber);
	if(theError) {
		Do_Error(theError, 203);
		return false;
	}
	
	//Ecrit le num�ro de version puis la structure
	SetFPos(fileNumber, 1, 0);
	bytesNumber = 2;
	theError = FSWrite(fileNumber, &bytesNumber, &prefVersion);
	if(theError) {
		Do_Error(theError, 103);
		return false;
	}
	bytesNumber = sizeof(thePrefs);
	theError = FSWrite(fileNumber, &bytesNumber, &thePrefs);
	FSClose(fileNumber);
	
	return true;
}

void Init_PrefDialog()
{
	Str255		tempString;
	long		tempValue;
				
	UseResFile(mainResFile);
	theDialog = GetNewDialog(3000, nil, (WindowPtr) -1);
	GetDItem(theDialog, 3, nil, &dialogItem[0], &aRect);
	GetDItem(theDialog, 5, nil, &dialogItem[1], &aRect);
	GetDItem(theDialog, 6, nil, &dialogItem[2], &aRect);
	GetDItem(theDialog, 7, nil, &dialogItem[3], &aRect);
	GetDItem(theDialog, 8, nil, &dialogItem[4], &aRect);
	GetDItem(theDialog, 9, nil, &dialogItem[5], &aRect);
	GetDItem(theDialog, 10, nil, &dialogItem[6], &aRect);
	GetDItem(theDialog, 11, nil, &dialogItem[7], &aRect);
	GetDItem(theDialog, 12, nil, &dialogItem[8], &aRect);
	GetDItem(theDialog, 13, nil, &dialogItem[9], &aRect);
	GetDItem(theDialog, 1, nil, &dialogItem[10], &aRect);
	GetDItem(theDialog, 14, nil, &dialogItem[11], &aRect);
#if demo
	GetDItem(theDialog, 4, nil, &dialogItem[12], &aRect);
	HiliteControl((ControlHandle) dialogItem[12], 255);
#endif
	
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	SetGWorld((CGrafPtr) savePort, nil);
	
	Hide_Menus();
	
	//Cr�e la bo�te de dialogue
	tempValue = HiWord(thePrefs.outputVol); //Max vol
	NumToString(tempValue, tempString);
	SetIText(dialogItem[0], tempString);
	tempValue = thePrefs.nbSamples; //Sampler Tracks
	NumToString(tempValue, tempString);
	SetIText(dialogItem[1], tempString);
	tempValue = thePrefs.STurbo; //Sampler Turbo
	NumToString(tempValue, tempString);
	SetIText(dialogItem[2], tempString);
	SetCtlValue((ControlHandle) dialogItem[3], thePrefs.loopDTD); //loop DTD
	tempValue = thePrefs.nbInsChannels; //Synth poly
	NumToString(tempValue, tempString);
	SetIText(dialogItem[4], tempString);
	tempValue = thePrefs.ITurbo; //Synth Turbo
	NumToString(tempValue, tempString);
	SetIText(dialogItem[5], tempString);
	tempValue = thePrefs.nbRytChannels; //Rythms Box Polyphony
	NumToString(tempValue, tempString);
	SetIText(dialogItem[6], tempString);
	tempValue = thePrefs.RTurbo; //Rythms Box Turbo
	NumToString(tempValue, tempString);
	SetIText(dialogItem[7], tempString);
	SetCtlValue((ControlHandle) dialogItem[8], thePrefs.flags & kByPassHeaders); //ByPassHeaders
	SetCtlValue((ControlHandle) dialogItem[9], thePrefs.flags & kNoInterruptBank); //No interrupt
	tempValue = thePrefs.bufferSize; //DTD buffer
	NumToString(tempValue / 1000, tempString);
	SetIText(dialogItem[11], tempString);
	SelIText(theDialog, 3, 0, 32767); //Misc
	
	dialogRunning = 1;
}

void Update_PrefDialog()
{
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	BeginUpdate(theDialog);
		OutLine_Item(theDialog, 23);
		OutLine_Item(theDialog, 25);
		OutLine_Item(theDialog, 27);
		OutLine_Item(theDialog, 29);
		OutLine_Item(theDialog, 31);
		Draw_DefaultItem(theDialog, 1);
		DrawDialog(theDialog);
	EndUpdate(theDialog);
	SetGWorld((CGrafPtr) savePort, nil);
}
	
void Act_PrefDialog(short item)
{
	if(item == 1) //OK
	Dispose_PrefDialog();
	if(item == 2) { //Cancel
		DisposeDialog(theDialog);
		//SetDAFont(0);
		for(i = 0; i < 20; ++i)
			//if(dialogItem[i])
			DisposeHandle(dialogItem[i]);
		dialogRunning = 0;
		Show_Menus();
	}
	if(item == 4) //Sounds Folder
	AskForSoundsFolder();
	if(item == 7) { //Case � cocher - loopDTD
		i = GetCtlValue((ControlHandle) dialogItem[3]);
		SetCtlValue((ControlHandle) dialogItem[3], !i);
	}
	if(item == 12) { //Case � cocher - byPass
		i = GetCtlValue((ControlHandle) dialogItem[8]);
		SetCtlValue((ControlHandle) dialogItem[8], !i);
	}
	if(item == 13) { //Case � cocher - interrupt
		i = GetCtlValue((ControlHandle) dialogItem[9]);
		SetCtlValue((ControlHandle) dialogItem[9], !i);
		if(i)
		Do_Error(-30006, 510);
	}
	if(item == 34)
	HMSetBalloons(!HMGetBalloons());
	if(item == -1 && (theKey == kReturnKey || theKey == kEnterKey)) { //Touche clavier
		HiliteControl((ControlHandle) dialogItem[10], inButton);
		Delay(kEnterDelay, &ticks);
		//Wait(kEnterDelay);
		Dispose_PrefDialog();
	}
}
	
void Dispose_PrefDialog()
{
	Str255		tempString;
	long		tempValue;
	
	//Examine le nombre de pistes du sampler
	GetIText(dialogItem[1], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 8)
	tempValue = 8;
	if(tempValue > 19)
	tempValue = 19;
	if(tempValue < thePrefs.nbSamples) {
		for(i = tempValue; i < thePrefs.nbSamples; ++i)
		SCD_KillTrack(i);
	}
	if(tempValue > thePrefs.nbSamples) {
		for(i = thePrefs.nbSamples; i < tempValue; ++i) {
			thePrefs.SStatus[i]		= initStereo;
			thePrefs.SSndNb[i]		= 0;
			thePrefs.SVol[i]		= defaultMaxVolume;
			thePrefs.SPan[i]		= 100;
			thePrefs.SRate[i]		= 0;
			thePrefs.SRelativeRate[i] = 0;
			thePrefs.SLoop[i]		= 0;
			thePrefs.SPlaying[i]	= false;
			thePrefs.SMaxVol[i]		= defaultMaxVolume;
			
			samples[i].fileID = 0;
			samples[i].totalSndNb = 0;
			GetIndString(samples[i].name, 0, 10);
			SCD_CreateTrack(i);
		}
	}
	if(tempValue != thePrefs.nbSamples) {
		for(i = 0; i < maxNbSamples; ++i)
		uc[i] = -1;
		for(i = 0; i < nbShowSamples; ++i) {
			c[i] = i;
			uc[i] = i;
		}
	}
	thePrefs.nbSamples = tempValue;
	
	//Examine le nombre de voix du synth�tiseur
	GetIText(dialogItem[4], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 0)
	tempValue = 0;
	if(tempValue > maxNbInsChannels)
	tempValue = maxNbInsChannels;
	if(tempValue < thePrefs.nbInsChannels)
		for(i = tempValue; i < thePrefs.nbInsChannels; ++i)
		SndDisposeChannel(insChannels[i], true);
	if(tempValue > thePrefs.nbInsChannels)
	for(i = thePrefs.nbInsChannels; i < tempValue; ++i) {
		theError = SndNewChannel(&insChannels[i], sampledSynth, initStereo, nil);
		if(theError)
		Do_Error(theError, 504);
		
		theCommand.cmd		= volumeCmd;
		theCommand.param1	= 0;
		theCommand.param2	= Shorts_To_Long(thePrefs.IVol, thePrefs.IVol);
		SndDoImmediate(insChannels[i], &theCommand);
		
		//PBM
	}
	SCD_SetSynthetizerMode(thePrefs.synthMode); //A tester
	thePrefs.nbInsChannels = tempValue;
	activeChan = 0;
	
	//Examine le nombre de voix de la Bo�te � Rythmes:
	GetIText(dialogItem[6], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 0)
	tempValue = 0;
	if(tempValue > kMaxRythmsChan)
	tempValue = kMaxRythmsChan;
	if(tempValue < thePrefs.nbRytChannels)
		for(i = tempValue; i < thePrefs.nbRytChannels; ++i)
		SndDisposeChannel(rytChannels[i], true);
	if(tempValue > thePrefs.nbRytChannels)
	for(i = thePrefs.nbRytChannels; i < tempValue; ++i) {
		theError = SndNewChannel(&rytChannels[i], sampledSynth, initStereo, nil);
		if(theError)
		Do_Error(theError, 504);
		
		theCommand.cmd		= volumeCmd;
		theCommand.param1	= 0;
		theCommand.param2	= Shorts_To_Long(thePrefs.RVol, thePrefs.RVol);
		SndDoImmediate(rytChannels[i], &theCommand);
		
		//PBM
	}
	thePrefs.nbRytChannels = tempValue;
	activeRythmChan = 0;
	
	//Examine Max vol
	GetIText(dialogItem[0], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 100)
	tempValue = 100;
	if(tempValue > 1024)
	tempValue = 1024;
	thePrefs.outputVol = Shorts_To_Long(tempValue, tempValue);
	SetDefaultOutputVolume(thePrefs.outputVol);
	
	//Examine Sampler Turbo
	GetIText(dialogItem[2], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < defaultMaxVolume)
	tempValue = defaultMaxVolume;
	if(tempValue > 1024)
	tempValue = 1024;
	if(thePrefs.STurbo != tempValue)
	for(i = 0; i < thePrefs.nbSamples; ++i) {
		thePrefs.SVol[i] = defaultMaxVolume;
		thePrefs.SMaxVol[i] = defaultMaxVolume;
		SCD_VolumeSample(i);
	}
	thePrefs.STurbo = tempValue;
	
	//Examine Synth Turbo
	GetIText(dialogItem[5], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < defaultMaxVolume)
	tempValue = defaultMaxVolume;
	if(tempValue > 1024)
	tempValue = 1024;
	if(thePrefs.ITurbo != tempValue) {
		thePrefs.IVol = defaultMaxVolume;
		thePrefs.IMaxVol = defaultMaxVolume;
		SCD_VolumeSynthetizer();
	}
	thePrefs.ITurbo = tempValue;
	
	//Examine Rythms Turbo
	GetIText(dialogItem[7], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < defaultMaxVolume)
	tempValue = defaultMaxVolume;
	if(tempValue > 1024)
	tempValue = 1024;
	if(thePrefs.RTurbo != tempValue) {
		thePrefs.RVol = defaultMaxVolume;
		thePrefs.RMaxVol = defaultMaxVolume;
	}
	thePrefs.RTurbo = tempValue;
	
	//Examine loopDTD
	thePrefs.loopDTD = GetCtlValue((ControlHandle) dialogItem[3]);
	
	//Examine byPassHeaders
	if(GetCtlValue((ControlHandle) dialogItem[8]))
	thePrefs.flags = thePrefs.flags | kByPassHeaders;
	else
	thePrefs.flags = thePrefs.flags & (~kByPassHeaders);
	
	//Examine No sound loading at interrupt
	if(GetCtlValue((ControlHandle) dialogItem[9]))
	thePrefs.flags = thePrefs.flags | kNoInterruptBank;
	else
	thePrefs.flags = thePrefs.flags & (~kNoInterruptBank);
	
	//Examine DTD buffer
	GetIText(dialogItem[11], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 25)
	tempValue = 25;
	thePrefs.bufferSize = tempValue * 1000;
	
	//Update
	SetGWorld((CGrafPtr) mainWin, nil);
	InvalRect(&shellRect);
	SetCtlValue(samplerScrollBar, 0);
	SetCtlMax(samplerScrollBar, thePrefs.nbSamples - nbShowSamples);
	if(thePrefs.recordWin) {
		SetGWorld((CGrafPtr) recordWin, nil);
		InvalRect(&recordRect);
	}
	if(thePrefs.instrumentsWin) {
		SetGWorld((CGrafPtr) instrumentsWin, nil);
		InvalRect(&instrumentsRect);
	}
	if(thePrefs.rythmsWin) {
		SetGWorld((CGrafPtr) rythmsWin, nil);
		InvalRect(&rythmsRect);
	}
	
	DisposeDialog(theDialog);
	
	Update_Pref();
	
	for(i = 0; i < 20; ++i)
		//if(dialogItem[i])
		DisposeHandle(dialogItem[i]);
	dialogRunning = 0;
	Show_Menus();
}