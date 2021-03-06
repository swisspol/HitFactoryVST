#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

//CONSTANTES LOCALES:

#define				oldBarHeight		20
#define				animCursorStart		2000
#define				animCursorEnd		2007
#define				minAnimTime			6

#define				kWhite				65535
#define				kGray				44395

//VARIABLES LOCALES:

short				animCursorFrame = 32000;
short				oldResFile;
long				lastChange = TickCount();

//FONCTIONS:

void SH_ForceUpdate(RgnHandle rgn)
{
	WindowRef		wpFirst = LMGetWindowList();
	
	PaintBehind(wpFirst, rgn);
	CalcVisBehind(wpFirst, rgn);
}

void GetMBarRgn(RgnHandle mBarRgn)
{
	Rect			mBarRect;

	mBarRect = qd.screenBits.bounds;
	mBarRect.bottom = mBarRect.top + oldBarHeight;
	RectRgn(mBarRgn, &mBarRect);
}

void ToggleMBarState()
{
	RgnHandle		GrayRgn = LMGetGrayRgn();
	
	if(MBarShown) {
		mBarRgn = NewRgn();
		GetMBarRgn(mBarRgn);
		LMSetMBarHeight(0);
		UnionRgn(GrayRgn,mBarRgn,GrayRgn);
		SH_ForceUpdate(mBarRgn);
		
		UseResFile(mainResFile);
		backWin = GetNewCWindow(10000, nil, (WindowPtr) 0);
		SizeWindow(backWin, dragRect.right, dragRect.bottom, true);
	}
	else {
		LMSetMBarHeight(oldBarHeight);
		DiffRgn(GrayRgn, mBarRgn, GrayRgn);
		DisposeRgn(mBarRgn);
		DrawMenuBar();
		
		DisposeWindow(backWin);
		SetGWorld((CGrafPtr) mainWin, nil);
	}
	MBarShown = !MBarShown;
}

void Hide_Menus()
{
	HiliteMenu(0);
	
	DisableItem(menu[0], 1);
	DisableItem(menu[1], 0);
	DisableItem(menu[2], 0);
	DisableItem(menu[3], 0);
	DisableItem(menu[4], 0);
	
	DrawMenuBar();
}

void Show_Menus()
{
	EnableItem(menu[0], 1);
	EnableItem(menu[1], 0);
	EnableItem(menu[2], 0);
	EnableItem(menu[3], 0);
	EnableItem(menu[4], 0);
	
	if(recorded) {
		if(musicFileSpec.parID && !saved)
		EnableItem(menu[1], 4);
		EnableItem(menu[1], 5);
		EnableItem(menu[1], 6);
	}
	
	DrawMenuBar();
}

void Anim_WaitCursor()
{
	if(TickCount() - lastChange > minAnimTime) {
		oldResFile = CurResFile();
		UseResFile(mainResFile);
		
		++animCursorFrame;
		if(animCursorFrame > animCursorEnd)
		animCursorFrame = animCursorStart;
		DisposeCCursor(waitCursor);
		waitCursor = GetCCursor(animCursorFrame);
		SetCCursor(waitCursor);
		
		UseResFile(oldResFile);
		lastChange = TickCount();
	}
}

void Wait(long time)
{
	long old = TickCount();
	while(TickCount() - old < time)
	;
}
	
void OutLine_Item(DialogPtr theDialog, short item)
{
	Rect		itemRect;
	RGBColor	theColor,
				oldColor;
	
	GetForeColor(&oldColor);
	GetDItem(theDialog, item, nil, nil, &itemRect);
	theColor.red = theColor.blue = theColor.green = kGray;
	RGBForeColor(&theColor);
	FrameRect(&itemRect);
	OffsetRect(&itemRect, 1, 1);
	theColor.red = theColor.blue = theColor.green = kWhite;
	RGBForeColor(&theColor);
	FrameRect(&itemRect);
	RGBForeColor(&oldColor);
}

void Draw_DefaultItem(DialogPtr theDialog, short item)
{
	Rect	itemRect;
	
	GetDItem(theDialog, item, nil, nil, &itemRect);
	itemRect.top = itemRect.top - 4;
	itemRect.left = itemRect.left - 4;
	itemRect.bottom = itemRect.bottom + 4;
	itemRect.right = itemRect.right + 4;
	PenSize(3,3);
	FrameRoundRect(&itemRect, 16, 16);
	PenSize(1,1);
}

