#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

//FONCTIONS:

void Init_QuantizeDialog()
{
	Str255			tempString;
	
	//Récupère le dialogue
	UseResFile(mainResFile);
	theDialog = GetNewDialog(3800, nil, (WindowPtr) -1);
	GetDItem(theDialog, 3, nil, &dialogItem[0], &aRect);
	GetDItem(theDialog, 4, nil, &dialogItem[1], &aRect);
	GetDItem(theDialog, 5, nil, &dialogItem[2], &aRect);
	GetDItem(theDialog, 6, nil, &dialogItem[3], &aRect);
	GetDItem(theDialog, 1, nil, &dialogItem[10], &aRect);
	
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	Hide_Menus();
		
	if(thePrefs.quantizeFlags & kQSmplStart) //Quantize sample starting
	SetCtlValue((ControlHandle) dialogItem[0], 1);
	if(thePrefs.quantizeFlags & kQSmplStop) //Quantize sample stopping
	SetCtlValue((ControlHandle) dialogItem[1], 1);
	if(thePrefs.quantizeFlags & kQSmplPitchAlign) //Align pitchs
	SetCtlValue((ControlHandle) dialogItem[2], 1);
	if(thePrefs.quantizeFlags & kQRB) //Quantize RB
	SetCtlValue((ControlHandle) dialogItem[3], 1);
	
	SetGWorld((CGrafPtr) savePort, nil);
	dialogRunning = 7;
}

void Act_QuantizeDialog(short item)
{
	RGBColor		theColor;
	
	switch(item) {
		
		case 1: //OK
		Dispose_QuantizeDialog();
		break;
		
		case 2: //Cancel
		DisposeDialog(theDialog);
		for(i = 0; i < 20; ++i)
			//if(dialogItem[i])
			DisposeHandle(dialogItem[i]);
		dialogRunning = 0;
		Show_Menus();
		break;
		
		case 3: //Check
		i = GetCtlValue((ControlHandle) dialogItem[0]);
		SetCtlValue((ControlHandle) dialogItem[0], !i);
		break;
		
		case 4: //Check
		i = GetCtlValue((ControlHandle) dialogItem[1]);
		SetCtlValue((ControlHandle) dialogItem[1], !i);
		break;
		
		case 5: //Check
		i = GetCtlValue((ControlHandle) dialogItem[2]);
		SetCtlValue((ControlHandle) dialogItem[2], !i);
		break;
		
		case 6: //Check
		i = GetCtlValue((ControlHandle) dialogItem[3]);
		SetCtlValue((ControlHandle) dialogItem[3], !i);
		break;
		
		case 12:
		HMSetBalloons(!HMGetBalloons());
		break;
		
		case -1: //L'utilisateur a appuyé sur une touche
		if(theKey == kReturnKey || theKey == kEnterKey) {
			HiliteControl((ControlHandle) dialogItem[10], inButton);
			Delay(kEnterDelay, &ticks);
			//Wait(kEnterDelay);
			Dispose_QuantizeDialog();
		}
		break;
		
	}
}

void Update_QuantizeDialog()
{
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	BeginUpdate(theDialog);
		OutLine_Item(theDialog, 7);
		OutLine_Item(theDialog, 10);
		Draw_DefaultItem(theDialog, 1);
		DrawDialog(theDialog);
	EndUpdate(theDialog);
	SetGWorld((CGrafPtr) savePort, nil);
}

void Dispose_QuantizeDialog()
{
	Str255		tempString;
	long		tempValue;
	
	//Quantize sample starting
	if(GetCtlValue((ControlHandle) dialogItem[0]))
	thePrefs.quantizeFlags = thePrefs.quantizeFlags | kQSmplStart;
	else
	thePrefs.quantizeFlags = thePrefs.quantizeFlags & (~kQSmplStart);
	//Quantize sample stopping
	if(GetCtlValue((ControlHandle) dialogItem[1]))
	thePrefs.quantizeFlags = thePrefs.quantizeFlags | kQSmplStop;
	else
	thePrefs.quantizeFlags = thePrefs.quantizeFlags & (~kQSmplStop);
	//Align pitchs
	if(GetCtlValue((ControlHandle) dialogItem[2]))
	thePrefs.quantizeFlags = thePrefs.quantizeFlags | kQSmplPitchAlign;
	else
	thePrefs.quantizeFlags = thePrefs.quantizeFlags & (~kQSmplPitchAlign);
	
	//Quantize RB
	if(GetCtlValue((ControlHandle) dialogItem[3]))
	thePrefs.quantizeFlags = thePrefs.quantizeFlags | kQRB;
	else
	thePrefs.quantizeFlags = thePrefs.quantizeFlags & (~kQRB);
	
	Update_Pref();
	
	nbSamplesToStart = 0;
	nbSamplesToStop = 0;
	nbRBMutes = 0;
	
	SetGWorld((CGrafPtr) mainWin, nil);
	InvalRect(&shellRect);
	
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
}