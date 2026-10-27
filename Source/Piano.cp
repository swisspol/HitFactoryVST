#include			<Folders.h>

#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

//VARIABLES LOCALES:

char				oldPiano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd];
short				listLength,
					pianoOffset;

//FONCTIONS:

short Create_Piano()
{
	long		prefFolderNum;
	short		volumeID;
	
	//Initialise la structure
	for(i = 0; i < pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd; ++i)
	piano[i] = 0;
	
	piano[55] = 'q';
	piano[56] = 's';
	piano[57] = 'd';
	piano[58] = 'f';
	piano[59] = 'g';
	piano[60] = 'h';
	piano[61] = 'j';
	piano[62] = 'k';
	piano[63] = 'l';
	piano[64] = 'm';
	piano[65] = '';
	
	piano[pianoSize + maxNbInstruments + maxNbSamples + 0] = '&';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 1] = 'Ž';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 2] = '\"';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 3] = '\'';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 4] = '(';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 5] = '¤';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 6] = '';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 7] = '!';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 8] = '';
	piano[pianoSize + maxNbInstruments + maxNbSamples + 9] = 'ˆ';
	
	piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 0] = ' ';
	piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 1] = 3;
	
	//CrŽe le fichier dans le dossier PrŽfŽrences
	FindFolder(kOnSystemDisk, 'pref', kDontCreateFolder, &volumeID, &prefFolderNum);
	theError = HCreate(volumeID, prefFolderNum, pianoName, 'HiFc', 'Data');
	if(theError) {
		Do_Error(theError, 301);
		return false;
	}
	Update_Piano(); //Ecrit le fichier
	
	return true;
}

short Read_Piano()
{
	short		fileNumber,
				temp,
				volumeID;
	long		bytesNumber,
				prefFolderNum;
	
	FindFolder(kOnSystemDisk, 'pref', kDontCreateFolder, &volumeID, &prefFolderNum);
	if(HOpen(volumeID, prefFolderNum, pianoName, fsRdWrPerm, &fileNumber)) //Existe pas
	return false;
	
	Anim_WaitCursor();
	
	SetFPos(fileNumber, 1, 0);
	bytesNumber = 2;
	theError = FSRead(fileNumber, &bytesNumber, &temp); //Contr™le le numŽro de version
	if(temp != pianoVersion) {
		FSClose(fileNumber); //Si mauvais: efface le fichier
		theError = HDelete(volumeID, prefFolderNum, pianoName);
		if(theError)
		Do_Error(theError, 304);
		return false;
	}
	
	Anim_WaitCursor();
	
	//Charge la structure en mŽmoire
	bytesNumber = (pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd) * sizeof(char);
	theError = FSRead(fileNumber, &bytesNumber, &piano);
	if(theError) {
		Do_Error(theError, 302);
		FSClose(fileNumber);
		return false;
	}
	FSClose(fileNumber);
	
	return true;
}

short Update_Piano()
{
	short		fileNumber,
				volumeID;
	long		bytesNumber,
				prefFolderNum;
	
	//Ouvre le fichier
	FindFolder(kOnSystemDisk, 'pref', kDontCreateFolder, &volumeID, &prefFolderNum);
	theError = HOpen(volumeID, prefFolderNum, pianoName, fsRdWrPerm, &fileNumber);
	if(theError) {
		Do_Error(theError, 303);
		return false;
	}
	
	//Ecrit le numŽro de version puis la structure
	SetFPos(fileNumber, 1, 0);
	bytesNumber = 2;
	theError = FSWrite(fileNumber, &bytesNumber, &pianoVersion);
	if(theError) {
		Do_Error(theError, 103);
		return false;
	}
	
	bytesNumber = (pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd) * sizeof(char);
	theError = FSWrite(fileNumber, &bytesNumber, &piano);
	if(theError) {
		Do_Error(theError, 103);
		return false;
	}
	FSClose(fileNumber); //Ferme le fichier
	
	return true;
}

