#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

// FONCTIONS:

void Init_Toolbox()
{
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	MaxApplZone();
}

void Init_Menus()
{
	MenuHandle	aMenu;
	
	menu[appleM] = GetMenu(appleID);
	AddResMenu(menu[appleM], 'DRVR');
	menu[fileM] = GetMenu(fileID);
	menu[utilitiesM] = GetMenu(utilitiesID);
	menu[windowM] = GetMenu(windowID);
	menu[editorM] = GetMenu(editorID);
	
	submenu[0] = GetMenu(110);
	submenu[1] = GetMenu(100);
	submenu[2] = GetMenu(120);
	
	popUpMenu = GetMenu(2000);
	popUpMenu2 = GetMenu(5000);
	patternMenu = GetMenu(6000);
	
	InsertMenu(menu[0], 0);
	InsertMenu(menu[1], 0);
	InsertMenu(menu[2], 0);
	InsertMenu(menu[3], 0);
	InsertMenu(menu[4], 0);
	
	InsertMenu(submenu[0], -1);
	InsertMenu(submenu[1], -1);
	InsertMenu(submenu[2], -1);
	
	InsertMenu(popUpMenu, -1);
	InsertMenu(popUpMenu2, -1);
	InsertMenu(patternMenu, -1);
	
	HMGetHelpMenuHandle(&aMenu);
	GetIndString(theString, 0, 9);
	AppendMenu(aMenu, theString);
	
	DrawMenuBar();
}

typedef OSErr (*IIPI_Proc)(importPlugInInfosPtr);

void Init_ImportPlugIns()
{
	short			idx,
					idx2,
					soundFilesNumber;
	CInfoPBRec		cipbr,
					cipbr2;
	HFileInfo		*fpb = (HFileInfo*) &cipbr,
					*fpb2 = (HFileInfo*) &cipbr2;
	DirInfo			*dpb = (DirInfo*) &cipbr;
	ParamBlockRec	theParam;
	Str63			plugInsFolder;
					//volumeName;
	Str255			errorString,
					initRoutineName;
	FSSpec			fileSpec;
	Ptr				thePtr;
	importPlugInInfos	plugInInfos;
	SymClass		theClass;
	IIPI_Proc		theProcPtr;
	
	importPlugInsNumber = 0;
	soundFilesNumber = 0;
	for(i = 0; i < kMaxImportPlugsIns * 2; ++i)
	importFileTypes[i] = 0L;
	
	//Get the name of the Plug-Ins folder
	GetIndString(plugInsFolder, 0, 21);
#if 0
	BlockMove(thePrefs.programLocation.name, volumeName, sizeof(Str63));
	++volumeName[0];
	volumeName[volumeName[0]] = ':';

	//R�cup�re VRefNum d'apr�s nom volume
	theParam.volumeParam.ioCompletion	= nil;
	theParam.volumeParam.ioVolIndex		= -1;
	theParam.volumeParam.ioNamePtr		= volumeName;
	theParam.volumeParam.ioVRefNum		= 0;
	PBGetVInfo(&theParam, false);
	thePrefs.programLocation.vRefNum = theParam.volumeParam.ioVRefNum;
#else
	short vRefNum;
	long dirID;
	
	//R�cup�re le dossier de l'application:
	HGetVol(nil, &vRefNum, &dirID);
#endif
	
	//Scan dossier HF
	fpb->ioVRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
	fpb->ioNamePtr = theString;
	fpb2->ioVRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
	fpb2->ioNamePtr = theString;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = dirID;//thePrefs.programLocation.parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		
		//Scan dossier Plug-Ins
		if(fpb->ioFlAttrib & 16 && EqualString(theString, plugInsFolder, false, false)) {
			for(idx2 = 1; true; ++idx2) {
				fpb2->ioDirID = dpb->ioDrDirID;
				fpb2->ioFDirIndex = idx2;
				if(PBGetCatInfo(&cipbr2, false))
				break;
				
				//Charge Plug-In
				if(fpb2->ioFlFndrInfo.fdType == 'ImPI') {
					fileSpec.vRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
					fileSpec.parID = dpb->ioDrDirID;
					BlockMove(theString, fileSpec.name, sizeof(Str63));
					if(!GetDiskFragment(&fileSpec, 0, 0, nil, kLoadNewCopy, 
					 &importPlugIns[importPlugInsNumber].connID, &thePtr, errorString)) {
						
						//GetIndSymbol(importPlugIns[importPlugInsNumber].connID, 0, volumeName, &thePtr, &theClass);
						//DebugStr(volumeName);
						
						//Init Plug-In
						GetIndString(initRoutineName, 0, 22);
						if(FindSymbol(importPlugIns[importPlugInsNumber].connID, initRoutineName, (char**) &theProcPtr, &theClass))
						CloseConnection(&importPlugIns[importPlugInsNumber].connID);
						else {
							for(i = 0; i < kTypesPerImport; ++i)
							plugInInfos.importFileType[i] = 0L;
							theError = (*theProcPtr)(&plugInInfos);
							/*BlockMove(&plugInInfos.importFileType[0], &volumeName[1], 4);
							volumeName[0]=4;
							DebugStr(volumeName);*/
							if(theError || plugInInfos.minimumVersion > curVersion)
							CloseConnection(&importPlugIns[importPlugInsNumber].connID);
							else {
								for(i = 0; i < kTypesPerImport; ++i)
								if(plugInInfos.importFileType[i]) {
									importFileTypes[soundFilesNumber] = plugInInfos.importFileType[i];
									++soundFilesNumber;
								}
								for(i = 0; i < kTypesPerImport; ++i)
								importPlugIns[importPlugInsNumber].importFileTypes[i] = plugInInfos.importFileType[i];
								
								++importPlugInsNumber;
							}
						}
					}
				}
				Anim_WaitCursor();
				
			}
		}
	}
}

