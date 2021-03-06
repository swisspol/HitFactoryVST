#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

#include			"Record Codes.h"

//CONSTANTES LOCALES:

#define			kMaxText			3000
#define			kMaxText_2			800
#define			kMove				10
#define			kMoveHand			10

// FONCTIONS:

void Show_AboutDialog()
{
	Handle		textItem,
				textHandle;
	Str255		aString;
	long		size_1,
				size_2,
				size_3;
	TEHandle	textEdit;
	
	size_1 = FreeMem() / 1000;
	size_2 = (GetApplLimit() - (Ptr) GetZone())/ 1000;
	size_3 = MaxBlock() / 1000;
	
	GetPort(&savePort);
	UseResFile(mainResFile);
	theDialog = GetNewDialog(1000, nil, (WindowPtr) -1);
	SetGWorld((CGrafPtr) theDialog, nil);
	DrawDialog(theDialog);
	TextFont(200);
	TextSize(10);
	ForeColor(whiteColor);
	BackColor(blackColor);
	GetIndString(theString, 800, 5);
	MoveTo(8, 264);
	DrawString(theString);
	GetIndString(theString, 800, 6);
	MoveTo(8, 277);
	DrawString(theString);
#if 0//defined(powerc) || defined (__powerc)
	GetIndString(theString, 800, 7);
	MoveTo(340, 12);
	DrawString(theString);
#endif
#if 1
	GetIndString(theString, 0, 19);
	NumToString(size_1, aString);
	BlockMove(&aString[1], &theString[theString[0] + 1], aString[0]);
	theString[0] = theString[0] + aString[0] + 3;
	theString[theString[0] - 2] = ' ';
	theString[theString[0] - 1] = '/';
	theString[theString[0]] = ' ';
	NumToString(size_2, aString);
	BlockMove(&aString[1], &theString[theString[0] + 1], aString[0]);
	theString[0] = theString[0] + aString[0] + 3;
	theString[theString[0] - 2] = ' ';
	theString[theString[0] - 1] = 'K';
	theString[theString[0]] = 'b';
	MoveTo(30, 10);
	DrawString(theString);
	
	GetIndString(theString, 0, 20);
	NumToString(size_3, aString);
	BlockMove(&aString[1], &theString[theString[0] + 1], aString[0]);
	theString[0] = theString[0] + aString[0] + 3;
	theString[theString[0] - 2] = ' ';
	theString[theString[0] - 1] = 'K';
	theString[theString[0]] = 'b';
	MoveTo(270, 10);
	DrawString(theString);
#endif
	
	GetDItem(theDialog, 2, nil, &textItem, &aRect);
	textHandle = Get1Resource('TEXT', 2000);
	DetachResource(textHandle);
	HLock(textHandle);
	//SetIText(textItem, thePrefs.userName);
	destRect = aRect;
	size_1 = GetHandleSize(textHandle) * 5.5 + aRect.right - aRect.left;
	destRect.right = size_1;
	textEdit = TENew(&destRect, &aRect);
	TESetText(*textHandle, GetHandleSize(textHandle), textEdit);
	TEScroll(aRect.right - aRect.left, 0, textEdit);
	//TEUpdate(&aRect, textEdit);
	
	size_2 = 0;
	while(!Button()) {
		Delay(1, &size_3);
		TEScroll(-1, 0, textEdit);
		++size_2;
		if(size_2 > size_1) {
			size_2 = 0;
			TEScroll(size_1, 0, textEdit);
		}
	}
	
	HUnlock(textHandle);
	DisposeHandle(textHandle);
	TEDispose(textEdit);
	DisposeDialog(theDialog);
	SetGWorld((CGrafPtr) savePort, nil);
}