void Display_List(short type)
{
	short		size;
	Str63		s;
	
	LDelRow(0, 0, theList);
	
	switch(type) {
		
		case 1:
		LAddRow(pianoSize + maxNbInstruments, 0, theList);
		listLength = pianoSize + maxNbInstruments;
		pianoOffset = 0;
		theCell.h = 0;
		size = 3;
		for(i = 0; i < 10; ++i) {
			theCell.v = 12 * i;
			LSetCell("C  ", 3, theCell, theList);
			++theCell.v;
			LSetCell("C# ", 3, theCell, theList);
			++theCell.v;
			LSetCell("D  ", 3, theCell, theList);
			++theCell.v;
			LSetCell("D# ", 3, theCell, theList);
			++theCell.v;
			LSetCell("E  ", 3, theCell, theList);
			++theCell.v;
			LSetCell("F  ", 3, theCell, theList);
			++theCell.v;
			LSetCell("F# ", 3, theCell, theList);
			++theCell.v;
			LSetCell("G  ", 3, theCell, theList);
			++theCell.v;
			LSetCell("G# ", 3, theCell, theList);
			++theCell.v;
			LSetCell("A  ", 3, theCell, theList);
			++theCell.v;
			LSetCell("A# ", 3, theCell, theList);
			++theCell.v;
			LSetCell("B  ", 3, theCell, theList);
			NumToString(i, s);
			for(theCell.v = 12 * i; theCell.v < 12 * i + 12; ++theCell.v) {
				LGetCell(theString, &size, theCell, theList);
				theString[2] = s[1];
				LSetCell(theString, 3, theCell, theList);
			}
		}
		BlockMove("\pInstr. ", theString, 8);
		for(i = 0; i < maxNbInstruments; ++i) {
			theCell.v = pianoSize + i;
			NumToString(i, s);
			BlockMove(&s[1], &theString[8], s[0]);
			theString[0] = 7 + s[0];
			LSetCell(&theString[1], theString[0], theCell, theList);
		}
		theCell.h = 1;
		for(theCell.v = 0; theCell.v < pianoSize + maxNbInstruments; ++theCell.v) {
			if(piano[theCell.v] == ' ')
			LSetCell("[Space]", 7, theCell, theList);
			if(piano[theCell.v] == 3)
			LSetCell("[Enter]", 7, theCell, theList);
			LSetCell(&piano[theCell.v], 1, theCell, theList);
		}
		break;
		
		case 2:
		LAddRow(maxNbSamples, 0, theList);
		listLength = maxNbSamples;
		pianoOffset = pianoSize + maxNbInstruments;
		theCell.h = 0;
		BlockMove("\pTrack ", theString, 7);
		for(theCell.v = 0; theCell.v < maxNbSamples; ++theCell.v) {
			NumToString(theCell.v, s);
			BlockMove(&s[1], &theString[7], s[0]);
			theString[0] = 6 + s[0];
			LSetCell(&theString[1], theString[0], theCell, theList);
		}
		theCell.h = 1;
		for(theCell.v = 0; theCell.v < maxNbSamples; ++theCell.v) {
			if(piano[pianoSize + maxNbInstruments + theCell.v] == ' ')
			LSetCell("[Space]", 7, theCell, theList);
			if(piano[pianoSize + maxNbInstruments + theCell.v] == 3)
			LSetCell("[Enter]", 7, theCell, theList);
			LSetCell(&piano[pianoSize + maxNbInstruments + theCell.v], 1, theCell, theList);
		}
		break;
		
		case 3:
		LAddRow(kPatternParts, 0, theList);
		listLength = kPatternParts;
		pianoOffset = pianoSize + maxNbInstruments + maxNbSamples;
		theCell.h = 0;
		BlockMove("\pPart ", theString, 7);
		for(theCell.v = 0; theCell.v < kPatternParts; ++theCell.v) {
			NumToString(theCell.v, s);
			BlockMove(&s[1], &theString[6], s[0]);
			theString[0] = 5 + s[0];
			LSetCell(&theString[1], theString[0], theCell, theList);
		}
		theCell.h = 1;
		for(theCell.v = 0; theCell.v < kPatternParts; ++theCell.v) {
			if(piano[pianoSize + maxNbInstruments + maxNbSamples + theCell.v] == ' ')
			LSetCell("[Space]", 7, theCell, theList);
			if(piano[pianoSize + maxNbInstruments + maxNbSamples + theCell.v] == 3)
			LSetCell("[Enter]", 7, theCell, theList);
			LSetCell(&piano[pianoSize + maxNbInstruments + maxNbSamples + theCell.v], 1, theCell, theList);
		}
		break;
		
		case 4:
		LAddRow(pianoAdd, 0, theList);
		listLength = pianoAdd;
		pianoOffset = pianoSize + maxNbInstruments + maxNbSamples + kPatternParts;
		theCell.h = 0;
		theCell.v = 0;
		BlockMove("\pPause", theString, 6);
		LSetCell(&theString[1], theString[0], theCell, theList);
		theCell.h = 1;
		LSetCell(&piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 0], 1, theCell, theList);
		if(piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 0] == ' ')
		LSetCell("[Space]", 7, theCell, theList);
		if(piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 0] == 3)
		LSetCell("[Enter]", 7, theCell, theList);
		
		theCell.h = 0;
		theCell.v = 1;
		BlockMove("\pR. Box", theString, 7);
		LSetCell(&theString[1], theString[0], theCell, theList);
		theCell.h = 1;
		LSetCell(&piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 1], 1, theCell, theList);
		if(piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 1] == ' ')
		LSetCell("[Space]", 7, theCell, theList);
		if(piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 1] == 3)
		LSetCell("[Enter]", 7, theCell, theList);
		break;
		
	}
	
	actualCell.h = 0;
	actualCell.v = 0;
	LSetSelect(true, actualCell, theList);
	actualCell.h = 1;
	LSetSelect(true, actualCell, theList);
}
	