typedef OSErr (*IDDPI_Proc)(DTDPlugInInfosPtr, SndChannelPtr*);

void Init_DTDPlugIns()
{
	short			idx,
					idx2,
					soundFilesNumber;
	CInfoPBRec		cipbr,
					cipbr2;
	HFileInfo		*fpb = (HFileInfo*) &cipbr,
					*fpb2 = (HFileInfo*) &cipbr2;
	DirInfo			*dpb = (DirInfo*) &cipbr;
	ParamBlockRec	theParam;
	Str63			plugInsFolder;
					//volumeName;
	Str255			errorString,
					initRoutineName;
	FSSpec			fileSpec;
	Ptr				thePtr;
	DTDPlugInInfos	plugInInfos;
	SymClass		theClass;
	IDDPI_Proc		theProcPtr;
	
	DTDPlugInsNumber = 0;
	soundFilesNumber = 0;
	for(i = 0; i < kMaxDTDPlugsIns * 2; ++i)
	DTDFileTypes[i] = 0L;
	
	//Get the name of the Plug-Ins folder
	GetIndString(plugInsFolder, 0, 21);
#if 0
	BlockMove(thePrefs.programLocation.name, volumeName, sizeof(Str63));
	++volumeName[0];
	volumeName[volumeName[0]] = ':';
	
	//R�cup�re VRefNum d'apr�s nom volume
	theParam.volumeParam.ioCompletion	= nil;
	theParam.volumeParam.ioVolIndex		= -1;
	theParam.volumeParam.ioNamePtr		= volumeName;
	theParam.volumeParam.ioVRefNum		= 0;
	PBGetVInfo(&theParam, false);
	thePrefs.programLocation.vRefNum = theParam.volumeParam.ioVRefNum;
#else
	short vRefNum;
	long dirID;
	
	//R�cup�re le dossier de l'application:
	HGetVol(nil, &vRefNum, &dirID);
#endif
	
	//Scan dossier HF
	fpb->ioVRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
	fpb->ioNamePtr = theString;
	fpb2->ioVRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
	fpb2->ioNamePtr = theString;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = dirID;//thePrefs.programLocation.parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		
		//Scan dossier Plug-Ins
		if(fpb->ioFlAttrib & 16 && EqualString(theString, plugInsFolder, false, false)) {
			for(idx2 = 1; true; ++idx2) {
				fpb2->ioDirID = dpb->ioDrDirID;
				fpb2->ioFDirIndex = idx2;
				if(PBGetCatInfo(&cipbr2, false))
				break;
				
				//Charge Plug-In
				if(fpb2->ioFlFndrInfo.fdType == 'DDPI') {
					fileSpec.vRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
					fileSpec.parID = dpb->ioDrDirID;
					BlockMove(theString, fileSpec.name, sizeof(Str63));
					if(!GetDiskFragment(&fileSpec, 0, 0, nil, kLoadNewCopy, 
					 &DTDPlugIns[DTDPlugInsNumber].connID, &thePtr, errorString)) {
						
						//Init Plug-In
						GetIndString(initRoutineName, 0, 25);
						if(FindSymbol(DTDPlugIns[DTDPlugInsNumber].connID, initRoutineName, (char**) &theProcPtr, &theClass))
						CloseConnection(&DTDPlugIns[DTDPlugInsNumber].connID);
						else {
							for(i = 0; i < kTypesPerImport; ++i)
							plugInInfos.DTDFileType[i] = 0L;
							theError = (*theProcPtr)(&plugInInfos, &chanFinished);
							if(theError || plugInInfos.minimumVersion > curVersion)
							CloseConnection(&DTDPlugIns[DTDPlugInsNumber].connID);
							else {
								for(i = 0; i < kTypesPerImport; ++i)
								if(plugInInfos.DTDFileType[i]) {
									DTDFileTypes[soundFilesNumber] = plugInInfos.DTDFileType[i];
									++soundFilesNumber;
								}
								for(i = 0; i < kTypesPerDTD; ++i)
								DTDPlugIns[DTDPlugInsNumber].DTDFileTypes[i] = plugInInfos.DTDFileType[i];
								
								++DTDPlugInsNumber;
							}
						}
					}
				}
				Anim_WaitCursor();
				
			}
		}
	}
}
	