void Show_Pub()
{
	short				itemType;
	Boolean			loop = true;
	CCrsrHandle			handCursor,
						handCursor2;
	EventRecord			theEvent;
	Handle				textHandle,
						dialogItem;
	StScrpHandle		stylHandle;
	short				curPos = 0;
	ControlHandle		scrollBar;
	TEHandle			theTEHandle;
	Point				whereMouse,
						oldWhereMouse;
	
#if demo
	SetRect(&sourceRect, 4, 4, 300, 366);
	SetRect(&destRect, 4, 4, 300, kMaxText);
	
	//R�cup�re le dialogue
	GetPort(&savePort);
	UseResFile(mainResFile);
	theDialog = GetNewDialog(128, nil, (WindowPtr) -1);
	SetGWorld((CGrafPtr) theDialog, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	GetDItem(theDialog, 1, nil, &dialogItem, &aRect);
	handCursor = GetCCursor(128);
	handCursor2 = GetCCursor(129);
	
	//Cr�e ascenseur
	SetRect(&aRect, 302, 1, 318, 369);
	scrollBar = NewControl(theDialog, &aRect, "\p", true, 0, 0, kMaxText, 16, nil);
	
	//Cr�e la TE
	theTEHandle = TEStylNew(&destRect, &sourceRect);
	//TESetSelect(0, 32767, theTEHandle);
	//TEDelete(theTEHandle);
	//TEScroll(0, curPos, theTEHandle);
	textHandle = Get1Resource('TEXT', 128);
	HLock(textHandle);
	stylHandle = (StScrpHandle) Get1Resource('styl', 128);
	HLock((Handle) stylHandle);
	TEStylInsert(*textHandle, GetHandleSize(textHandle), stylHandle, theTEHandle);
	sourceRect.left -= 3;
	sourceRect.top -= 3;
	sourceRect.bottom += 3;
	sourceRect.right += 3;
	
	//Update
	TEUpdate(&sourceRect, theTEHandle);
	FrameRect(&sourceRect);
	Draw_DefaultItem(theDialog, 1);
	DrawDialog(theDialog);
	
	while(loop) {
		
		GetMouse(&whereMouse);
		if(PtInRect(whereMouse, &sourceRect))
		SetCCursor(handCursor);
		else
		SetCCursor(mouseCursor);
		
		if(WaitNextEvent(everyEvent, &theEvent, (long) 20, 0)) {
			theKey = (theEvent.message & keyCodeMask);
			if((theEvent.what == keyDown) && ((theKey == kReturnKey) || (theKey == kEnterKey))) {
				HiliteControl((ControlHandle) dialogItem, inButton);
				Delay(kEnterDelay, &ticks);
				loop = false;
			}
			else if(IsDialogEvent(&theEvent)
			&& DialogSelect(&theEvent, &whichDialog, &itemHit)
			&& whichDialog == theDialog)
			switch(itemHit) {
				
				case 1:
				loop = false;
				break;
				
				case 2:
				GlobalToLocal(&theEvent.where);
				itemType = TestControl(scrollBar, theEvent.where);
				switch(itemType) {
					
					case inThumb:
					TrackControl(scrollBar, theEvent.where, nil);
					TEScroll(0, curPos - GetCtlValue(scrollBar), theTEHandle);
					curPos = GetCtlValue(scrollBar);
					break;
					
					case inUpButton:
					HiliteControl(scrollBar, inUpButton);
					while(Button())
					if(curPos > kMove) {
						curPos -= kMove;
						TEScroll(0, kMove, theTEHandle);
						SetCtlValue(scrollBar, curPos);
					}
					else if(curPos > 0) {
						curPos -= 1;
						TEScroll(0, 1, theTEHandle);
						SetCtlValue(scrollBar, curPos);
					}
					HiliteControl(scrollBar, 0);
					break;
					
					case inDownButton:
					HiliteControl(scrollBar, inDownButton);
					while(Button())
					if(curPos < kMaxText - kMove) {
						curPos += kMove;
						TEScroll(0, -kMove, theTEHandle);
						SetCtlValue(scrollBar, curPos);
					}
					else if(curPos < kMaxText) {
						curPos += 1;
						TEScroll(0, -1, theTEHandle);
						SetCtlValue(scrollBar, curPos);
					}
					HiliteControl(scrollBar, 0);
					break;
					
					case inPageUp:
					TEScroll(0, curPos, theTEHandle);
					curPos = 0;
					SetCtlValue(scrollBar, curPos);
					break;
					
					case inPageDown:
					TEScroll(0, curPos - kMaxText, theTEHandle);
					curPos = kMaxText;
					SetCtlValue(scrollBar, curPos);
					break;
					
				}
				break;
				
				case 3:
				SetCCursor(handCursor2);
				GetMouse(&oldWhereMouse);
				while(Button()) {
					GetMouse(&whereMouse);
					
					if(whereMouse.v < oldWhereMouse.v)
					if(curPos < kMaxText - kMoveHand) {
						curPos += kMoveHand;
						TEScroll(0, -kMoveHand, theTEHandle);
						SetCtlValue(scrollBar, curPos);
					}
					else if(curPos < kMaxText) {
						curPos += 1;
						TEScroll(0, -1, theTEHandle);
						SetCtlValue(scrollBar, curPos);
					}
					
					if(whereMouse.v > oldWhereMouse.v)
					if(curPos > kMoveHand) {
						curPos -= kMoveHand;
						TEScroll(0, kMoveHand, theTEHandle);
						SetCtlValue(scrollBar, curPos);
					}
					else if(curPos > 0) {
						curPos -= 1;
						TEScroll(0, 1, theTEHandle);
						SetCtlValue(scrollBar, curPos);
					}
					
					oldWhereMouse = whereMouse;
				}
				SetCCursor(handCursor);
				break;
				
			}
		}
	}
	
	TEDispose(theTEHandle);
	HUnlock(textHandle);
	ReleaseResource(textHandle);
	HUnlock((Handle) stylHandle);
	ReleaseResource((Handle) stylHandle);
	DisposeControl(scrollBar);
	SetCCursor(mouseCursor);
	ReleaseResource((Handle) handCursor);
	ReleaseResource((Handle) handCursor2);
	DisposeDialog(theDialog);
	SetGWorld((CGrafPtr) savePort, nil);
#else
	return;
#endif
}
	
void Show_QuickHelp()
{
	Handle		aHandle;
	Rect		textRect;
	
	SetDAFont(3);
	GetPort(&savePort);
	UseResFile(mainResFile);
	theDialog = GetNewDialog(5000, nil, (WindowPtr) -1);
	SetDialogDefaultItem(theDialog, 1);
	
	SetGWorld((CGrafPtr) theDialog, nil);
	TextSize(10);
	TextFace(1);
	
	OutLine_Item(theDialog, 3);
	aHandle = Get1Resource('TEXT', 100);
	HLock(aHandle);
	GetDItem(theDialog, 3, nil, nil, &textRect);
	textRect.top = textRect.top + 10;
	textRect.left = textRect.left + 4;
	textRect.bottom = textRect.bottom - 4;
	textRect.right = textRect.right - 4;
	TextBox(*aHandle, GetHandleSize(aHandle), &textRect, 0);
	HUnlock(aHandle);
	ReleaseResource(aHandle);
	
	OutLine_Item(theDialog, 5);
	aHandle = Get1Resource('TEXT', 200);
	HLock(aHandle);
	GetDItem(theDialog, 5, nil, nil, &textRect);
	textRect.top = textRect.top + 10;
	textRect.left = textRect.left + 4;
	textRect.bottom = textRect.bottom - 4;
	textRect.right = textRect.right - 4;
	TextBox(*aHandle, GetHandleSize(aHandle), &textRect, 0);
	HUnlock(aHandle);
	ReleaseResource(aHandle);
	
	OutLine_Item(theDialog, 7);
	aHandle = Get1Resource('TEXT', 300);
	HLock(aHandle);
	GetDItem(theDialog, 7, nil, nil, &textRect);
	textRect.top = textRect.top + 10;
	textRect.left = textRect.left + 4;
	textRect.bottom = textRect.bottom - 4;
	textRect.right = textRect.right - 4;
	TextBox(*aHandle, GetHandleSize(aHandle), &textRect, 0);
	HUnlock(aHandle);
	ReleaseResource(aHandle);
	
	SetGWorld((CGrafPtr) savePort, nil);
	
	ModalDialog(nil, &itemHit);
	DisposeDialog(theDialog);
	SetDAFont(0);
}

short Register()
{
	GrafPtr				savePort_2;
	DialogPtr			theDialog_2;
	short				itemType;
	Boolean			loop = true,
						ok = false;
	CCrsrHandle			handCursor,
						handCursor2;
	EventRecord			theEvent;
	Handle				textHandle,
						dialogItem;
	StScrpHandle		stylHandle;
	short				curPos = 0;
	ControlHandle		scrollBar;
	TEHandle			theTEHandle;
	Point				whereMouse,
						oldWhereMouse;
	Rect				sourceRect_2;
	Handle				codeBox,
						nameBox;
	Str255				tempString;
	long				nb1,
						nb2,
						nb;
	
	SetRect(&sourceRect_2, 7, 7, 427, 249);
	SetRect(&destRect, 7, 7, 427, kMaxText_2);
	
	//R�cup�re le dialogue
	GetPort(&savePort_2);
	UseResFile(mainResFile);
	theDialog_2 = GetNewDialog(129, nil, (WindowPtr) -1);
	SetGWorld((CGrafPtr) theDialog_2, nil);
	TextFont(3);
	TextSize(10);
	TextFace(1);
	GetDItem(theDialog_2, 1, nil, &dialogItem, &aRect);
	GetDItem(theDialog_2, 3, nil, &codeBox, &aRect);
	GetDItem(theDialog_2, 4, nil, &nameBox, &aRect);
	SelIText(theDialog_2, 3, 0, 32767);
	handCursor = GetCCursor(128);
	handCursor2 = GetCCursor(129);
	
	//Cr�e ascenseur
	SetRect(&aRect, 429, 4, 445, 252);
	scrollBar = NewControl(theDialog_2, &aRect, "\p", true, 0, 0, kMaxText_2, 16, nil);
	
	//Cr�e la TE
	theTEHandle = TEStylNew(&destRect, &sourceRect_2);
	//TESetSelect(0, 32767, theTEHandle);
	//TEDelete(theTEHandle);
	//TEScroll(0, curPos, theTEHandle);
	textHandle = Get1Resource('TEXT', 129);
	HLock(textHandle);
	stylHandle = (StScrpHandle) Get1Resource('styl', 129);
	HLock((Handle) stylHandle);
	TEStylInsert(*textHandle, GetHandleSize(textHandle), stylHandle, theTEHandle);
	sourceRect_2.left -= 3;
	sourceRect_2.top -= 3;
	sourceRect_2.bottom += 3;
	sourceRect_2.right += 3;
	
	//Update
	TEUpdate(&sourceRect_2, theTEHandle);
	FrameRect(&sourceRect_2);
	OutLine_Item(theDialog_2, 7);
	DrawDialog(theDialog_2);
	
	while(loop) {
		
		GetMouse(&whereMouse);
		if(PtInRect(whereMouse, &sourceRect_2))
		SetCCursor(handCursor);
		else
		SetCCursor(mouseCursor);
		
		if(WaitNextEvent(everyEvent, &theEvent, (long) 20, 0)
			&& IsDialogEvent(&theEvent)
			&& DialogSelect(&theEvent, &whichDialog, &itemHit)
			&& whichDialog == theDialog_2)
		switch(itemHit) {
			
			case 1:
			GetIText(codeBox, tempString);
			if(tempString[0] != 13) {
				SysBeep(0);
				Do_Error(0, 903);
				SelIText(theDialog_2, 3, 0, 32767);
				TEUpdate(&sourceRect_2, theTEHandle);
				FrameRect(&sourceRect_2);
				OutLine_Item(theDialog_2, 7);
				DrawDialog(theDialog_2);
				break;
			}
			tempString[0] = 5;
			StringToNum(tempString, &nb1);
			tempString[6] = 0;
			BlockMove(&tempString[6], &nb, sizeof(long));
			BlockMove(&tempString[11], &tempString[1], 3);
			tempString[0] = 3;
			StringToNum(tempString, &nb2);
			
			if(nb1 * nb2 == nb) {
				GetIText(codeBox, thePrefs.registed);
				GetIText(nameBox, thePrefs.userName);
				Update_Pref();
				if(recorded) {
					EnableItem(menu[1], 5);
					EnableItem(menu[1], 6);
				}
				loop = false;
				ok = true;
			}
			else {
				SysBeep(0);
				Do_Error(0, 903);
				SelIText(theDialog_2, 3, 0, 32767);
				TEUpdate(&sourceRect_2, theTEHandle);
				FrameRect(&sourceRect_2);
				OutLine_Item(theDialog_2, 7);
				DrawDialog(theDialog_2);
			}
			break;
			
			case 2:
			loop = false;
			break;
			
			case 5:
			GlobalToLocal(&theEvent.where);
			itemType = TestControl(scrollBar, theEvent.where);
			switch(itemType) {
				
				case inThumb:
				TrackControl(scrollBar, theEvent.where, nil);
				TEScroll(0, curPos - GetCtlValue(scrollBar), theTEHandle);
				curPos = GetCtlValue(scrollBar);
				break;
				
				case inUpButton:
				HiliteControl(scrollBar, inUpButton);
				while(Button())
				if(curPos > kMove) {
					curPos -= kMove;
					TEScroll(0, kMove, theTEHandle);
					SetCtlValue(scrollBar, curPos);
				}
				else if(curPos > 0) {
					curPos -= 1;
					TEScroll(0, 1, theTEHandle);
					SetCtlValue(scrollBar, curPos);
				}
				HiliteControl(scrollBar, 0);
				break;
				
				case inDownButton:
				HiliteControl(scrollBar, inDownButton);
				while(Button())
				if(curPos < kMaxText_2 - kMove) {
					curPos += kMove;
					TEScroll(0, -kMove, theTEHandle);
					SetCtlValue(scrollBar, curPos);
				}
				else if(curPos < kMaxText_2) {
					curPos += 1;
					TEScroll(0, -1, theTEHandle);
					SetCtlValue(scrollBar, curPos);
				}
				HiliteControl(scrollBar, 0);
				break;
				
				case inPageUp:
				TEScroll(0, curPos, theTEHandle);
				curPos = 0;
				SetCtlValue(scrollBar, curPos);
				break;
				
				case inPageDown:
				TEScroll(0, curPos - kMaxText_2, theTEHandle);
				curPos = kMaxText_2;
				SetCtlValue(scrollBar, curPos);
				break;
				
			}
			break;
			
			case 6:
			SetCCursor(handCursor2);
			GetMouse(&oldWhereMouse);
			while(Button()) {
				GetMouse(&whereMouse);
				
				if(whereMouse.v < oldWhereMouse.v)
				if(curPos < kMaxText_2 - kMoveHand) {
					curPos += kMoveHand;
					TEScroll(0, -kMoveHand, theTEHandle);
					SetCtlValue(scrollBar, curPos);
				}
				else if(curPos < kMaxText_2) {
					curPos += 1;
					TEScroll(0, -1, theTEHandle);
					SetCtlValue(scrollBar, curPos);
				}
				
				if(whereMouse.v > oldWhereMouse.v)
				if(curPos > kMoveHand) {
					curPos -= kMoveHand;
					TEScroll(0, kMoveHand, theTEHandle);
					SetCtlValue(scrollBar, curPos);
				}
				else if(curPos > 0) {
					curPos -= 1;
					TEScroll(0, 1, theTEHandle);
					SetCtlValue(scrollBar, curPos);
				}
				
				oldWhereMouse = whereMouse;
			}
			SetCCursor(handCursor);
			break;
			
		}
	}
	
	TEDispose(theTEHandle);
	HUnlock(textHandle);
	ReleaseResource(textHandle);
	HUnlock((Handle) stylHandle);
	ReleaseResource((Handle) stylHandle);
	DisposeControl(scrollBar);
	SetCCursor(mouseCursor);
	ReleaseResource((Handle) handCursor);
	ReleaseResource((Handle) handCursor2);
	DisposeDialog(theDialog_2);
	SetGWorld((CGrafPtr) savePort_2, nil);
	
	return ok;
}
	
void Do_Error(long theError, short consCode)
{
	short		itemType,
				ID;
	Handle		textItem,
				reasonText;
	GrafPtr		oldPort;
	DialogPtr	tempDialog;
	
	GetPort(&oldPort);

	UseResFile(mainResFile);
	tempDialog = GetNewDialog(2000, nil, (WindowPtr) -1);
	SetPort(tempDialog);
	TextFont(3);
	TextSize(10);
	OutLine_Item(tempDialog, 12);
	OutLine_Item(tempDialog, 14);
	SetCCursor(mouseCursor);
	
	GetDItem(tempDialog, 2, &itemType, &textItem, &aRect); //Prgm error
	NumToString(theError, theString);
	SetIText(textItem, theString);
	GetDItem(tempDialog, 3, &itemType, &textItem, &aRect); //ResError
	NumToString(ResError(), theString);
	SetIText(textItem, theString);
	GetDItem(tempDialog, 4, &itemType, &textItem, &aRect); //MemError
	NumToString(MemError(), theString);
	SetIText(textItem, theString);
	GetDItem(tempDialog, 5, &itemType, &textItem, &aRect);
	reasonText = Get1Resource('STR ', theError); //Error meaning
	if(!reasonText)
	reasonText = Get1Resource('STR ', 0);
	SetIText(textItem, (StringPtr) *reasonText);
	GetDItem(tempDialog, 6, &itemType, &textItem, &aRect); //Error text
	ID = consCode / 100;
	GetIndString(theString, ID * 100, consCode - ID * 100);
	SetIText(textItem, theString);
	
	SetDialogDefaultItem(tempDialog, 1);
	ModalDialog(nil, &itemHit);
	DisposeDialog(tempDialog);
	
	SetPort(oldPort);
}

void Move_Left()
{
	if(c[0] > 0) {
		for(i = 0; i < maxNbSamples; ++i)
		uc[i] = -1;
		for(i = 0; i < nbShowSamples; ++i) {
			--c[i];
			uc[c[i]] = i;
		}
		SetGWorld((CGrafPtr) mainWin, nil);
		BackColor(whiteColor);
		ForeColor(blackColor);
		ScrollRect(&shellRect, 80, 0, nil);
		SetRect(&sourceRect, 0, 0, 80, 270);
		SetRect(&destRect, 0, 0, 80, 270);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
			&sourceRect, &destRect, srcCopy, nil);
		Draw_Options(0);
		Draw_Pitch(0);
		if(!thePrefs.SLoop[c[0]]) {
			if(thePrefs.SStatus[c[0]] == initStereo || thePrefs.SStatus[c[0]] == initMono)
			Draw_Panoramic(0);
		}
		Draw_Volume(0);
		if(thePrefs.SPlaying[c[0]]) {
			SetRect(&sourceRect, 451, 100, 491, 118);
			SetRect(&destRect, 39, 251, 79, 269);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
		}
		Draw_Name(0);
		Draw_Number(0);
		
		SetCtlValue(samplerScrollBar, GetCtlValue(samplerScrollBar) - 1);
	}
}

