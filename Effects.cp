#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

//FONCTIONS:

void Init_EffectsDialog()
{
	Str255			tempString;
	
#if demo
	return;
#else
	//R�cup�re le dialogue
	UseResFile(mainResFile);
	theDialog = GetNewDialog(3600, nil, (WindowPtr) -1);
	GetDItem(theDialog, 3, nil, &dialogItem[0], &aRect);
	GetDItem(theDialog, 4, nil, &dialogItem[1], &aRect);
	GetDItem(theDialog, 5, nil, &dialogItem[2], &aRect);
	GetDItem(theDialog, 6, nil, &dialogItem[3], &aRect);
	GetDItem(theDialog, 7, nil, &dialogItem[4], &aRect);
	GetDItem(theDialog, 8, nil, &dialogItem[5], &aRect);
	GetDItem(theDialog, 1, nil, &dialogItem[10], &aRect);
	
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	Hide_Menus();
		
	#if defined(powerc) || defined (__powerc)
	SetCtlValue((ControlHandle) dialogItem[0], thePrefs.SReverb); //Enable reverb?
	NumToString(thePrefs.localization.roomSize, tempString);
	SetIText(dialogItem[1], tempString);
	NumToString(thePrefs.localization.roomReflectivity, tempString);
	SetIText(dialogItem[2], tempString);
	NumToString(thePrefs.localization.reverbAttenuation, tempString);
	SetIText(dialogItem[3], tempString);
	NumToString(thePrefs.localization.referenceDistance, tempString);
	SetIText(dialogItem[4], tempString);
	NumToString(thePrefs.localization.currentLocation.distance, tempString);
	SetIText(dialogItem[5], tempString);
	SelIText(theDialog, 4, 0, 32767); //Misc
	#endif
	
	SetGWorld((CGrafPtr) savePort, nil);
	dialogRunning = 6;
#endif
}

void Act_EffectsDialog(short item)
{
	RGBColor		theColor;
	
#if demo
	return;
#else
	switch(item) {
		
		case 1: //OK
		Dispose_EffectsDialog();
		break;
		
		case 2: //Cancel
		DisposeDialog(theDialog);
		for(i = 0; i < 20; ++i)
			//if(dialogItem[i])
			DisposeHandle(dialogItem[i]);
		dialogRunning = 0;
		Show_Menus();
		break;
		
		case 3: //Case � cocher - reverb
		i = GetCtlValue((ControlHandle) dialogItem[0]);
		SetCtlValue((ControlHandle) dialogItem[0], !i);
		Do_Error(-30003, 509);
		break;
	
		case 21:
		HMSetBalloons(!HMGetBalloons());
		break;
	
		case -1: //L'utilisateur a appuy� sur une touche
		if(theKey == kReturnKey || theKey == kEnterKey) {
			HiliteControl((ControlHandle) dialogItem[10], inButton);
			Delay(kEnterDelay, &ticks);
			//Wait(kEnterDelay);
			Dispose_EffectsDialog();
		}
		break;
		
	}
#endif
}

void Update_EffectsDialog()
{
#if demo
	return;
#else
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	BeginUpdate(theDialog);
		OutLine_Item(theDialog, 14);
		OutLine_Item(theDialog, 16);
		OutLine_Item(theDialog, 18);
		Draw_DefaultItem(theDialog, 1);
		DrawDialog(theDialog);
	EndUpdate(theDialog);
	SetGWorld((CGrafPtr) savePort, nil);
#endif
}

void Dispose_EffectsDialog()
{
	Str255		tempString;
	long		tempValue;
	
#if demo
return;
#else
	#if defined(powerc) || defined (__powerc)
	i = GetCtlValue((ControlHandle) dialogItem[0]);
	thePrefs.SReverb = i;
	
	GetIText(dialogItem[1], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 1)
	tempValue = 1;
	thePrefs.localization.roomSize = tempValue;
	
	GetIText(dialogItem[2], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue > -1)
	tempValue = -1;
	thePrefs.localization.roomReflectivity = tempValue;
	
	GetIText(dialogItem[3], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 1)
	tempValue = 1;
	thePrefs.localization.reverbAttenuation = tempValue;
	
	GetIText(dialogItem[4], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 1)
	tempValue = 1;
	thePrefs.localization.referenceDistance = tempValue;
	
	GetIText(dialogItem[5], tempString);
	StringToNum(tempString, &tempValue);
	if(tempValue < 1)
	tempValue = 1;
	thePrefs.localization.currentLocation.distance = tempValue;
	#endif
	
	Update_Pref();
	
	DisposeDialog(theDialog);
	for(i = 0; i < 20; ++i)
		//if(dialogItem[i])
		DisposeHandle(dialogItem[i]);
	dialogRunning = 0;
	Show_Menus();
	
	#if defined(powerc) || defined (__powerc)
	if(thePrefs.SReverb)
	for(i = 0; i < thePrefs.nbSamples; ++i)
	SCD_EffectsSample(i);
	#endif
	
	SetGWorld((CGrafPtr) mainWin, nil);
	InvalRect(&shellRect);
#endif
}