void Init_LoadMenus()
{
	short			idx,
					idx2,
					menuItem = 0;
	CInfoPBRec		cipbr,
					cipbr2;
	HFileInfo		*fpb = (HFileInfo*) &cipbr,
					*fpb2 = (HFileInfo*) &cipbr2;
	DirInfo			*dpb = (DirInfo*) &cipbr;
	ParamBlockRec	theParam;
	Str63			volumeName,
					plugInsFolder;
	
#if demo
	return;
#else
	if(!thePrefs.soundsFolder.name[0])
	return;
	
	GetIndString(plugInsFolder, 0, 21);
	BlockMove(thePrefs.soundsFolder.name, volumeName, sizeof(Str63));
	++volumeName[0];
	volumeName[volumeName[0]] = ':';
	
	//R�cup�re VRefNum d'apr�s nom volume
	theParam.volumeParam.ioCompletion	= nil;
	theParam.volumeParam.ioVolIndex		= -1;
	theParam.volumeParam.ioNamePtr		= volumeName;
	theParam.volumeParam.ioVRefNum		= 0;
	PBGetVInfo(&theParam, false);
	thePrefs.soundsFolder.vRefNum = theParam.volumeParam.ioVRefNum;
	
	loadMenus[0] = NewMenu(kStartLoadMenusID, "\p"); //Cr�e le sous-menu
	InsertMenu(loadMenus[0], -1);
	SetItemCmd(popUpMenu, 5, hMenuCmd);
	SetItemMark(popUpMenu, 5, kStartLoadMenusID);
	
	fpb->ioVRefNum = thePrefs.soundsFolder.vRefNum; //Scan dossiers
	fpb->ioNamePtr = theString;
	fpb2->ioVRefNum = thePrefs.soundsFolder.vRefNum;
	fpb2->ioNamePtr = theString;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = thePrefs.soundsFolder.parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		if((fpb->ioFlAttrib & 16) && theString[1] != ' ' && !EqualString(theString, plugInsFolder, false, false)) {
			++menuItem;
			if(menuItem > kNbLoadMenus - 1)
			break;
			AppendMenu(loadMenus[0], theString);
			loadMenus[menuItem] = NewMenu(kStartLoadMenusID + menuItem, "\p");
			InsertMenu(loadMenus[menuItem], -1);
			SetItemCmd(loadMenus[0], menuItem, hMenuCmd);
			SetItemMark(loadMenus[0], menuItem, kStartLoadMenusID + menuItem);
			
			loadMenusDirID[menuItem] = dpb->ioDrDirID;
			for(idx2 = 1; true; ++idx2) {
				fpb2->ioDirID = dpb->ioDrDirID;
				fpb2->ioFDirIndex = idx2;
				if(PBGetCatInfo(&cipbr2, false))
				break;
				
				i = 0;
				while(importFileTypes[i]) {
					if(fpb2->ioFlFndrInfo.fdType == importFileTypes[i]) {
						AppendMenu(loadMenus[menuItem], theString);
						break;
					}
					++i;
				}
				
				if(fpb2->ioFlFndrInfo.fdType == 'Bank')
				AppendMenu(loadMenus[menuItem], theString);
			}
		}
		Anim_WaitCursor();
	}
#endif
}