void Move_Right()
{
	if(c[nbShowSamples - 1] < thePrefs.nbSamples - 1) {
		for(i = 0; i < maxNbSamples; ++i)
		uc[i] = -1;
		for(i = 0; i < nbShowSamples; ++i) {
			++c[i];
			uc[c[i]] = i;
		}
		SetGWorld((CGrafPtr) mainWin, nil);
		BackColor(whiteColor);
		ForeColor(blackColor);
		ScrollRect(&shellRect, -80, 0, nil);
		SetRect(&sourceRect, 0, 0, 80, 270);
		SetRect(&destRect, 80 * (nbShowSamples - 1), 0, samplerWide, 270);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
			&sourceRect, &destRect, srcCopy, nil);
		Draw_Options(nbShowSamples - 1);
		Draw_Pitch(nbShowSamples - 1);
		if(!thePrefs.SLoop[c[nbShowSamples - 1]]) {
			if(thePrefs.SStatus[c[nbShowSamples - 1]] == initStereo || thePrefs.SStatus[c[nbShowSamples - 1]] == initMono)
			Draw_Panoramic(nbShowSamples - 1);
		}
		Draw_Volume(nbShowSamples - 1);
		if(thePrefs.SPlaying[c[nbShowSamples - 1]]) {
			SetRect(&sourceRect, 451, 100, 491, 118);
			SetRect(&destRect, 39 + (nbShowSamples - 1) * 80, 251, 79 + (nbShowSamples - 1) * 80, 269);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
				&sourceRect, &destRect, srcCopy, nil);
		}
		Draw_Name(nbShowSamples - 1);
		Draw_Number(nbShowSamples - 1);
		
		SetCtlValue(samplerScrollBar, GetCtlValue(samplerScrollBar) + 1);
	}
}