void Init_PianoDialog()
{
	Rect		listSize,
				listRect;
					
	//Initialize les variables
	SetRect(&listSize, 0, 0, 2, 1);
	SetRect(&listRect, 4, 90, 104, 286);
	theCell.h = theCell.v = 0;
	
	//RŽcupre le dialogue
	UseResFile(mainResFile);
	theDialog = GetNewDialog(8000, nil, (WindowPtr) -1);
	GetDItem(theDialog, 5, nil, &dialogItem[0], &aRect);
	GetDItem(theDialog, 6, nil, &dialogItem[1], &aRect);
	GetDItem(theDialog, 1, nil, &dialogItem[10], &aRect);
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	SetCtlValue((ControlHandle) dialogItem[0], 1); //Menu
	SetCtlValue((ControlHandle) dialogItem[1], thePrefs.flags & kStopWhenKeyUp); //Stop KeyUp?
	theList = LNew(&listRect, &listSize, theCell, nil, theDialog,
		true, false, false, true);
	Hide_Menus();
	
	Display_List(1);
	
	BlockMove(piano, oldPiano, (pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd) * sizeof(char));
	
	SetGWorld((CGrafPtr) savePort, nil);
	dialogRunning = 2;
}

void Act_PianoDialog(short item)
{
	switch(item) {
		
		case 1: //OK
		Dispose_PianoDialog();
		break;
		
		case 2: //Cancel
		LDispose(theList);
		DisposeDialog(theDialog);
		for(i = 0; i < 20; ++i)
			//if(dialogItem[i])
			DisposeHandle(dialogItem[i]);
		BlockMove(oldPiano, piano, (pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd) * sizeof(char));
		dialogRunning = 0;
		Show_Menus();
		break;
		
		case 3: //Reset
		BlockMove(oldPiano, piano, (pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd) * sizeof(char));
		Display_List(GetCtlValue((ControlHandle) dialogItem[0]));
		break;
		
		case 4: //Clear All
		for(i = 0; i < pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd; ++i)
		piano[i] = 0;
		SetCtlValue((ControlHandle) dialogItem[0], 1);
		Display_List(1);
		break;
		
		case 5: //Menu
		Display_List(GetCtlValue((ControlHandle) dialogItem[0]));
		break;
		
		case 6: //Check box KeyUp
		i = GetCtlValue((ControlHandle) dialogItem[1]);
		SetCtlValue((ControlHandle) dialogItem[1], !i);
		break;
		
		case 7: //L'utilisateur a cliquŽ sur la liste
		GetPort(&savePort);
		SetGWorld((CGrafPtr) theDialog, nil);
		GlobalToLocal(&event.where);
		actualCell.h = 0;
		LSetSelect(false, actualCell, theList);
		actualCell.h = 1;
		LSetSelect(false, actualCell, theList);
		LClick(event.where, nil, theList);
		actualCell.h = 0;
		actualCell.v = 0;
		LGetSelect(true, &actualCell, theList);
		actualCell.h = 0;
		LSetSelect(true, actualCell, theList);
		actualCell.h = 1;
		LSetSelect(true, actualCell, theList);
		SetGWorld((CGrafPtr) savePort, nil);
		break;
		
		case 9:
		HMSetBalloons(!HMGetBalloons());
		break;
		
		case -1: //L'utilisateur a appuyŽ sur une touche
		switch(theKey) {
			
			case kDownArrowKey:
			actualCell.h = 0;
			LSetSelect(false, actualCell, theList);
			actualCell.h = 1;
			LSetSelect(false, actualCell, theList);
			if(actualCell.v < listLength - 1)
			++actualCell.v;
			actualCell.h = 0;
			LSetSelect(true, actualCell, theList);
			actualCell.h = 1;
			LSetSelect(true, actualCell, theList);
			LAutoScroll(theList);
			break;
			
			case kUpArrowKey:
			actualCell.h = 0;
			LSetSelect(false, actualCell, theList);
			actualCell.h = 1;
			LSetSelect(false, actualCell, theList);
			if(actualCell.v > 0)
			--actualCell.v;
			actualCell.h = 0;
			LSetSelect(true, actualCell, theList);
			actualCell.h = 1;
			LSetSelect(true, actualCell, theList);
			LAutoScroll(theList);
			break;
			
			case kDeleteKey:
			piano[pianoOffset + actualCell.v] = 0;
			LClrCell(actualCell, theList);
			break;
			
			case kReturnKey:
			HiliteControl((ControlHandle) dialogItem[10], inButton);
			Dispose_PianoDialog();
			break;
			
			default:
			theCell.h = 1;
			for(i = 0; i < pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd; ++i)
			if(piano[i] == theChar) {
				piano[i] = 0;
				if(i >= pianoOffset && i < (pianoOffset + listLength)) {
					theCell.v = i - pianoOffset;
					LClrCell(theCell, theList);
				}
			}
			piano[pianoOffset + actualCell.v] = theChar;
			LSetCell(&theChar, 1, actualCell, theList);
			if(theChar == ' ')
			LSetCell("[Space]", 7, actualCell, theList);
			if(theChar == 3)
			LSetCell("[Enter]", 7, actualCell, theList);
			break;
			
		}
		break;
		
	}
}

void Update_PianoDialog()
{
	GetPort(&savePort);
	SetGWorld((CGrafPtr) theDialog, nil);
	BeginUpdate(theDialog);
		//OutLine_Item(theDialog, 5);
		DrawDialog(theDialog);
		LUpdate((*theList)->port->visRgn, theList);
		SetRect(&aRect, 3, 89, 105, 287);
		FrameRect(&aRect);
		Draw_DefaultItem(theDialog, 1);
	EndUpdate(theDialog);
	SetGWorld((CGrafPtr) savePort, nil);
}

void Dispose_PianoDialog()
{
	Update_Piano();
	
	//Examine KeyUp
	if(GetCtlValue((ControlHandle) dialogItem[1]))
	thePrefs.flags = thePrefs.flags | kStopWhenKeyUp;
	else
	thePrefs.flags = thePrefs.flags & (~kStopWhenKeyUp);
	
	LDispose(theList);
	DisposeDialog(theDialog);
	for(i = 0; i < 20; ++i)
		//if(dialogItem[i])
		DisposeHandle(dialogItem[i]);
	dialogRunning = 0;
	Show_Menus();
}