void Init_SamplerWindow()
{
	UseResFile(mainResFile);
	mainWin = GetNewCWindow(1000, nil, (WindowPtr) -1);
	SetGWorld((CGrafPtr) mainWin, nil);
	
	samplerScrollBar = GetNewControl(1000, mainWin);
	SizeControl(samplerScrollBar, samplerWide, 16);
	SetCtlValue(samplerScrollBar, 0);
	SetCtlMax(samplerScrollBar, thePrefs.nbSamples - nbShowSamples);
	
	SizeWindow(mainWin, samplerWide, 286, true);
	MoveWindow(mainWin, thePrefs.smpWinPos.h, thePrefs.smpWinPos.v, false);
	ShowWindow(mainWin);
	thePrefs.samplesWin = true;
	
	NoPurgePixels(GetGWorldPixMap((CGrafPort*) mainWin));
	LockPixels(GetGWorldPixMap((CGrafPort*) mainWin));
	
	TextFont(200);
	TextSize(9);
	GetBackColor(&theBackColor);
	
	SelectWindow(mainWin);
}

void Init_RecorderWindow()
{
	UseResFile(mainResFile);
	recordWin = GetNewCWindow(2000, nil, (WindowPtr) -1);
	SetGWorld((CGrafPtr) recordWin, nil);
	
	MoveWindow(recordWin, thePrefs.recWinPos.h, thePrefs.recWinPos.v, false);
	if(thePrefs.instrumentsWin) {
		ShowWindow(recordWin);
		CheckItem(menu[3], 1, true);
	}
	
	NoPurgePixels(GetGWorldPixMap((CGrafPort*) recordWin));
	LockPixels(GetGWorldPixMap((CGrafPort*) recordWin));
	
	TextFont(200);
	TextSize(9);
	RGBForeColor(&thePrefs.displayColor);
	
	//SelectWindow(recordWin);
}

void Init_SynthetizerWindow()
{
	UseResFile(mainResFile);
	instrumentsWin = GetNewCWindow(3000, nil, (WindowPtr) -1);
	SetGWorld((CGrafPtr) instrumentsWin, nil);
	
	if(thePrefs.insWinExt)
	SizeWindow(instrumentsWin, 288, 150, true);
	else
	SizeWindow(instrumentsWin, 205, 150, true);
	
	MoveWindow(instrumentsWin, thePrefs.insWinPos.h, thePrefs.insWinPos.v, false);
	if(thePrefs.instrumentsWin) {
		ShowWindow(instrumentsWin);
		CheckItem(menu[3], 2, true);
	}
	
	NoPurgePixels(GetGWorldPixMap((CGrafPort*) instrumentsWin));
	LockPixels(GetGWorldPixMap((CGrafPort*) instrumentsWin));
	
	TextFont(200);
	TextSize(9);
	RGBForeColor(&thePrefs.sampleColor);
	
	BlockMove(&insListRect, &listRect, sizeof(Rect));
	listRect.right = listRect.right - 15;
	SetRect(&listSize, 0, 0, 1, maxNbInstruments);
	theCell.h = theCell.v = 0;
	instrumentsList = LNew(&listRect, &listSize, theCell, nil, instrumentsWin, 
		true, false, false, true);
	for(theCell.v = 0; theCell.v < maxNbInstruments; ++theCell.v)
	LSetCell(&samples[theCell.v].name[1], samples[theCell.v].name[0], theCell, instrumentsList);
	theCell.v = activeInstrument;
	LSetSelect(true, theCell, instrumentsList);
	
	SelectWindow(instrumentsWin);
}

void Init_RythmsBoxWindow()
{
	UseResFile(mainResFile);
	rythmsWin = GetNewCWindow(6000, nil, (WindowPtr) -1);
	SetGWorld((CGrafPtr) rythmsWin, nil);
	
	if(thePrefs.rytWinExt)
	SizeWindow(rythmsWin, 320, 150, true);
	else
	SizeWindow(rythmsWin, 320, 41, true);
	
	MoveWindow(rythmsWin, thePrefs.rytWinPos.h, thePrefs.rytWinPos.v, false);
	if(thePrefs.rythmsWin) {
		ShowWindow(rythmsWin);
		CheckItem(menu[3], 3, true);
	}
	
	NoPurgePixels(GetGWorldPixMap((CGrafPort*) rythmsWin));
	LockPixels(GetGWorldPixMap((CGrafPort*) rythmsWin));
	
	TextFont(200);
	TextSize(9);
	GetBackColor(&theBackColor2);
	RGBForeColor(&thePrefs.sampleColor);
	
	SelectWindow(rythmsWin);
}

typedef OSErr (*IEPI_Proc)(editorPlugInInfosPtr, short, short, record*);