void Move_Max_Left()
{
	if(c[0] > 0) {
		for(i = 0; i < maxNbSamples; ++i)
		uc[i] = -1;
		for(i = 0; i < nbShowSamples; ++i) {
			c[i] = i;
			uc[c[i]] = i;
		}
		SetGWorld((CGrafPtr) mainWin, nil);
		InvalRect(&shellRect);
		SetCtlValue(samplerScrollBar, 0);
	}
}

void Move_Max_Right()
{
	if(c[nbShowSamples - 1] < thePrefs.nbSamples - 1) {
		for(i = 0; i < maxNbSamples; ++i)
		uc[i] = -1;
		for(i = 0; i < nbShowSamples; ++i) {
			c[i] = thePrefs.nbSamples - nbShowSamples + i;
			uc[c[i]] = i;
		}
		SetGWorld((CGrafPtr) mainWin, nil);
		InvalRect(&shellRect);
		SetCtlValue(samplerScrollBar, thePrefs.nbSamples);
	}
}

long Shorts_To_Long(short s1, long final)
{
	BlockMove(&s1, &final, 2);
	
	return final;
}

void Reset_Windows()
{
	/*thePrefs.mainWinPos.h = 0;
	thePrefs.mainWinPos.v = 40;
	thePrefs.recWinPos.h = 420;
	thePrefs.recWinPos.v = 350;
	thePrefs.insWinPos.h = 20;
	thePrefs.insWinPos.v = 330;*/
	thePrefs.smpWinPos.h = (dragRect.right - samplerWide) / 2;
	thePrefs.smpWinPos.v = 40;
	thePrefs.recWinPos.h = thePrefs.smpWinPos.h + 288 + (samplerWide - 288 - 159) / 2;
	thePrefs.recWinPos.v = 360;
	thePrefs.insWinPos.h = thePrefs.smpWinPos.h;
	thePrefs.insWinPos.v = 330;
	thePrefs.rytWinPos.h = thePrefs.smpWinPos.h + samplerWide - 320;
	thePrefs.rytWinPos.v = 330;
	
	MoveWindow(mainWin, thePrefs.smpWinPos.h, thePrefs.smpWinPos.v, false);
	MoveWindow(recordWin, thePrefs.recWinPos.h, thePrefs.recWinPos.v, false);
	MoveWindow(instrumentsWin, thePrefs.insWinPos.h, thePrefs.insWinPos.v, false);
	MoveWindow(rythmsWin, thePrefs.rytWinPos.h, thePrefs.insWinPos.v, false);
}

