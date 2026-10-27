#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

//FONCTIONS:

void Init_ColorsDialog()
{
	//Récupère le dialogue
	UseResFile(mainResFile);
	theDialog = GetNewDialog(3700, nil, (WindowPtr) -1);
	GetDItem(theDialog, 1, nil, &dialogItem[10], &aRect);
	
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	Hide_Menus();
		
	BlockMove(&thePrefs.sampleColor, &recordPrefs.sampleColor, sizeof(RGBColor));
	BlockMove(&thePrefs.DTDColor, &recordPrefs.DTDColor, sizeof(RGBColor));
	BlockMove(&thePrefs.displayColor, &recordPrefs.displayColor, sizeof(RGBColor));
	
	SetGWorld((CGrafPtr) savePort, nil);
	dialogRunning = 5;
}

void Act_ColorsDialog(short item)
{
	RGBColor		theColor;
	
	switch(item) {
		
		case 1: //OK
		Dispose_ColorsDialog();
		break;
		
		case 2: //Cancel
		DisposeDialog(theDialog);
		for(i = 0; i < 20; ++i)
			//if(dialogItem[i])
			DisposeHandle(dialogItem[i]);
		dialogRunning = 0;
		Show_Menus();
		break;
		
		case 3: //sample color
		UseResFile(mainResFile);
		whereDLG.h = whereDLG.v = -1;
		GetIndString(theString, 0, 8);
		if(GetColor(whereDLG, theString, &recordPrefs.sampleColor, &theColor)) {
			GetPort(&savePort);
			SetGWorld((CGrafPtr) theDialog, nil);
			BlockMove(&theColor, &recordPrefs.sampleColor, sizeof(RGBColor));
			GetDItem(theDialog, 3, nil, &dialogItem[0], &aRect);
			RGBForeColor(&recordPrefs.sampleColor);
			PaintRect(&aRect);
			ForeColor(blackColor);
			FrameRect(&aRect);
			SetGWorld((CGrafPtr) savePort, nil);
		}
		break;
		
		case 5: //DTD color
		UseResFile(mainResFile);
		whereDLG.h = whereDLG.v = -1;
		GetIndString(theString, 0, 8);
		if(GetColor(whereDLG, theString, &recordPrefs.DTDColor, &theColor)) {
			GetPort(&savePort);
			SetGWorld((CGrafPtr) theDialog, nil);
			BlockMove(&theColor, &recordPrefs.DTDColor, sizeof(RGBColor));
			GetDItem(theDialog, 5, nil, &dialogItem[1], &aRect);
			RGBForeColor(&recordPrefs.DTDColor);
			PaintRect(&aRect);
			ForeColor(blackColor);
			FrameRect(&aRect);
			SetGWorld((CGrafPtr) savePort, nil);
		}
		break;
		
		case 7: //display color
		UseResFile(mainResFile);
		whereDLG.h = whereDLG.v = -1;
		GetIndString(theString, 0, 8);
		if(GetColor(whereDLG, theString, &recordPrefs.displayColor, &theColor)) {
			GetPort(&savePort);
			SetGWorld((CGrafPtr) theDialog, nil);
			BlockMove(&theColor, &recordPrefs.displayColor, sizeof(RGBColor));
			GetDItem(theDialog, 7, nil, &dialogItem[2], &aRect);
			RGBForeColor(&recordPrefs.displayColor);
			PaintRect(&aRect);
			ForeColor(blackColor);
			FrameRect(&aRect);
			SetGWorld((CGrafPtr) savePort, nil);
		}
		break;
		
		case 12:
		HMSetBalloons(!HMGetBalloons());
		break;
		
		case -1: //L'utilisateur a appuyé sur une touche
		if(theKey == kReturnKey || theKey == kEnterKey) {
			HiliteControl((ControlHandle) dialogItem[10], inButton);
			Delay(kEnterDelay, &ticks);
			//Wait(kEnterDelay);
			Dispose_ColorsDialog();
		}
		break;
		
	}
}

void Update_ColorsDialog()
{
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	BeginUpdate(theDialog);
		GetDItem(theDialog, 3, nil, &dialogItem[0], &aRect);
		RGBForeColor(&recordPrefs.sampleColor);
		PaintRect(&aRect);
		ForeColor(blackColor);
		FrameRect(&aRect);
		GetDItem(theDialog, 5, nil, &dialogItem[1], &aRect);
		RGBForeColor(&recordPrefs.DTDColor);
		PaintRect(&aRect);
		ForeColor(blackColor);
		FrameRect(&aRect);
		GetDItem(theDialog, 7, nil, &dialogItem[2], &aRect);
		RGBForeColor(&recordPrefs.displayColor);
		PaintRect(&aRect);
		ForeColor(blackColor);
		FrameRect(&aRect);
		OutLine_Item(theDialog, 9);
		Draw_DefaultItem(theDialog, 1);
		DrawDialog(theDialog);
	EndUpdate(theDialog);
	SetGWorld((CGrafPtr) savePort, nil);
}

void Dispose_ColorsDialog()
{
	BlockMove(&recordPrefs.sampleColor, &thePrefs.sampleColor, sizeof(RGBColor));
	BlockMove(&recordPrefs.DTDColor, &thePrefs.DTDColor, sizeof(RGBColor));
	BlockMove(&recordPrefs.displayColor, &thePrefs.displayColor, sizeof(RGBColor));
	
	Update_Pref();
	
	DisposeDialog(theDialog);
	for(i = 0; i < 20; ++i)
		//if(dialogItem[i])
		DisposeHandle(dialogItem[i]);
	dialogRunning = 0;
	Show_Menus();
	
	SetGWorld((CGrafPtr) mainWin, nil);
	InvalRect(&shellRect);
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
}