void Init_EditorPlugIns()
{
	short			idx,
					idx2,
					tempID;
	CInfoPBRec		cipbr,
					cipbr2;
	HFileInfo		*fpb = (HFileInfo*) &cipbr,
					*fpb2 = (HFileInfo*) &cipbr2;
	DirInfo			*dpb = (DirInfo*) &cipbr;
	ParamBlockRec	theParam;
	Str63			plugInsFolder;
					//volumeName;
	Str255			errorString,
					initRoutineName;
	FSSpec			fileSpec;
	Ptr				thePtr;
	editorPlugInInfos	plugInInfos;
	SymClass		theClass;
	IEPI_Proc		theProcPtr;
	
	editorPlugInsNumber = 0;
	currentEditor = -1;
	for(i = 0; i < kMaxEditorPlugsIns; ++i) {
		editorPlugIns[i].resFile = -1;
		editorPlugIns[i].update = true;
	}
	
	//Get the name of the Plug-Ins folder
	GetIndString(plugInsFolder, 0, 21);
#if 0
	BlockMove(thePrefs.programLocation.name, volumeName, sizeof(Str63));
	++volumeName[0];
	volumeName[volumeName[0]] = ':';
	
	//R�cup�re VRefNum d'apr�s nom volume
	theParam.volumeParam.ioCompletion	= nil;
	theParam.volumeParam.ioVolIndex		= -1;
	theParam.volumeParam.ioNamePtr		= volumeName;
	theParam.volumeParam.ioVRefNum		= 0;
	PBGetVInfo(&theParam, false);
	thePrefs.programLocation.vRefNum = theParam.volumeParam.ioVRefNum;
#else
	short vRefNum;
	long dirID;
	
	//R�cup�re le dossier de l'application:
	HGetVol(nil, &vRefNum, &dirID);
#endif
	
	//Scan dossier HF
	fpb->ioVRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
	fpb->ioNamePtr = theString;
	fpb2->ioVRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
	fpb2->ioNamePtr = theString;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = dirID;//thePrefs.programLocation.parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		
		//Scan dossier Plug-Ins
		if(fpb->ioFlAttrib & 16 && EqualString(theString, plugInsFolder, false, false)) {
			for(idx2 = 1; true; ++idx2) {
				fpb2->ioDirID = dpb->ioDrDirID;
				fpb2->ioFDirIndex = idx2;
				if(PBGetCatInfo(&cipbr2, false))
				break;
				
				//Charge Plug-In
				if(fpb2->ioFlFndrInfo.fdType == 'EdPI') {
					fileSpec.vRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
					fileSpec.parID = dpb->ioDrDirID;
					BlockMove(theString, fileSpec.name, sizeof(Str63));
					if(!GetDiskFragment(&fileSpec, 0, 0, nil, kLoadNewCopy, 
					 &editorPlugIns[editorPlugInsNumber].connID, &thePtr, errorString)) {
						
						//Init Plug-In
						GetIndString(initRoutineName, 0, 33);
						if(FindSymbol(editorPlugIns[editorPlugInsNumber].connID, initRoutineName, (char**) &theProcPtr, &theClass))
						CloseConnection(&editorPlugIns[editorPlugInsNumber].connID);
						else {
							tempID = FSpOpenResFile(&fileSpec, fsRdWrPerm);
							if(tempID == -1)
							CloseConnection(&editorPlugIns[editorPlugInsNumber].connID);
							else {
							
								theError = (*theProcPtr)(&plugInInfos, tempID, mainResFile, tempRec);
								if(theError || plugInInfos.minimumVersion > curVersion)
								CloseConnection(&editorPlugIns[editorPlugInsNumber].connID);
								else {
									InsMenuItem(menu[4], plugInInfos.name, 32000);
									if(currentEditor == -1) {
										currentEditor = 0;
										CheckItem(menu[4], 1, true);
									}
									
									++editorPlugInsNumber;
								}
								
							}
						}
					}
				}
				Anim_WaitCursor();
				
			}
		}
	}
}
	