short AskForSaving()
{
#if demo
	return true;
#else
	if(!saved && recorded) {
		UseResFile(mainResFile);
		theDialog = GetNewDialog(6000, nil, (WindowPtr) -1);
		SetDialogDefaultItem(theDialog, 1);
		ModalDialog(nil, &itemHit);
		DisposeDialog(theDialog);
		if(itemHit == 2)
		return false;
		if(itemHit == 1 && musicFileSpec.parID) {
			if(isCMusFile) {
				Do_Error(-30006, 409);
				return false;
			}
			else {
				Write_Music();
				return true;
			}
		}
		if(itemHit == 1 && !musicFileSpec.parID)
			if(!Save_Music())
			return false;
	}
	
	return true;
#endif
}

pascal short SelectHook(short item, DialogPtr dialog, void* thePtr)
{
	if(GetWRefCon(WindowPtr(dialog)) != sfMainDialogRefCon)
	return sfHookNullEvent;
	
	if(item == 10)
	return sfItemOpenButton;
	
	return item;
}

UniversalProcPtr SelectHookRoutine = NewDlgHookYDProc(SelectHook);

short AskForSoundsFolder()
{
	StandardFileReply	theReply;
	ParamBlockRec		theParam;
	SFTypeList			theList;
	
	UseResFile(mainResFile);
	theList[0] = 'fold';
	CustomGetFile(nil, 1, theList, &theReply, 21000, whereToShow, SelectHookRoutine, nil, nil, nil, nil);
	if(!theReply.sfGood) {
		thePrefs.soundsFolder.vRefNum = 0;
		thePrefs.soundsFolder.parID = 0;
		thePrefs.soundsFolder.name[0] = 0;
		
		return false;
	}
	
	thePrefs.soundsFolder.vRefNum = theReply.sfFile.vRefNum;
	thePrefs.soundsFolder.parID = theReply.sfFile.parID;
	//BlockMove(theReply.sfFile.name, thePrefs.programLocation.name, sizeof(Str63));
	
	theParam.volumeParam.ioCompletion	= nil;
	theParam.volumeParam.ioVolIndex		= 0;
	theParam.volumeParam.ioNamePtr		= thePrefs.soundsFolder.name;
	theParam.volumeParam.ioVRefNum		= thePrefs.soundsFolder.vRefNum;
	PBGetVInfo(&theParam, false);
	//++thePrefs.programLocation.name[0];
	//thePrefs.programLocation.name[thePrefs.programLocation.name[0]] = ':';
	
	return true;
}

short Check_StartUpFile()
{
	FSSpec			theSpec;
	short			fileNumber,
					temp;
	long			bytesNumber;
	FInfo			theInfo;
	
	UseResFile(mainResFile);
	GetIndString(theSpec.name, 0, 14);
	
	//R�cup�re le dossier de l'application:
	HGetVol(nil, &theSpec.vRefNum, &theSpec.parID);
	
	//theSpec.parID = thePrefs.programLocation.parID;
	//theSpec.vRefNum = thePrefs.programLocation.vRefNum;
	
	if(!FSpGetFInfo(&theSpec, &theInfo))
	Open_Music(&theSpec);
	
	return true;
}