pascal short DialogHook(short item, DialogPtr dialog, openData* data)
{
	Handle				soundHandle,
						dialogItem;
	short				fileID,
						oldResFileID = CurResFile(),
						itemType;
	Rect				itemRect;
	GrafPtr				oldPort;
	SoundHeaderPtr		stdHeader;
	ExtSoundHeaderPtr	extHeader;
	Str63				text;
	long				offset;

	if(GetWRefCon(WindowPtr(dialog)) != sfMainDialogRefCon)
	return sfHookNullEvent;
	
	if(item == sfHookFirstCall) {
		NumToString(MaxBlock() / 1000, text);
		ParamText(text, nil, nil, nil);
		
		/*if(!data->extendedDLOG) {
			SizeWindow(dialog, 336, 310, true);
			HideDItem(dialog, 11);
		}
		else
		HideDItem(dialog, 12);*/
	}
	
	if(item == 10 && (data->reply->sfType == 'sfil' || data->reply->sfType == 'Bank')) { //PlaySound
		SetCCursor(waitCursor);
		fileID = FSpOpenResFile(&(data->reply->sfFile), fsRdPerm);
		if(fileID != -1) {
			UseResFile(fileID);
			soundHandle = Get1IndResource('snd ', 1);
			if(!soundHandle) {
				SysBeep(0);
				
				BlockMove("\p?", text, 2);
				GetDItem(dialog, 11, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
				GetDItem(dialog, 12, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
				GetDItem(dialog, 13, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
				GetDItem(dialog, 14, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
				GetDItem(dialog, 15, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
				GetDItem(dialog, 16, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
				
				goto Pass;
			}
			
			GetDItem(dialog, 11, &itemType, &dialogItem, &itemRect);
			GetResInfo(soundHandle, &itemType, (ResType*) &offset, text);
			SetIText(dialogItem, text);
			
			GetSoundHeaderOffset((SndListHandle) soundHandle, &offset); //Display infos
			stdHeader = (SoundHeaderPtr) (*soundHandle + offset);
			extHeader = (ExtSoundHeaderPtr) (*soundHandle + offset);
			HLock(soundHandle);
			NumToString((unsigned short) HiWord(stdHeader->sampleRate), text);
			GetDItem(dialog, 13, &itemType, &dialogItem, &itemRect);
			SetIText(dialogItem, text);
			NumToString(GetHandleSize(soundHandle) / 1000, text);
			GetDItem(dialog, 16, &itemType, &dialogItem, &itemRect);
			SetIText(dialogItem, text);
			if(stdHeader->encode == extSH) {
				NumToString(extHeader->numChannels, text);
				GetDItem(dialog, 14, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
				NumToString(extHeader->sampleSize, text);
				GetDItem(dialog, 12, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
				NumToString(1000 * extHeader->numFrames / (unsigned short) HiWord(stdHeader->sampleRate), text);
				BlockMove(&text[text[0] - 2], &text[text[0] - 1], 3);
				text[text[0] - 2] = '.';
				++text[0];
				GetDItem(dialog, 15, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
			}
			else {
				GetDItem(dialog, 14, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, "\p1");
				GetDItem(dialog, 12, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, "\p8");
				NumToString(1000 * stdHeader->length / (unsigned short) HiWord(stdHeader->sampleRate), text);
				BlockMove(&text[text[0] - 2], &text[text[0] - 1], 3);
				text[text[0] - 2] = '.';
				++text[0];
				GetDItem(dialog, 15, &itemType, &dialogItem, &itemRect);
				SetIText(dialogItem, text);
			}
			HUnlock(soundHandle);
			
			SndPlay(nil, (SndListResource**) soundHandle, false); //Play sound
			//ReleaseResource(soundHandle);
		Pass:
			CloseResFile(fileID);
			DisposeHandle(soundHandle);
			UseResFile(oldResFileID);
		}
		InitCursor();
		
		return sfHookNullEvent;
	}
	
	/*if(item == 11) { //Small
		SizeWindow(dialog, 336, 310, true);
		ShowDItem(dialog, 12);
		HideDItem(dialog, 11);
		
		data->extendedDLOG = false;
		//SetRect(&itemRect, 306, 114, 338, 146);
		//EraseRect(&itemRect);
		//DrawDialog(dialog);
		
		return sfHookNullEvent;
	}
	
	if(item == 12) { //Extended
		SizeWindow(dialog, 497, 310, true);
		ShowDItem(dialog, 11);
		HideDItem(dialog, 12);
		
		data->extendedDLOG = true;
		SetRect(&itemRect, 340, 62, 338, 496);
		EraseRect(&itemRect);
		DrawDialog(dialog);
		
		return sfHookNullEvent;
	}*/
	
	
	return item;
}

pascal Boolean FileFilter(ParmBlkPtr fileParamPtr, openData* data)
{
	OSType* typePtr;
	
	typePtr = data->fileListPtr;
	while(*typePtr != 0L) {
		if(fileParamPtr->fileParam.ioFlFndrInfo.fdType == *typePtr)
		return false;
		
		++typePtr;
	}
	
	if(data->addBank && fileParamPtr->fileParam.ioFlFndrInfo.fdType == 'Bank')
	return false;
	
	return true;
}