void Init_Driver()
{
	GetDefaultOutputVolume(&oldOutputVol);
	SetDefaultOutputVolume(thePrefs.outputVol);
	
	for(i = 0; i < maxNbSamples; ++i) {
		samples[i].fileID			= 0;
		samples[i].type				= sampleType;
		samples[i].sndNb			= 0;
		samples[i].totalSndNb		= 0;
		samples[i].snd				= nil; //!
		samples[i].chan				= nil; //!
		GetIndString(samples[i].name, 0, 10);
		samples[i].spec.vRefNum		= 0;
		samples[i].spec.parID		= 0;
		samples[i].spec.name[0]		= 0;
	}
	for(i = 0; i < maxNbInstruments; ++i) {
		instruments[i].fileID		= 0;
		instruments[i].snd			= nil;
		GetIndString(instruments[i].name, 0, 11);
		instruments[i].spec.vRefNum	= 0;
		instruments[i].spec.parID	= 0;
		instruments[i].spec.name[0]	= 0;
	}
	for(i = 0; i < kPatternParts; ++i) {
		rythms[i].fileID			= 0;
		rythms[i].snd				= nil;
		GetIndString(rythms[i].name, 0, 16);
		rythms[i].spec.vRefNum		= 0;
		rythms[i].spec.parID		= 0;
		rythms[i].spec.name[0]		= 0;
	}
	for(i = 0; i < maxNbInsChannels; ++i) //!
	insChannels[i] = nil;
	for(i = 0; i < kMaxRythmsChan; ++i) //!
	rytChannels[i] = nil;
	
	Anim_WaitCursor();
	
	for(i = 0; i < thePrefs.nbSamples; ++i) {
		thePrefs.SStatus[i]			= initStereo;
		thePrefs.SSndNb[i]			= 0;
		thePrefs.SVol[i]			= defaultMaxVolume;
		thePrefs.SPan[i]			= 100;
		thePrefs.SRate[i]			= 0;
		thePrefs.SRelativeRate[i]	= 0;
		thePrefs.SLoop[i]			= 0;
		thePrefs.SPlaying[i]		= false;
		
		thePrefs.SMaxVol[i]			= defaultMaxVolume;
		
#if defined(powerc) || defined (__powerc)
		thePrefs.SSourceMode[i]		= kSSpSourceMode_Unfiltered;
#endif
		
		SCD_CreateTrack(i);
	}
	
	Anim_WaitCursor();
	
	thePrefs.IVol = defaultMaxVolume;
	thePrefs.IMaxVol = defaultMaxVolume;
	theCommand.cmd		= volumeCmd;
	theCommand.param1	= 0;
	theCommand.param2	= Shorts_To_Long(thePrefs.IVol, thePrefs.IVol);
	for(i = 0; i < thePrefs.nbInsChannels; ++i) {
		theError = SndNewChannel(&insChannels[i], sampledSynth, initStereo, nil);
		if(theError) {
			Do_Error(theError, 504);
			break;
		}
		SndDoImmediate(insChannels[i], &theCommand);
	}
	SCD_SetSynthetizerMode(thePrefs.synthMode);
	/*if(thePrefs.synthMode == 1) {
		for(i = 0; i < thePrefs.nbInsChannels; ++i) {
			theError = SndNewChannel(&insChannels[i], sampledSynth, initStereo, nil);
			if(theError) {
				Do_Error(theError, 504);
				break;
			}
			SndDoImmediate(insChannels[i], &theCommand);
		}
	}
	else {
		for(i = 0; i < thePrefs.nbInsChannels; i = i + 2) {
			theError = SndNewChannel(&insChannels[i], sampledSynth, initChanLeft, nil);
			if(theError) {
				Do_Error(theError, 504);
				break;
			}
			SndDoImmediate(insChannels[i], &theCommand);
		}
		for(i = 1; i < thePrefs.nbInsChannels; i = i + 2) {
			theError = SndNewChannel(&insChannels[i], sampledSynth, initChanRight, nil);
			if(theError) {
				Do_Error(theError, 504);
				break;
			}
			SndDoImmediate(insChannels[i], &theCommand);
		}
	}*/
	
	thePrefs.RPlaying			= false;
	thePrefs.RBPM				= 120;
	thePrefs.RCurrent			= 0;
	thePrefs.RNext				= 0;
	thePrefs.RVol				= defaultMaxVolume;
	thePrefs.RMaxVol			= defaultMaxVolume;
	for(i = 0; i < kPatternParts; ++i) {
		thePrefs.RPartVol[i]	= kDefaultPartVolume;
		thePrefs.RPartPan[i]	= 100;
		thePrefs.RPartPitch[i]	= baseKey;
		thePrefs.RPartMute[i]	= false;
	}
	interTime					= -(15000000 / thePrefs.RBPM);
	patternPos					= -1;
	
	Anim_WaitCursor();
	
	theCommand.cmd		= volumeCmd;
	theCommand.param1	= 0;
	theCommand.param2	= Shorts_To_Long(defaultMaxVolume, defaultMaxVolume);
	for(i = 0; i < thePrefs.nbRytChannels; ++i) {
		theError = SndNewChannel(&rytChannels[i], sampledSynth, initStereo, nil);
		if(theError) {
			Do_Error(theError, 504);
			break;
		}
		SndDoImmediate(rytChannels[i], &theCommand);
	}
}

void Initialization()
{
	short*		globalEventMaskPtr = (short*) 0x0144;
	
	whereDLG.h = 100;
	whereDLG.v = 100;
	whereToShow.h = -1;
	whereToShow.v = -1;
	musicFileSpec.vRefNum = 0;
	musicFileSpec.parID = 0;
	typeList_3[0] = 'Msic';
	typeList_3[1] = 'CMus';
	typeList_4[0] = 'Patt';
	for(i = 0; i < maxNbSamples; ++i)
	uc[i] = -1;
	for(i = 0; i < nbShowSamples; ++i) {
		c[i] = i;
		uc[i] = i;
	}
	for(i = 0; i < maxNbInsChannels; ++i) {
		currentNotes[i][0] = -1;
		currentNotes[i][1] = -1;
	}
	GetIndString(patternName, 0, 15);
	
	globalEventMask = *globalEventMaskPtr;
	SetEventMask(everyEvent);
	
	SetRect(&shellRect, 0, 0, samplerWide, 270);
	SetRect(&nameRect, 0, 1, samplerWide, 18);
	SetRect(&playRect, 0, 251, samplerWide, 268);
	SetRect(&volumeRect, 0, 82, samplerWide, 208);
	SetRect(&pitchRect, 0, 213, samplerWide, 229);
	SetRect(&panoramicRect, 0, 231, samplerWide, 247);
	SetRect(&optionsRect, 0, 41, samplerWide, 78);
	
	SetRect(&recordRect, 0, 0, 159, 41);
	SetRect(&timeCodeRect, 3, 3, 100, 14); //155
	SetRect(&recRecRect, 40, 22, 79, 40);
	SetRect(&recEditRect, 1, 22, 40, 40);
	SetRect(&recStopRect, 79, 22, 118, 40);
	SetRect(&recPlayRect, 118, 22, 157, 40);
	
	SetRect(&instrumentsRect, 0, 0, 288, 150);
	SetRect(&insListRect, 6, 22, 199, 144);
	SetRect(&insVolumeRect, 207, 32, 280, 148);
	SetRect(&insLoadRect, 0, 0, 0, 0);
	SetRect(&insOptionsRect, 207, 4, 280, 31);
	
	SetRect(&rythmsRect, 0, 0, 320, 150);
	SetRect(&rytLoadRect, 1, 1, 78, 18);
	SetRect(&rytPlayRect, 1, 22, 78, 39);
	SetRect(&rytCurrentRect, 105, 19, 155, 38);
	SetRect(&rytNextRect, 187, 19, 237, 38);
	SetRect(&rytOptionsRect, 264, 2, 316, 38);
	SetRect(&rytNamesRect, 1, 47, 64, 147);
	SetRect(&rytMuteRect, 66, 47, 76, 147);
	SetRect(&rytPatternRect, 78, 47, 238, 147);
	SetRect(&rytPanoramicRect, 0, 0, 0, 0);
	SetRect(&rytVolumeRect, 240, 42, 317, 148);
}

void Init_GWorlds()
{
	PicHandle		thePic;
	short			idx,
					idx2,
					interfaceFileID;
	CInfoPBRec		cipbr,
					cipbr2;
	HFileInfo		*fpb = (HFileInfo*) &cipbr,
					*fpb2 = (HFileInfo*) &cipbr2;
	DirInfo			*dpb = (DirInfo*) &cipbr;
	ParamBlockRec	theParam;
	Str63			plugInsFolder;
					//volumeName;
	FSSpec			fileSpec;
	
	//Look:
	//Get the name of the Plug-Ins folder
	GetIndString(plugInsFolder, 0, 21);
#if 0
	BlockMove(thePrefs.programLocation.name, volumeName, sizeof(Str63));
	++volumeName[0];
	volumeName[volumeName[0]] = ':';
	
	//R�cup�re VRefNum d'apr�s nom volume
	theParam.volumeParam.ioCompletion	= nil;
	theParam.volumeParam.ioVolIndex		= -1;
	theParam.volumeParam.ioNamePtr		= volumeName;
	theParam.volumeParam.ioVRefNum		= 0;
	PBGetVInfo(&theParam, false);
	thePrefs.programLocation.vRefNum = theParam.volumeParam.ioVRefNum;
#else
	short vRefNum;
	long dirID;
	
	//R�cup�re le dossier de l'application:
	HGetVol(nil, &vRefNum, &dirID);
#endif
	//Scan dossier HF
	fpb->ioVRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
	fpb->ioNamePtr = theString;
	fpb2->ioVRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
	fpb2->ioNamePtr = theString;
	for(idx = 1; true; ++idx) {
		fpb->ioDirID = dirID;//thePrefs.programLocation.parID;
		fpb->ioFDirIndex = idx;
		if(PBGetCatInfo(&cipbr, false))
		break;
		
		//Scan dossier Plug-Ins
		if(fpb->ioFlAttrib & 16 && EqualString(theString, plugInsFolder, false, false)) {
			for(idx2 = 1; true; ++idx2) {
				fpb2->ioDirID = dpb->ioDrDirID;
				fpb2->ioFDirIndex = idx2;
				if(PBGetCatInfo(&cipbr2, false))
				break;
				
				//Charge Plug-In
				if(fpb2->ioFlFndrInfo.fdType == 'ItPI') {
					fileSpec.vRefNum = vRefNum;//thePrefs.programLocation.vRefNum;
					fileSpec.parID = dpb->ioDrDirID;
					BlockMove(theString, fileSpec.name, sizeof(Str63));
					
					interfaceFileID = FSpOpenResFile(&fileSpec, fsRdPerm);
					if(interfaceFileID == -1) {
						Do_Error(ResError(), 109);
						ExitToShell();
					}
					UseResFile(interfaceFileID);
					goto Load;
				}
				
			}
		}
	}
	//Si pas Interface
	Do_Error(theError, 108);
	/*if(AskForLocation())
	goto Look;
	else*/
	ExitToShell();
		
	Load:
	SetRect(&aRect, 0, 0, 940, 302);
	theError = NewGWorld(&picGWorld, 8, &aRect, nil, nil, 0L);
	if(theError) {
		Do_Error(theError, 808);
		ExitToShell();
	}
	NoPurgePixels(GetGWorldPixMap(picGWorld));
	LockPixels(GetGWorldPixMap(picGWorld));
	SetGWorld(picGWorld, nil);
	BackColor(whiteColor);
	ForeColor(blackColor);
	PaintRect(&aRect);
	
	Anim_WaitCursor();
	
	SetRect(&aRect, 0, 0, 300, 300);
	theError = NewGWorld(&tempGWorld, 8, &aRect, nil, nil, 0L);
	if(theError) {
		Do_Error(theError, 808);
		ExitToShell();
	}
	NoPurgePixels(GetGWorldPixMap(tempGWorld));
	LockPixels(GetGWorldPixMap(tempGWorld));
	SetGWorld(tempGWorld, nil);
	TextFont(200);
	TextSize(9);
	BackColor(whiteColor);
	ForeColor(blackColor);
	PaintRect(&aRect);
	
	Anim_WaitCursor();
	
	SetGWorld(picGWorld, nil);
	
	thePic = GetPicture(1000);
	SetRect(&aRect, 0, 0, 80, 270); //Sampler track
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(1100);
	SetRect(&aRect, 81, 0, 161, 270); //Sampler track - invert
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(2000);
	SetRect(&aRect, 162, 0, 450, 150); //Synthetizer
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(2100);
	SetRect(&aRect, 162, 151, 450, 301); //Synthetizer - invert
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(3000);
	SetRect(&aRect, 451, 0, 610, 41); //Recorder
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(3100);
	SetRect(&aRect, 451, 42, 610, 83); //Recorder - invert
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(4000);
	SetRect(&aRect, 611, 0, 931, 150); //Rythms Box
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(4100);
	SetRect(&aRect, 611, 151, 931, 301); //Rythms Box - invert
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	
	Anim_WaitCursor();
	
	thePic = GetPicture(100);
	SetRect(&aRect, 451, 100, 491, 118); //Green play button
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(105);
	SetRect(&aRect, 492, 119, 532, 137); //Dark Green play button
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(110);
	SetRect(&aRect, 492, 100, 532, 118); //Red record button
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(120);
	SetRect(&aRect, 533, 100, 544, 111); //Check button red
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(130);
	SetRect(&aRect, 533, 112, 544, 133); //Check button green
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(140);
	SetRect(&aRect, 533, 134, 544, 145); //Check button yellow
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(150);
	SetRect(&aRect, 451, 161, 463, 184); //Bank buttons
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(160);
	SetRect(&aRect, 464, 161, 476, 184); //Invert bank buttons
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	
	thePic = GetPicture(200);
	SetRect(&aRect, 451, 120, 471, 160); //Big fader
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	thePic = GetPicture(210);
	SetRect(&aRect, 472, 120, 486, 136); //Little fader
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	
	thePic = GetPicture(300);
	SetRect(&aRect, 451, 185, 462, 196); //RB Quantize Mute
	DrawPicture(thePic, &aRect);
	ReleaseResource((Handle) thePic);
	
	CloseResFile(interfaceFileID);
	UseResFile(mainResFile);
}