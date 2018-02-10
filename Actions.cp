#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Record Codes.h"
#include			"Variables.h"

#define				leftPos			64
#define				leftRect		60
#define				rightRect		84

//CONSTANTES LOCALES:

#define				commandColor	greenColor
#define				kBPMDelay		6L

//FONCTIONS:

Boolean Tracking(GWorldPtr upSrcGWorld, GWorldPtr downSrcGWorld, GWorldPtr destGWorld, 
	Rect* upSrcRect, Rect* downSrcRect, Rect* buttonRect)
{
	Point		whereMouse,
				oldWhereMouse;
	RGBColor	oldForeColor;
	
	GetForeColor(&oldForeColor);
	BackColor(whiteColor);
	ForeColor(blackColor);
	oldWhereMouse.h = oldWhereMouse.v = 0;
	while(Button()) {
		GetMouse(&whereMouse);
		if(whereMouse.h != oldWhereMouse.h || whereMouse.v != oldWhereMouse.v) {
			if(PtInRect(whereMouse, buttonRect))
			CopyBits(GWBitMapPtr(downSrcGWorld), GWBitMapPtr(destGWorld),
				downSrcRect, buttonRect, srcCopy, nil);
			else
			CopyBits(GWBitMapPtr(upSrcGWorld), GWBitMapPtr(destGWorld),
				upSrcRect, buttonRect, srcCopy, nil);
		}
		oldWhereMouse = whereMouse;
	}
	CopyBits(GWBitMapPtr(upSrcGWorld), GWBitMapPtr(destGWorld),
		upSrcRect, buttonRect, srcCopy, nil);
	RGBForeColor(&oldForeColor);
	RGBBackColor(&theBackColor);
	
	if(PtInRect(whereMouse, buttonRect))
	return true;
	
	return false;
}

Boolean Test_Tracking(Point where, GWorldPtr upSrcGWorld, GWorldPtr downSrcGWorld, GWorldPtr destGWorld, 
	Rect* upSrcRect, Rect* downSrcRect, Rect* buttonRect)
{
	RGBColor	oldForeColor;
	
	GetForeColor(&oldForeColor);
	BackColor(whiteColor);
	ForeColor(blackColor);
	
	if(PtInRect(where, buttonRect))
	CopyBits(GWBitMapPtr(downSrcGWorld), GWBitMapPtr(destGWorld),
		downSrcRect, buttonRect, srcCopy, nil);
	else
	CopyBits(GWBitMapPtr(upSrcGWorld), GWBitMapPtr(destGWorld),
		upSrcRect, buttonRect, srcCopy, nil);
	RGBForeColor(&oldForeColor);
	RGBBackColor(&theBackColor);
	
	if(PtInRect(where, buttonRect))
	return true;
	
	return false;
}

void End_Tracking(GWorldPtr srcGWorld, GWorldPtr destGWorld, Rect* srcRect, Rect* buttonRect)
{
	RGBColor	oldForeColor;
	
	GetForeColor(&oldForeColor);
	BackColor(whiteColor);
	ForeColor(blackColor);
	CopyBits(GWBitMapPtr(srcGWorld), GWBitMapPtr(destGWorld),
		srcRect, buttonRect, srcCopy, nil);
	RGBForeColor(&oldForeColor);
	RGBBackColor(&theBackColor);
}

short Act_Sampler(Point whereMouse)
{
	Rect			paddleRect;
	Str255			itemText;
	unsigned char	textLength;
	
	itemHit = TestControl(samplerScrollBar, whereMouse);
	if(itemHit) {
		switch(itemHit) {
			
			case inThumb:
			short	oldTrueButton = (whereMouse.h - 16) / ((samplerWide - 32) / nbShowSamples);;
			while(Button()) {
				GetMouse(&whereMouse);
				trueButton = (whereMouse.h - 16) / ((samplerWide - 32) / nbShowSamples);
				if(trueButton > oldTrueButton) {
					Move_Right();
					oldTrueButton = trueButton;
				}
				if(trueButton < oldTrueButton) {
					Move_Left();
					oldTrueButton = trueButton;
				}
			}
			break;
			
			case inUpButton:
			HiliteControl(samplerScrollBar, inUpButton);
			while(Button()) {
				Delay(kEnterDelay, &ticks);
				Move_Left();
			}
			HiliteControl(samplerScrollBar, 0);
			break;
			
			case inDownButton:
			HiliteControl(samplerScrollBar, inDownButton);
			while(Button()) {
				Delay(kEnterDelay, &ticks);
				Move_Right();
			}
			HiliteControl(samplerScrollBar, 0);
			break;
			
			case inPageUp:
			case inPageDown:
			itemHit = (samplerWide - 32) / (thePrefs.nbSamples - nbShowSamples);
			itemHit = whereMouse.h / itemHit;
			for(i = 0; i < maxNbSamples; ++i)
			uc[i] = -1;
			for(i = 0; i < nbShowSamples; ++i) {
				c[i] = itemHit + i;
				uc[itemHit + i] = i;
			}
			SetCtlValue(samplerScrollBar, itemHit);
			InvalRect(&shellRect);
			break;
			
		}
		return true;
	}
	
	button = whereMouse.h / 80;
	trueButton = c[button];
	if(button > nbShowSamples - 1)
	return false;
		
	if(PtInRect(whereMouse, &nameRect)) {
		if(event.modifiers & optionKey || event.modifiers & controlKey) {
			SetRect(&sourceRect, 1, 1, 79, 19);
			SetRect(&destRect, 81 + 1, 1, 81 + 79, 19);
			SetRect(&aRect, 1 + 80 * button, 1, 79 + 80 * button, 19);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect)) {
				BackColor(whiteColor);
				Draw_Name(button);
				return false;
			}
			BackColor(whiteColor);
			Draw_Name(button);
			if(event.modifiers & optionKey)
			SF_OpenSample(trueButton);
			if(event.modifiers & controlKey)
			SF_OpenDirectToDisk(trueButton);
			SetRect(&aRect, button * 80, 0, 80 + button * 80, 270);
			InvalRect(&aRect);
			
			return true;
		}
		
		/*BackColor(whiteColor);
		ForeColor(blackColor);
		SetRect(&sourceRect, 1 + 80, 1, 80 + 78, 18);
		SetRect(&destRect, 1 + 80 * button, 1, 78 + 80 * button, 18);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
			&sourceRect, &destRect, srcCopy, nil);*/
		SetRect(&sourceRect, 1 + 81, 1, 81 + 79, 19);
		SetRect(&destRect, 1 + 80 * button, 1, 79 + 80 * button, 19);
		End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &destRect);
		if(samples[trueButton].type == sampleType)
		RGBForeColor(&thePrefs.sampleColor);
		else
		RGBForeColor(&thePrefs.DTDColor);
		RGBBackColor(&theBackColor);
		MoveTo(7 + 80 * button,14);
		DrawString(samples[trueButton].name);
		
		CheckItem(popUpMenu, 7, false);
		CheckItem(popUpMenu, 8, false);
		CheckItem(popUpMenu, 9, false);
		CheckItem(popUpMenu, 10, false);
		if(thePrefs.SStatus[trueButton] == initStereo)
		CheckItem(popUpMenu, 7, true);
		if(thePrefs.SStatus[trueButton] == initMono)
		CheckItem(popUpMenu, 8, true);
		if(thePrefs.SStatus[trueButton] == initChanLeft)
		CheckItem(popUpMenu, 9, true);
		if(thePrefs.SStatus[trueButton] == initChanRight)
		CheckItem(popUpMenu, 10, true);
		
		long menuResult = PopUpMenuSelect(popUpMenu, thePrefs.smpWinPos.v + 18, 
			thePrefs.smpWinPos.h + 2 + 80 * button, 1);
		SetRect(&sourceRect, 1, 1, 79, 19);
		End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &destRect);
		BackColor(whiteColor);
		Draw_Name(button);
		
		if(HiWord(menuResult) == 2000)
		switch(LoWord(menuResult)) {
			
			case 1:
			if(SF_OpenSample(trueButton)) {
				saved = false;
				if(musicFileSpec.parID && recorded)
				EnableItem(menu[1], 4);
			}
			break;
			
			case 2:
			if(SF_OpenDirectToDisk(trueButton)) {
				saved = false;
				if(musicFileSpec.parID && recorded)
				EnableItem(menu[1], 4);
			}
			break;
			
			case 3:
			SF_CloseSample(trueButton);
			break;
			
#if demo
			case 5:
			Do_Error(-30008, 904);
			break;
#endif

			case 7:
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSInit;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = initStereo;
			}
			
			SCD_ReInitTrack(trueButton, initStereo);
			break;
			
			case 8:
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSInit;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = initMono;
			}
			
			SCD_ReInitTrack(trueButton, initMono);
			break;
			
			case 9:
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSInit;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = initChanLeft;
			}
			
			SCD_ReInitTrack(trueButton, initChanLeft);
			break;
			
			case 10:
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSInit;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = initChanRight;
			}
			
			SCD_ReInitTrack(trueButton, initChanRight);
			break;
			
		}
		
		for(i = 1; i < kNbLoadMenus; ++i)
		if(HiWord(menuResult) == kStartLoadMenusID + i) {
			GetItem(loadMenus[i], LoWord(menuResult), theString);
			SF_FastLoad(trueButton, theString, loadMenusDirID[i]);
			break;
		}
	
		SetRect(&aRect, button * 80, 0, 80 + button * 80, 270);
		InvalRect(&aRect);
		
		return true;
	}
	
	if(PtInRect(whereMouse, &optionsRect)) {
		SetRect(&aRect, 5 + 80 * button, 41, 16 + 80 * button, 52);
		if(PtInRect(whereMouse, &aRect) && thePrefs.SReverb) {
			SetRect(&sourceRect, 5, 41, 16, 52);
			SetRect(&destRect, 81 + 5, 41, 81 + 16, 52);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect)) {
				SetRect(&sourceRect, 533, 100, 544, 111);
				if(thePrefs.SSourceMode[trueButton] == kSSpSourceMode_Localized)
				End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
				return false;
			}
			
			if(thePrefs.SSourceMode[trueButton] == kSSpSourceMode_Localized)
			thePrefs.SSourceMode[trueButton] = kSSpSourceMode_Unfiltered;
			else
			thePrefs.SSourceMode[trueButton] = kSSpSourceMode_Localized;
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSEffects;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = thePrefs.SSourceMode[trueButton];
			}
			
			SCD_EffectsSample(trueButton);
			
			SetRect(&sourceRect, 5, 41, 75, 52);
			SetRect(&destRect, 5 + 80 * button, 41, 75 + 80 * button, 52);
			End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &destRect);
			SetRect(&sourceRect, 533, 100, 544, 111);
			SetRect(&destRect, 5 + 80 * button, 41, 16 + 80 * button, 52);
			if(thePrefs.SSourceMode[trueButton] == kSSpSourceMode_Localized)
			End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &destRect);
			
			return true;
		}
		
		SetRect(&aRect, 40 + 80 * button, 41, 51 + 80 * button, 52);
		if(PtInRect(whereMouse, &aRect) && thePrefs.SReverb) {
			SetRect(&sourceRect, 40, 41, 51, 52);
			SetRect(&destRect, 81 + 40, 41, 81 + 51, 52);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect)) {
				SetRect(&sourceRect, 533, 100, 544, 111);
				if(thePrefs.SSourceMode[trueButton] == kSSpSourceMode_Ambient)
				End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
				return false;
			}
			
			if(thePrefs.SSourceMode[trueButton] == kSSpSourceMode_Ambient)
			thePrefs.SSourceMode[trueButton] = kSSpSourceMode_Unfiltered;
			else
			thePrefs.SSourceMode[trueButton] = kSSpSourceMode_Ambient;
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSEffects;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = thePrefs.SSourceMode[trueButton];
			}
			
			SCD_EffectsSample(trueButton);
			
			SetRect(&sourceRect, 5, 41, 75, 52);
			SetRect(&destRect, 5 + 80 * button, 41, 75 + 80 * button, 52);
			End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &destRect);
			SetRect(&sourceRect, 533, 100, 544, 111);
			SetRect(&destRect, 40 + 80 * button, 41, 51 + 80 * button, 52);
			if(thePrefs.SSourceMode[trueButton] == kSSpSourceMode_Ambient)
			End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &destRect);
			
			return true;
		}
		
		SetRect(&aRect, 5 + 80 * button, 54, 16 + 80 * button, 65);
		if(PtInRect(whereMouse, &aRect)) {
			SetRect(&sourceRect, 5, 54, 16, 65);
			SetRect(&destRect, 81 + 5, 54, 81 + 16, 65);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect) || thePrefs.SStatus[trueButton] == initChanLeft 
					|| thePrefs.SStatus[trueButton] == initChanRight) {
				SetRect(&sourceRect, 533, 134, 544, 145);
				if(thePrefs.SLoop[trueButton])
				End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
				return false;
			}
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSLoop;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = 0;
			}
			
			if(!thePrefs.SLoop[trueButton])
			thePrefs.SLoop[trueButton] = loopValue;
			else {
				thePrefs.SLoop[trueButton] = 0;
				BackColor(whiteColor);
				ForeColor(blackColor);
				Draw_Panoramic(button);
				RGBBackColor(&theBackColor);
			}
			SetRect(&sourceRect, 533, 134, 544, 145);
			if(thePrefs.SLoop[trueButton])
			End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
			SetRect(&aRect, 80 * button, 231, 80 + 80 * button, 247);
			InvalRect(&aRect);
				
			return true;
		}
		
		SetRect(&aRect, 5 + 80 * button, 67, 16 + 80 * button, 78);
		if(PtInRect(whereMouse, &aRect)) {
			SetRect(&sourceRect, 5, 67, 16, 78);
			SetRect(&destRect, 81 + 5, 67, 81 + 16, 78);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect)) {
				SetRect(&sourceRect, 533, 112, 544, 133);
				if(thePrefs.SMaxVol[trueButton] == thePrefs.STurbo)
				End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
				return false;
			}
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSTurbo;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = 0;
			}
			
			if(thePrefs.SMaxVol[trueButton] == thePrefs.STurbo) {
				thePrefs.SMaxVol[trueButton] = defaultMaxVolume;
				thePrefs.SVol[trueButton] = thePrefs.SVol[trueButton]
					/ (thePrefs.STurbo / defaultMaxVolume);
			}
			else {
				thePrefs.SMaxVol[trueButton] = thePrefs.STurbo;
				thePrefs.SVol[trueButton] = thePrefs.SVol[trueButton]
					* (thePrefs.STurbo / defaultMaxVolume);
			}
			SCD_VolumeSample(trueButton);
			SetRect(&sourceRect, 533, 112, 544, 133);
			if(thePrefs.SMaxVol[trueButton] == thePrefs.STurbo)
			End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
			//SetRect(&aRect, 80 * button, 82, 80 + 80 * button, 208);
			//InvalRect(&aRect);
			
			return true;
		}
		
		if((samples[trueButton].totalSndNb == 1) || 
			(samples[trueButton].type == directToDiskType)
			|| !samples[trueButton].fileID)
		return false;
		
		SetRect(&sourceRect, 451, 161, 463, 173);
		SetRect(&destRect, 464, 161, 476, 173);
		SetRect(&aRect, 57 + 80 * button, 54, 69 + 80 * button, 66);
		if(PtInRect(whereMouse, &aRect)) {
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect))
			return false;
			
			if(samples[trueButton].fileID) {
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kSNext;
					tempRec[position].command = trueButton;
					tempRec[position].command_2 = 0;
				}
				
				SF_LoadNextSample(trueButton);
			}
			BackColor(whiteColor);
			Draw_Name(button);
			
			return true;
		}
		
		SetRect(&sourceRect, 451, 173, 463, 184);
		SetRect(&destRect, 464, 173, 476, 184);
		SetRect(&aRect, 57 + 80 * button, 66, 69 + 80 * button, 77);
		if(PtInRect(whereMouse, &aRect)) {
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect))
			return false;
			
			if(samples[trueButton].fileID) {
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kSPrev;
					tempRec[position].command = trueButton;
					tempRec[position].command_2 = 0;
				}
				
				SF_LoadPrevSample(trueButton);
			}
			BackColor(whiteColor);
			Draw_Name(button);
			
			return true;
		}

		return true;
	}
		
	if(PtInRect(whereMouse, &playRect)) {
		SetRect(&sourceRect, 1, 251, 40, 269);
		SetRect(&destRect, 81 + 1, 251, 81 + 40, 269);
		SetRect(&aRect, 1 + 80 * button, 251, 40 + 80 * button, 269);
		if(PtInRect(whereMouse, &aRect)) {
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect))
			return false;
			if(!thePrefs.SPlaying[trueButton])
			return false;
			
			if(thePrefs.quantizeFlags & kQSmplStop && !(event.modifiers & optionKey)
				&& samples[trueButton].type == sampleType) {
				SetRect(&aRect, 39 + 80 * button, 251, 79 + 80 * button, 269);
				SetRect(&sourceRect, 492, 119, 532, 137);
				End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
				
				for(i = 0; i < nbSamplesToStart; ++i)
				if(trueButton == samplesToStart[i])
				return false;
				for(i = 0; i < nbSamplesToStop; ++i)
				if(trueButton == samplesToStop[i])
				return false;	
				
				samplesToStop[nbSamplesToStop] = trueButton;
				++nbSamplesToStop;
			}
			else {
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kSStop;
					tempRec[position].command = trueButton;
					tempRec[position].command_2 = 0;
				}
				
				if(samples[trueButton].type == sampleType)
				SCD_StopSample(trueButton);
				else
				SCD_StopDirectToDisk(trueButton);
				chanFinished = nil;
				
				SetRect(&sourceRect, 1, 251, 79, 269);
				SetRect(&aRect, 1 + 80 * button, 251, 79 + 80 * button, 269);
				End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
			}
			return true;
		}
		
		SetRect(&sourceRect, 39, 251, 79, 269);
		SetRect(&destRect, 81 + 39, 251, 81 + 79, 269);
		SetRect(&aRect, 39 + 80 * button, 251, 79 + 80 * button, 269);
		if(PtInRect(whereMouse, &aRect)) {
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) mainWin, 
				&sourceRect, &destRect, &aRect) || thePrefs.SPlaying[trueButton]) {
				SetRect(&sourceRect, 451, 100, 491, 118);
				if(thePrefs.SPlaying[trueButton])
				End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
				if(thePrefs.quantizeFlags & kQSmplStart)
				for(i = 0; i < nbSamplesToStart; ++i)
				if(trueButton == samplesToStart[i]) {
					SetRect(&sourceRect, 492, 119, 532, 137);
					End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
					break;
				}
				if(thePrefs.quantizeFlags & kQSmplStop)
				for(i = 0; i < nbSamplesToStop; ++i)
				if(trueButton == samplesToStop[i]) {
					SetRect(&sourceRect, 492, 119, 532, 137);
					End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
					break;
				}
				return false;
			}
			
			if(samples[trueButton].fileID) {
				if(thePrefs.quantizeFlags & kQSmplStart && !(event.modifiers & optionKey)
					&& samples[trueButton].type == sampleType) {
					SetRect(&sourceRect, 492, 119, 532, 137);
					End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
					
					for(i = 0; i < nbSamplesToStart; ++i)
					if(trueButton == samplesToStart[i])
					return false;
					for(i = 0; i < nbSamplesToStop; ++i)
					if(trueButton == samplesToStop[i])
					return false;
					
					samplesToStart[nbSamplesToStart] = trueButton;
					++nbSamplesToStart;
				}
				else {
					if(recording) {
						++position;
						tempRec[position].time = ActualTime;
						tempRec[position].action = kSStart;
						tempRec[position].command = trueButton;
						tempRec[position].command_2 = 0;
					}
				
					if(samples[trueButton].type == sampleType)
					SCD_StartSample(trueButton);
					else
					SCD_StartDirectToDisk(trueButton);
					
					SetRect(&sourceRect, 451, 100, 491, 118);
					End_Tracking(picGWorld, (CGrafPort*) mainWin, &sourceRect, &aRect);
				}
			}
			return true;
		}
		
		return true;
	}
	
	if(PtInRect(whereMouse, &volumeRect)) {
		UseResFile(mainResFile);
		GetIndString(itemText, 0, 5);
		textLength = itemText[0];
		SetRect(&sourceRect, 25, 82, 55, 208);
		SetRect(&destRect, 25 + 80 * button, 82, 55 + 80 * button, 208);
		SetRect(&paddleRect, 451, 120, 471, 160);
		oldPoint.h = oldPoint.v = 0;
		BackColor(whiteColor);
		ForeColor(blackColor);
		
		GetMouse(&newPoint);
		tempValue = volPaddleStart + (thePrefs.SMaxVol[trueButton] - thePrefs.SVol[trueButton])
		/ (thePrefs.SMaxVol[trueButton] / volPaddleLength);
		offset = newPoint.v - tempValue;
		if(offset < -20) offset = 0;
		if(offset > 20) offset = 0;
		
		while(Button()) {		
			GetMouse(&newPoint);
			newPoint.v = newPoint.v - offset;
			tempValue = thePrefs.SMaxVol[trueButton] - 
				(newPoint.v - volPaddleStart) * (thePrefs.SMaxVol[trueButton] / volPaddleLength);
			if(tempValue > thePrefs.SMaxVol[trueButton])
			tempValue = thePrefs.SMaxVol[trueButton];
			if(tempValue < 0)
			tempValue = 0;
					
			if(newPoint.v != oldPoint.v) {
				//SetGWorld(tempGWorld, nil);
				thePrefs.SVol[trueButton] = tempValue;
				SCD_VolumeSample(trueButton);
				i = volPaddleStart + (thePrefs.SMaxVol[trueButton] - thePrefs.SVol[trueButton])
					/ (thePrefs.SMaxVol[trueButton] / volPaddleLength);
				SetRect(&aRect, 30, i - 20, 50, i + 20);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&sourceRect, &sourceRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&paddleRect, &aRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
				//SetGWorld((CGrafPtr) mainWin, nil);
				NumToString(tempValue, theString);
				BlockMove(&theString[1], &itemText[textLength + 1], theString[0]);
				itemText[0] = textLength + theString[0];
				RGBBackColor(&theBackColor);
				Draw_Display(button, itemText);
				BackColor(whiteColor);
				ForeColor(blackColor);
				
				if(recording && tempRec[position].time != ActualTime
					&& tempRec[position].command_2 != thePrefs.SVol[trueButton]) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kSVolume;
					tempRec[position].command = trueButton;
					tempRec[position].command_2 = thePrefs.SVol[trueButton];
				}
			}
			oldPoint = newPoint;
				
		}
		RGBBackColor(&theBackColor);
		Draw_Number(button);
		
		return true;
	}
	
	if(PtInRect(whereMouse, &pitchRect)) {
		if(!(thePrefs.MIDIFlags & kUseHFClock) && (thePrefs.quantizeFlags & kQSmplPitchAlign))
		return false;
		UseResFile(mainResFile);
		GetIndString(itemText, 0, 6);
		textLength = itemText[0];
		SetRect(&sourceRect, 0, 213, 80, 229);
		SetRect(&destRect, 80 * button, 213, 80 + 80 * button, 229);
		SetRect(&paddleRect, 472, 120, 486, 136);
		oldPoint.h = oldPoint.v = 0;
		BackColor(whiteColor);
		ForeColor(blackColor);
		
		GetMouse(&newPoint);
		tempValue = 40 + 80 * button + thePrefs.SRelativeRate[trueButton];
		offset = newPoint.h - tempValue;
		if(offset < -7) offset = 0;
		if(offset > 7) offset = 0;
		
		while(Button()) {		
			GetMouse(&newPoint);
			newPoint.h = newPoint.h - offset;
			thePrefs.SRelativeRate[trueButton] = newPoint.h - 40 - 80 * button;
			if(thePrefs.SRelativeRate[trueButton] > kMaxRelativeRate)
			thePrefs.SRelativeRate[trueButton] = kMaxRelativeRate;
			if(thePrefs.SRelativeRate[trueButton] < kMinRelativeRate)
			thePrefs.SRelativeRate[trueButton] = kMinRelativeRate;
			
			if(event.modifiers & optionKey)
			thePrefs.SRelativeRate[trueButton] = 0;
			
			if(newPoint.h != oldPoint.h) {
				//SetGWorld(tempGWorld, nil);
				if(thePrefs.SPlaying[trueButton])
				SCD_PitchSample(trueButton);
				i = 40 + thePrefs.SRelativeRate[trueButton];
				SetRect(&aRect, i - 7, 213, i + 7, 229);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&sourceRect, &sourceRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&paddleRect, &aRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
				//SetGWorld((CGrafPtr) mainWin, nil);
				NumToString(thePrefs.SRelativeRate[trueButton], theString);
				BlockMove(&theString[1], &itemText[textLength + 1], theString[0]);
				itemText[0] = textLength + theString[0];
				RGBBackColor(&theBackColor);
				Draw_Display(button, itemText);
				BackColor(whiteColor);
				ForeColor(blackColor);
				
				if(recording && tempRec[position].time != ActualTime
					&& tempRec[position].command_2 != thePrefs.SRelativeRate[trueButton]) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kSPitch;
					tempRec[position].command = trueButton;
					tempRec[position].command_2 = thePrefs.SRelativeRate[trueButton];
				}
			}
			oldPoint = newPoint;
			
		}
		RGBBackColor(&theBackColor);
		Draw_Number(button);
		
		return true;
	}
	
	if(PtInRect(whereMouse, &panoramicRect)) {
		if(thePrefs.SLoop[trueButton] || thePrefs.SStatus[trueButton] == initChanLeft 
			|| thePrefs.SStatus[trueButton] == initChanRight)
		return false;
		
		UseResFile(mainResFile);
		GetIndString(itemText, 0, 7);
		textLength = itemText[0];
		SetRect(&sourceRect, 0, 231, 80, 247);
		SetRect(&destRect, 80 * button, 231, 80 + 80 * button, 247);
		SetRect(&paddleRect, 472, 120, 486, 136);
		oldPoint.h = oldPoint.v = 0;
		BackColor(whiteColor);
		ForeColor(blackColor);
		
		GetMouse(&newPoint);
		tempValue = 15 + 80 * button + (200 - thePrefs.SPan[trueButton]) / 4;
		offset = newPoint.h - tempValue;
		if(offset < -7) offset = 0;
		if(offset > 7) offset = 0;
		
		while(Button()) {		
			GetMouse(&newPoint);
			newPoint.h = newPoint.h - offset;
			tempValue = 200 - (4 * (newPoint.h - 15 - 80 * button));
			if(tempValue > 200)
			tempValue = 200;
			if(tempValue < 0)
			tempValue = 0;
			
			if(event.modifiers & optionKey)
			tempValue = 100;
			
			if(newPoint.h != oldPoint.h) {
				//SetGWorld(tempGWorld, nil);
				thePrefs.SPan[trueButton] = tempValue;
				SCD_VolumeSample(trueButton);
				i = 15 + (200 - thePrefs.SPan[trueButton]) / 4;
				SetRect(&aRect, i - 7, 231, i + 7, 247);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&sourceRect, &sourceRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&paddleRect, &aRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
				//SetGWorld((CGrafPtr) mainWin, nil);
				NumToString(100 - thePrefs.SPan[trueButton], theString);
				BlockMove(&theString[1], &itemText[textLength + 1], theString[0]);
				itemText[0] = textLength + theString[0];
				RGBBackColor(&theBackColor);
				Draw_Display(button, itemText);
				BackColor(whiteColor);
				ForeColor(blackColor);
				
				if(recording && tempRec[position].time != ActualTime
					&& tempRec[position].command_2 != thePrefs.SPan[trueButton]) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kSPano;
					tempRec[position].command = trueButton;
					tempRec[position].command_2 = thePrefs.SPan[trueButton];
				}
			}
			oldPoint = newPoint;
			
		}
		RGBBackColor(&theBackColor);
		Draw_Number(button);
		
		return true;
	}
	
	return false;
}

short Act_Recorder(Point whereMouse)
{
	if(PtInRect(whereMouse, &recRecRect)) {
		SetRect(&sourceRect, 451 + 40, 0 + 22, 451 + 79, 0 + 40);
		SetRect(&destRect, 451 + 40, 42 + 22, 451 + 79, 42 + 40);
		if(!Tracking(picGWorld, picGWorld, (CGrafPort*) recordWin, 
			&sourceRect, &destRect, &recRecRect) || playing) {
			SetRect(&sourceRect, 492, 100, 531, 118);
			if(recording)
			End_Tracking(picGWorld, (CGrafPort*) recordWin, &sourceRect, &recRecRect);
			return false;
		}
		
		SetRect(&sourceRect, 492, 100, 531, 118);
		End_Tracking(picGWorld, (CGrafPort*) recordWin, &sourceRect, &recRecRect);
		recording = true;
		recorded = true;
		saved = false;
		patternPos = -1; //!
		for(i = 0; i < editorPlugInsNumber; ++i)
		editorPlugIns[i].update = true;
		position = -1;
		activeChan = 0;
		Clear_Music();
		
		for(i = 0; i < thePrefs.nbSamples; ++i)
		thePrefs.SSndNb[i] = samples[i].sndNb;
		BlockMove(&thePrefs, &recordPrefs, sizeof(thePrefs));
		
		DisableItem(menu[1], 1);
		DisableItem(menu[1], 2);
		DisableItem(menu[1], 4);
		DisableItem(menu[1], 5);
		DisableItem(menu[1], 6);
		DisableItem(menu[1], 8);
		DisableItem(menu[2], 3);
		DisableItem(menu[2], 5);
		DisableItem(menu[2], 8);
		DisableItem(popUpMenu, 1);
		DisableItem(popUpMenu, 2);
		DisableItem(popUpMenu, 3);
		DisableItem(popUpMenu, 5);
		DisableItem(popUpMenu2, 0);
		
		startTime = TickCount();
		
		return true;
	}
	
	if(PtInRect(whereMouse, &recStopRect)) {
		SetRect(&sourceRect, 451 + 79, 0 + 22, 451 + 118, 0 + 40);
		SetRect(&destRect, 451 + 79, 42 + 22, 451 + 118, 42 + 40);
		if(!Tracking(picGWorld, picGWorld, (CGrafPort*) recordWin, 
			&sourceRect, &destRect, &recStopRect) || (!playing && !recording))
		return false;
		
		if(recording) {
			++position;
			tempRec[position].time = ActualTime;
			tempRec[position].action = kEnd;
			tempRec[position].command = 0;
			tempRec[position].command_2 = 0;
		}
		
		if(playing) {
			TM_KillReadTask();
			for(i = 0; i < thePrefs.nbSamples; ++i) {
				if(samples[i].type == sampleType)
				SCD_StopSample(i);
				else
				SCD_StopDirectToDisk(i);
			}
			chanFinished = nil;
			theCommand.cmd		= quietCmd;
			theCommand.param1	= 0;
			theCommand.param2	= 0;
			for(i = 0; i < thePrefs.nbInsChannels; ++i)
			SndDoImmediate(insChannels[i], &theCommand);
			RB_Stop();
			for(i = 0; i < thePrefs.nbRytChannels; ++i)
			SndDoImmediate(rytChannels[i], &theCommand);
		}
		playing = false;
		recording = false;
		position = 0;
		
		SetRect(&sourceRect, 451, 0, 610, 41);
		End_Tracking(picGWorld, (CGrafPort*) recordWin, &sourceRect, &recordRect);
		//MoveTo(14,18);
		//NumToString(currentTime, theString);
		//DrawString(theString);
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
		if(thePrefs.rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			InvalRect(&rythmsRect);
		}
		
		return true;
	}
	
	if(PtInRect(whereMouse, &recPlayRect)) {
		SetRect(&sourceRect, 451 + 118, 0 + 22, 451 + 157, 0 + 40);
		SetRect(&destRect, 451 + 118, 42 + 22, 451 + 157, 42 + 40);
		if(!Tracking(picGWorld, picGWorld, (CGrafPort*) recordWin, 
			&sourceRect, &destRect, &recPlayRect)) {
			SetRect(&sourceRect, 451, 100, 490, 118);
			if(playing)
			End_Tracking(picGWorld, (CGrafPort*) recordWin, &sourceRect, &recPlayRect);
			return false;
		}
		if(!recorded)
		return false;
		if(playing || recording)
		return false;
		
		SetRect(&sourceRect, 451, 100, 490, 118);
		End_Tracking(picGWorld, (CGrafPort*) recordWin, &sourceRect, &recPlayRect);
		playing = true;
		
		/*if(position == 0) {
			Install_Music();
			startTime = TickCount();
			activeChan = 0;
		}
		else
		startTime = TickCount() - tempRec[position - 1].time * 10 / 16;*/
		Install_Music();
		activeChan = 0;
		writePos = readPos = 0;
		TM_InstallReadTask();
		
		DisableItem(menu[1], 1);
		DisableItem(menu[1], 2);
		DisableItem(menu[1], 6);
		DisableItem(menu[1], 8);
		DisableItem(menu[2], 3);
		DisableItem(menu[2], 5);
		DisableItem(menu[2], 8);
		DisableItem(popUpMenu, 1);
		DisableItem(popUpMenu, 2);
		DisableItem(popUpMenu, 3);
		DisableItem(popUpMenu, 5);
		DisableItem(popUpMenu2, 0);
				
		if(!position) {
			SetGWorld((CGrafPtr) mainWin, nil);
			InvalRect(&shellRect);
		}
		if(thePrefs.instrumentsWin && !position) {
			SetGWorld((CGrafPtr) instrumentsWin, nil);
			InvalRect(&instrumentsRect);
		}
		if(thePrefs.rythmsWin && !position) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			InvalRect(&rythmsRect);
		}
		return true;
	}
	
	if(PtInRect(whereMouse, &recEditRect)) {
		SetRect(&sourceRect, 451 + 1, 0 + 22, 451 + 40, 0 + 40);
		SetRect(&destRect, 451 + 1, 42 + 22, 451 + 40, 42 + 40);
		if(!Tracking(picGWorld, picGWorld, (CGrafPort*) recordWin, 
			&sourceRect, &destRect, &recEditRect))
		return false;
		
		if(recorded && !recording && !playing)
		Init_EditDialog();
		
		return true;
	}
	
	return false;
}

short Act_Synthetizer(Point whereMouse)
{
	Rect		paddleRect;
	Boolean	dlbClick = false;
	
	if(PtInRect(whereMouse, &insListRect)) {
		BackColor(whiteColor);
		dlbClick = LClick(whereMouse, nil, instrumentsList);
		/*theCell.h = 0;
		theCell.v = activeInstrument;
		LSetSelect(false, theCell, instrumentsList);*/
		theCell.h = 0;
		theCell.v = 0;
		LGetSelect(true, &theCell, instrumentsList);
		LSetSelect(true, theCell, instrumentsList);
		activeInstrument = theCell.v;
		RGBBackColor(&theBackColor);
		
		if(!playing && !recording)
		if(dlbClick && SF_OpenInstrument(activeInstrument)) {
			theCell.h = 0;
			theCell.v = activeInstrument;
			LSetCell(&instruments[activeInstrument].name[1], 
				instruments[activeInstrument].name[0], theCell, instrumentsList);
			saved = false;
			if(musicFileSpec.parID && recorded)
			EnableItem(menu[1], 4);
		};
		
		return true;
	}
	
	if(PtInRect(whereMouse, &insVolumeRect)) {
		SetRect(&sourceRect, 162 + 230, 0 + 32, 162 + 260, 0 + 148);
		SetRect(&destRect, 230, 32, 260, 148);
		SetRect(&paddleRect, 451, 120, 471, 160);
		oldPoint.h = oldPoint.v = 0;
		BackColor(whiteColor);
		ForeColor(blackColor);
		
		GetMouse(&newPoint);
		tempValue = insVolPaddleStart + (thePrefs.IMaxVol - thePrefs.IVol)
		/ (thePrefs.IMaxVol / insVolPaddleLength);
		offset = newPoint.v - tempValue;
		if(offset < -20) offset = 0;
		if(offset > 20) offset = 0;
		
		while(Button()) {
			GetMouse(&newPoint);
			newPoint.v = newPoint.v - offset;
			tempValue = thePrefs.IMaxVol - 
				(newPoint.v - insVolPaddleStart) * (thePrefs.IMaxVol / insVolPaddleLength);
			if(tempValue > thePrefs.IMaxVol)
			tempValue = thePrefs.IMaxVol;
			if(tempValue < 0)
			tempValue = 0;
					
			if(newPoint.v != oldPoint.v) {
				//SetGWorld(tempGWorld, nil);
				thePrefs.IVol = tempValue;
				SCD_VolumeSynthetizer();
				i = insVolPaddleStart + (thePrefs.IMaxVol - thePrefs.IVol)
					/ (thePrefs.IMaxVol / insVolPaddleLength);
				SetRect(&aRect, 234, i - 20, 254, i + 20);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&sourceRect, &destRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&paddleRect, &aRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(instrumentsWin),
					&destRect, &destRect, srcCopy, nil);
				//SetGWorld((CGrafPtr) instrumentsWin, nil);
				
				if(recording && tempRec[position].time != ActualTime
					&& tempRec[position].command != thePrefs.IVol) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kIVolume;
					tempRec[position].command = thePrefs.IVol;
					tempRec[position].command_2 = 0;
				}
			}
			oldPoint = newPoint;
				
		}
		RGBBackColor(&theBackColor);
		
		return true;
	}
	
	if(PtInRect(whereMouse, &insOptionsRect)) {
		SetRect(&aRect, 209, 6, 220, 17);
		if(PtInRect(whereMouse, &aRect)) {
			SetRect(&sourceRect, 162 + 209, 0 + 6, 162 + 220, 0 + 17);
			SetRect(&destRect, 162 + 209, 151 + 6, 162 + 220, 151 + 17);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) instrumentsWin, 
				&sourceRect, &destRect, &aRect)) {
				SetRect(&sourceRect, 533, 134, 544, 145);
				if(thePrefs.synthMode == 2)
				End_Tracking(picGWorld, (CGrafPort*) instrumentsWin, &sourceRect, &aRect);
				return false;
			}
			
			if(thePrefs.synthMode == 1)
			thePrefs.synthMode = 2;
			else
			thePrefs.synthMode = 1;
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kMSynthMode;
				tempRec[position].command = thePrefs.synthMode;
				tempRec[position].command_2 = 0;
			}
			
			SCD_SetSynthetizerMode(thePrefs.synthMode);
			
			SetRect(&sourceRect, 533, 134, 544, 145);
			if(thePrefs.synthMode == 2)
			End_Tracking(picGWorld, (CGrafPort*) instrumentsWin, &sourceRect, &aRect);
			
			return true;
		}
		
		SetRect(&aRect, 209, 19, 220, 30);
		if(PtInRect(whereMouse, &aRect)) {
			SetRect(&sourceRect, 162 + 209, 0 + 19, 162 + 220, 0 + 30);
			SetRect(&destRect, 162 + 209, 151 + 19, 162 + 220, 151 + 30);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) instrumentsWin, 
				&sourceRect, &destRect, &aRect)) {
				SetRect(&sourceRect, 533, 112, 544, 133);
				if(thePrefs.IMaxVol == thePrefs.ITurbo)
				End_Tracking(picGWorld, (CGrafPort*) instrumentsWin, &sourceRect, &aRect);
				return false;
			}
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kITurbo;
				tempRec[position].command = 0;
				tempRec[position].command_2 = 0;
			}
			
			if(thePrefs.IMaxVol == thePrefs.ITurbo) {
				thePrefs.IMaxVol = defaultMaxVolume;
				thePrefs.IVol = thePrefs.IVol / (thePrefs.ITurbo / defaultMaxVolume);
			}
			else {
				thePrefs.IMaxVol = thePrefs.ITurbo;
				thePrefs.IVol = thePrefs.IVol * (thePrefs.ITurbo / defaultMaxVolume);
			}
			SCD_VolumeSynthetizer();
			
			SetRect(&sourceRect, 533, 112, 544, 133);
			if(thePrefs.IMaxVol == thePrefs.ITurbo)
			End_Tracking(picGWorld, (CGrafPort*) instrumentsWin, &sourceRect, &aRect);
			//SetRect(&aRect, 207, 32, 280, 148);
			//InvalRect(&aRect);
				
			return true;
		}
		
		return true;
	}
	
	return false;
}

short Act_RythmsBox(Point whereMouse)
{
	Rect		paddleRect;
	short		pos;
	
	if(PtInRect(whereMouse, &rytLoadRect)) {
		if(event.modifiers & optionKey) {
			SetRect(&sourceRect, 611 + 1, 0 + 1, 611 + 78, 0 + 18);
			SetRect(&destRect, 611 + 1, 151 + 1, 611 + 78, 151 + 18);
			SetRect(&aRect, 1, 1, 78, 18);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) rythmsWin, 
				&sourceRect, &destRect, &aRect)) {
				BackColor(whiteColor);
				Draw_Rythms_Title();
				return false;
			}
			BackColor(whiteColor);
			Draw_Rythms_Title();
			event.modifiers = event.modifiers & (~optionKey);
			RB_OpenPatternSet();
			InvalRect(&rythmsRect);
			
			return true;
		}
		
		/*BackColor(whiteColor);
		ForeColor(blackColor);
		SetRect(&sourceRect, 1 + 80, 1, 80 + 78, 18);
		SetRect(&destRect, 1 + 80 * button, 1, 78 + 80 * button, 18);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
			&sourceRect, &destRect, srcCopy, nil);*/
		SetRect(&sourceRect, 611 + 1, 151 + 1, 611 + 78, 151 + 18);
		SetRect(&destRect, 1, 1, 78, 18);
		End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &destRect);
		RGBForeColor(&thePrefs.sampleColor);
		MoveTo(7,14);
		DrawString(patternName);
		
		long menuResult = PopUpMenuSelect(popUpMenu2, thePrefs.rytWinPos.v + 18, 
			thePrefs.rytWinPos.h + 2, 1);
		SetRect(&sourceRect, 611 + 1, 0 + 1, 611 + 78, 0 + 18);
		End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &destRect);
		BackColor(whiteColor);
		Draw_Rythms_Title();
		
		switch(LoWord(menuResult)) {
			
			case 1:
			RB_OpenPatternSet();
			break;
			
			case 2:
			RB_SavePatternSet();
			break;
			
			case 4:
			RB_CopyPattern(thePrefs.RCurrent, thePrefs.RNext);
			thePrefs.RCurrent = thePrefs.RNext;
			Draw_Rythms_Next();
			Draw_Rythms_Pattern();
			break;
			
			case 5:
			RB_ClearPattern(thePrefs.RCurrent);
			break;
			
		}
	
		InvalRect(&rythmsRect);
		
		return true;
	}
	
	if(PtInRect(whereMouse, &rytPlayRect)) {
		SetRect(&sourceRect, 611 + 1, 0 + 22, 611 + 39, 0 + 39);
		SetRect(&destRect, 611 + 1, 151 + 22, 611 + 39, 151 + 39);
		SetRect(&aRect, 1, 22, 39, 39);
		if(PtInRect(whereMouse, &aRect)) {
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) rythmsWin, 
				&sourceRect, &destRect, &aRect))
			return false;
			
			if(thePrefs.RPlaying) {
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kRStop;
					tempRec[position].command = 0;
					tempRec[position].command_2 = 0;
				}
			
				RB_Stop();
			
				SetRect(&sourceRect, 611 + 1, 0 + 22, 611 + 78, 0 + 39);
				End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &rytPlayRect);
			}
			
			if(event.modifiers & optionKey) {
				patternPos = -1;
				clockCount = 5;
			}
			
			return true;
		}
		
		SetRect(&sourceRect, 611 + 39, 0 + 22, 611 + 78, 0 + 39);
		SetRect(&destRect, 611 + 39, 151 + 22, 611 + 78, 151 + 39);
		SetRect(&aRect, 39, 22, 78, 39);
		if(PtInRect(whereMouse, &aRect)) {
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) rythmsWin, 
				&sourceRect, &destRect, &aRect) || thePrefs.RPlaying) {
				SetRect(&sourceRect, 451, 100, 490, 117);
				if(thePrefs.RPlaying)
				End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
				return false;
			}
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRPlay;
				tempRec[position].command = 0;
				tempRec[position].command_2 = 0;
			}
		
			RB_Start();
			
			SetRect(&sourceRect, 451, 100, 490, 117);
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
			return true;
		}
		
		return true;
	}
	
	if(PtInRect(whereMouse, &rytCurrentRect)) {
		if(thePrefs.RPlaying)
		return false;
		
		long menuResult = PopUpMenuSelect(patternMenu, thePrefs.rytWinPos.v + 20, 
			thePrefs.rytWinPos.h + 109, thePrefs.RCurrent + 1);
		
		if(LoWord(menuResult)) {
			thePrefs.RCurrent = LoWord(menuResult) - 1;
			patternPos = -1;
			clockCount = 5;
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRCurrent;
				tempRec[position].command = thePrefs.RCurrent;
				tempRec[position].command_2 = 0;
			}
		}
		
		Draw_Rythms_Current();
		InvalRect(&rytPatternRect);
		return true;
	}
	
	if(PtInRect(whereMouse, &rytNextRect)) {
		long menuResult = PopUpMenuSelect(patternMenu, thePrefs.rytWinPos.v + 20, 
			thePrefs.rytWinPos.h + 191, thePrefs.RNext + 1);
		
		if(LoWord(menuResult)) {
			thePrefs.RNext = LoWord(menuResult) - 1;
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRNext;
				tempRec[position].command = thePrefs.RNext;
				tempRec[position].command_2 = 0;
			}
		}
		
		Draw_Rythms_Next();
		//InvalRect(&rytPatternRect);
		return true;
	}
		
	if(PtInRect(whereMouse, &rytNamesRect)) {
		trueButton = (whereMouse.v - 47) / 10;
		if(thePrefs.RPlaying || (!thePrefs.RPlaying && (playing || recording))) {
			if(!rythms[trueButton].fileID && !MIDIDriverEnabled)
			return false;
			//RGBBackColor(&theBackColor);
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 611 + 2, 0 + 48 + 10 * trueButton, 611 + 64, 0 + 57 + 10 * trueButton);
			SetRect(&destRect, 2, 48 + 10 * trueButton, 64, 57 + 10 * trueButton);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect, &destRect, srcCopy, nil);
			RGBForeColor(&thePrefs.DTDColor);
			TextFont(3);
			MoveTo(3, 55 + 10 * trueButton);
			if(!rythms[trueButton].fileID && MIDIDriverEnabled) {
				NumToString(thePrefs.RPartPitch[trueButton], theString);
				pos = theString[0];
				BlockMove(&theString[1], &theString[6], pos);
				BlockMove("\pMIDI ", &theString[0], 6);
				theString[0] += pos;
				DrawString(theString);
			}
			else
			DrawString(rythms[trueButton].name);
			
			if(!playing && !recording) {
				pos = patternPos;
				pattern[thePrefs.RCurrent][trueButton][pos] = 255;
				SetRect(&sourceRect, 611 + 78, 151 + 47, 611 + 88, 151 + 57);
				SetRect(&destRect, 78 + 10 * pos, 47 + 10 * trueButton, 88 + 10 * pos, 57 + 10 * trueButton);
				End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &destRect);
			}
			else if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRInstrument;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = 0;
			}
			if(rythms[trueButton].fileID)
			RB_PlaySound(trueButton);
			else
			MIDI_SendNote(9, thePrefs.RPartPitch[trueButton], thePrefs.RVol * thePrefs.RPartVol[trueButton] / 810);
			while(Button())
			;
			
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 611 + 2, 0 + 48 + 10 * trueButton, 611 + 64, 0 + 57 + 10 * trueButton);
			SetRect(&destRect, 2, 48 + 10 * trueButton, 64, 57 + 10 * trueButton);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect, &destRect, srcCopy, nil);
			RGBForeColor(&thePrefs.sampleColor);
			MoveTo(3, 55 + 10 * trueButton);
			if(!rythms[trueButton].fileID && MIDIDriverEnabled) {
				NumToString(thePrefs.RPartPitch[trueButton], theString);
				pos = theString[0];
				BlockMove(&theString[1], &theString[6], pos);
				BlockMove("\pMIDI ", &theString[0], 6);
				theString[0] += pos;
				DrawString(theString);
			}
			else
			DrawString(rythms[trueButton].name);
			TextFont(200);
		}
		else {
			if(event.modifiers & optionKey) {
				while(Button())
				;
				TextFont(3);
				RB_LoadSound(trueButton);
				TextFont(200);
				TextSize(9);
				TextFace(0);
				//RGBBackColor(&theBackColor);
				Draw_Rythms_Names();
				
				return true;
			}
			
			if(event.modifiers & controlKey) {
				if(!rythms[trueButton].fileID && !MIDIDriverEnabled)
				return false;
				if(playing || recording)
				return false;
				BackColor(whiteColor);
				ForeColor(blackColor);
				SetRect(&sourceRect, 611 + 2, 0 + 48 + 10 * trueButton, 611 + 64, 0 + 57 + 10 * trueButton);
				SetRect(&destRect, 2, 48 + 10 * trueButton, 64, 57 + 10 * trueButton);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
					&sourceRect, &destRect, srcCopy, nil);
				RGBForeColor(&thePrefs.DTDColor);
				TextFont(3);
				MoveTo(3, 55 + 10 * trueButton);
				if(!rythms[trueButton].fileID && MIDIDriverEnabled) {
					NumToString(thePrefs.RPartPitch[trueButton], theString);
					pos = theString[0];
					BlockMove(&theString[1], &theString[6], pos);
					BlockMove("\pMIDI ", &theString[0], 6);
					theString[0] += pos;
					DrawString(theString);
				}
				else
				DrawString(rythms[trueButton].name);
				
				if(rythms[trueButton].fileID)
				RB_PlaySound(trueButton);
				else
				MIDI_SendNote(9, thePrefs.RPartPitch[trueButton], thePrefs.RVol * thePrefs.RPartVol[trueButton] / 810);
				while(Button())
				;
				
				BackColor(whiteColor);
				ForeColor(blackColor);
				SetRect(&sourceRect, 611 + 2, 0 + 48 + 10 * trueButton, 611 + 64, 0 + 57 + 10 * trueButton);
				SetRect(&destRect, 2, 48 + 10 * trueButton, 64, 57 + 10 * trueButton);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
					&sourceRect, &destRect, srcCopy, nil);
				RGBForeColor(&thePrefs.sampleColor);
				MoveTo(3, 55 + 10 * trueButton);
				if(!rythms[trueButton].fileID && MIDIDriverEnabled) {
					NumToString(thePrefs.RPartPitch[trueButton], theString);
					pos = theString[0];
					BlockMove(&theString[1], &theString[6], pos);
					BlockMove("\pMIDI ", &theString[0], 6);
					theString[0] += pos;
					DrawString(theString);
				}
				else
				DrawString(rythms[trueButton].name);
				TextFont(200);
				
				return true;
			}
			
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 611 + 2, 0 + 48 + 10 * trueButton, 611 + 64, 0 + 57 + 10 * trueButton);
			SetRect(&destRect, 2, 48 + 10 * trueButton, 64, 57 + 10 * trueButton);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect, &destRect, srcCopy, nil);
			RGBForeColor(&thePrefs.DTDColor);
			TextFont(3);
			MoveTo(3, 55 + 10 * trueButton);
			if(!rythms[trueButton].fileID && MIDIDriverEnabled) {
				NumToString(thePrefs.RPartPitch[trueButton], theString);
				pos = theString[0];
				BlockMove(&theString[1], &theString[6], pos);
				BlockMove("\pMIDI ", &theString[0], 6);
				theString[0] += pos;
				DrawString(theString);
			}
			else
			DrawString(rythms[trueButton].name);
			while(Button())
			;				
			BackColor(whiteColor);
			ForeColor(blackColor);
			SetRect(&sourceRect, 611 + 2, 0 + 48 + 10 * trueButton, 611 + 64, 0 + 57 + 10 * trueButton);
			SetRect(&destRect, 2, 48 + 10 * trueButton, 64, 57 + 10 * trueButton);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect, &destRect, srcCopy, nil);
			RGBForeColor(&thePrefs.sampleColor);
			MoveTo(3, 55 + 10 * trueButton);
			if(!rythms[trueButton].fileID && MIDIDriverEnabled) {
				NumToString(thePrefs.RPartPitch[trueButton], theString);
				pos = theString[0];
				BlockMove(&theString[1], &theString[6], pos);
				BlockMove("\pMIDI ", &theString[0], 6);
				theString[0] += pos;
				DrawString(theString);
			}
			else
			DrawString(rythms[trueButton].name);
			TextFont(200);
				
			RB_EditPart(trueButton);
			
			Draw_Rythms_Names();
		}
		
		return true;
	}
	
	if(PtInRect(whereMouse, &rytMuteRect)) {
		trueButton = (whereMouse.v - 47) / 10;
		SetRect(&sourceRect, 611 + 66, 0 + 47, 611 + 76, 0 + 57);
		SetRect(&destRect, 611 + 66, 151 + 47, 611 + 76, 151 + 57);
		SetRect(&aRect, 66, 47 + 10 * trueButton, 76, 57 + 10 * trueButton);
		if(!Tracking(picGWorld, picGWorld, (CGrafPort*) rythmsWin, 
			&sourceRect, &destRect, &aRect)) {
			SetRect(&sourceRect, 611 + 66, 151 + 47, 611 + 76, 151 + 57);
			if(thePrefs.RPartMute[trueButton])
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
			if(thePrefs.quantizeFlags & kQRB)
			for(i = 0; i < nbRBMutes; ++i)
			if(trueButton == RBMutes[i]) {
				SetRect(&sourceRect, 451, 185, 460, 194);
				End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
			}
			return false;
		}
		
		if(thePrefs.quantizeFlags & kQRB && !(event.modifiers & optionKey)) {
			SetRect(&sourceRect, 451, 185, 461, 195);
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
			
			for(i = 0; i < nbRBMutes; ++i)
			if(trueButton == RBMutes[i])
			return false;
			
			RBMutes[nbRBMutes] = trueButton;
			++nbRBMutes;
		}
		else {
			thePrefs.RPartMute[trueButton] = !thePrefs.RPartMute[trueButton];
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRMute;
				tempRec[position].command = trueButton;
				tempRec[position].command_2 = 0;
			}
			
			if(thePrefs.RPartMute[trueButton])
			SetRect(&sourceRect, 611 + 66, 151 + 47, 611 + 76, 151 + 57);
			else
			SetRect(&sourceRect, 611 + 66, 0 + 47, 611 + 76, 0 + 57);
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
		}
		
		return true;
	}
	
	if(PtInRect(whereMouse, &rytPatternRect)) {
		if(/*playing || */recording)
		return false;
		trueButton = (whereMouse.v - 47) / 10;
		pos = (whereMouse.h - 78) / 10;
		SetRect(&sourceRect, 611 + 78 + 10 * pos, 0 + 47 + 10 * trueButton, 611 + 88 + 10 * pos, 0 + 57 + 10 * trueButton);
		SetRect(&destRect, 611 + 78 + 10 * pos, 151 + 47 + 10 * trueButton, 611 + 88 + 10 * pos, 151 + 57 + 10 * trueButton);
		SetRect(&aRect, 78 + 10 * pos, 47 + 10 * trueButton, 88 + 10 * pos, 57 + 10 * trueButton);
		if(!Tracking(picGWorld, picGWorld, (CGrafPort*) rythmsWin, 
			&sourceRect, &destRect, &aRect)) {
			SetRect(&sourceRect, 611 + 78 + 10 * pos, 151 + 47 + 10 * trueButton, 611 + 88 + 10 * pos, 151 + 57 + 10 * trueButton);
			if(pattern[thePrefs.RCurrent][trueButton][pos])
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
			return false;
		}
		
		if(pattern[thePrefs.RCurrent][trueButton][pos])
		pattern[thePrefs.RCurrent][trueButton][pos] = 0;
		else
		pattern[thePrefs.RCurrent][trueButton][pos] = 255;
		
		/*if(recording) {
			++position;
			tempRec[position].time = ActualTime;
			tempRec[position].action = kRPattern;
			tempRec[position].command = trueButton;
			tempRec[position].command_2 = pos;
		}*/
		
		if(pattern[thePrefs.RCurrent][trueButton][pos])
		SetRect(&sourceRect, 611 + 78 + 10 * pos, 151 + 47 + 10 * trueButton, 611 + 88 + 10 * pos, 151 + 57 + 10 * trueButton);
		else
		SetRect(&sourceRect, 611 + 78 + 10 * pos, 0 + 47 + 10 * trueButton, 611 + 88 + 10 * pos, 0 + 57 + 10 * trueButton);
		End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
		return true;
	}
		
	if(PtInRect(whereMouse, &rytVolumeRect)) {
		SetRect(&sourceRect, 611 + 262, 0 + 42, 611 + 295, 0 + 148);
		SetRect(&destRect, 262, 42, 295, 148);
		SetRect(&paddleRect, 451, 120, 471, 160);
		oldPoint.h = oldPoint.v = 0;
		BackColor(whiteColor);
		ForeColor(blackColor);
		
		GetMouse(&newPoint);
		tempValue = kRytVolPaddleStart + (thePrefs.RMaxVol - thePrefs.RVol)
		/ (thePrefs.RMaxVol / kRytVolPaddleLength);
		offset = newPoint.v - tempValue;
		if(offset < -20) offset = 0;
		if(offset > 20) offset = 0;
		
		while(Button()) {
			GetMouse(&newPoint);
			newPoint.v = newPoint.v - offset;
			tempValue = thePrefs.RMaxVol - 
				(newPoint.v - kRytVolPaddleStart) * (thePrefs.RMaxVol / kRytVolPaddleLength);
			if(tempValue > thePrefs.RMaxVol)
			tempValue = thePrefs.RMaxVol;
			if(tempValue < 0)
			tempValue = 0;
					
			if(newPoint.v != oldPoint.v) {
				//SetGWorld(tempGWorld, nil);
				thePrefs.RVol = tempValue;
				//SCD_VolumeRythmsBox();
				i = kRytVolPaddleStart + (thePrefs.RMaxVol - thePrefs.RVol)
					/ (thePrefs.RMaxVol / kRytVolPaddleLength);
				SetRect(&aRect, 269, i - 20, 289, i + 20);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&sourceRect, &destRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&paddleRect, &aRect, srcCopy, nil);
				CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(rythmsWin),
					&destRect, &destRect, srcCopy, nil);
				//SetGWorld((CGrafPtr) rythmsWin, nil);
				
				if(recording && tempRec[position].time != ActualTime
					&& tempRec[position].command != thePrefs.RVol) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kRVolume;
					tempRec[position].command = thePrefs.RVol;
					tempRec[position].command_2 = 0;
				}
			}
			oldPoint = newPoint;
				
		}
		RGBBackColor(&theBackColor);
		
		return true;
	}
	
	if(PtInRect(whereMouse, &rytOptionsRect)) {
		SetRect(&aRect, 265, 3, 276, 14);
		if(PtInRect(whereMouse, &aRect) && (thePrefs.MIDIFlags & kUseHFClock)) {
			SetRect(&sourceRect, 611 + 265, 151 + 3, 611 + 276, 151 + 14);
			
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
			RGBBackColor(&theBackColor);
			if(thePrefs.quantizeFlags & kQSmplPitchAlign) {
				SetGWorld((CGrafPtr) mainWin, nil);
				BackColor(whiteColor);
				ForeColor(blackColor);
				SetGWorld((CGrafPtr) rythmsWin, nil);
			}
			while(Button()) {
				if(event.modifiers & optionKey)
				thePrefs.RBPM = thePrefs.RBPM + 10;
				else
				++thePrefs.RBPM;
				if(thePrefs.RBPM > 240)
				thePrefs.RBPM = 240;
				interTime = -(15000000 / thePrefs.RBPM);
				
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kRBPM;
					tempRec[position].command = thePrefs.RBPM;
					tempRec[position].command_2 = 0;
				}
				
				if(thePrefs.quantizeFlags & kQSmplPitchAlign) {
					SetGWorld((CGrafPtr) mainWin, nil);
					for(i = 0; i < thePrefs.nbSamples; ++i)
					if(samples[i].fileID && samples[i].type == sampleType) {
						SCD_AlignSample(i);
						if(thePrefs.SPlaying[i])
						SCD_PitchSample(i);
						if(uc[i] != -1) {
							SetRect(&sourceRect, 0, 213, 80, 229);
							SetRect(&destRect, 80 * uc[i], 213, 80 + 80 * uc[i], 229);
							CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
								&sourceRect, &destRect, srcCopy, nil);
							SetRect(&sourceRect, 472, 120, 486, 136);
							itemHit = 40 + 80 * uc[i] + thePrefs.SRelativeRate[i];
							SetRect(&destRect, itemHit - 7, 213, itemHit + 7, 229);
							CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
								&sourceRect, &destRect, srcCopy, nil);
						}
					}
					SetGWorld((CGrafPtr) rythmsWin, nil);
				}
			
				Draw_Rythms_BPM();
				if(thePrefs.quantizeFlags & kQSmplPitchAlign)
				Delay(kBPMDelay - 1, &ticks);
				else
				Delay(kBPMDelay, &ticks);
				//Wait(kBPMDelay);
			}
			SetRect(&sourceRect, 611 + 265, 0 + 3, 611 + 276, 0 + 14);
			SetRect(&destRect, 265, 3, 276, 14);
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &destRect);
			if(thePrefs.quantizeFlags & kQSmplPitchAlign) {
				SetGWorld((CGrafPtr) mainWin, nil);
				RGBBackColor(&theBackColor);
				SetGWorld((CGrafPtr) rythmsWin, nil);
			}
			
			return true;
		}
		
		SetRect(&aRect, 265, 14, 276, 25);
		if(PtInRect(whereMouse, &aRect) && (thePrefs.MIDIFlags & kUseHFClock)) {
			SetRect(&sourceRect, 611 + 265, 151 + 14, 611 + 276, 151 + 25);
			
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
			RGBBackColor(&theBackColor);
			if(thePrefs.quantizeFlags & kQSmplPitchAlign) {
				SetGWorld((CGrafPtr) mainWin, nil);
				BackColor(whiteColor);
				ForeColor(blackColor);
				SetGWorld((CGrafPtr) rythmsWin, nil);
			}
			while(Button()) {
				if(event.modifiers & optionKey)
				thePrefs.RBPM = thePrefs.RBPM - 10;
				else
				--thePrefs.RBPM;
				if(thePrefs.RBPM < 20)
				thePrefs.RBPM = 20;
				interTime = -(15000000 / thePrefs.RBPM);
				
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kRBPM;
					tempRec[position].command = thePrefs.RBPM;
					tempRec[position].command_2 = 0;
				}
				
				if(thePrefs.quantizeFlags & kQSmplPitchAlign) {
					SetGWorld((CGrafPtr) mainWin, nil);
					for(i = 0; i < thePrefs.nbSamples; ++i)
					if(samples[i].fileID && samples[i].type == sampleType) {
						SCD_AlignSample(i);
						if(thePrefs.SPlaying[i])
						SCD_PitchSample(i);
						if(uc[i] != -1) {
							SetRect(&sourceRect, 0, 213, 80, 229);
							SetRect(&destRect, 80 * uc[i], 213, 80 + 80 * uc[i], 229);
							CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
								&sourceRect, &destRect, srcCopy, nil);
							SetRect(&sourceRect, 472, 120, 486, 136);
							itemHit = 40 + 80 * uc[i] + thePrefs.SRelativeRate[i];
							SetRect(&destRect, itemHit - 7, 213, itemHit + 7, 229);
							CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
								&sourceRect, &destRect, srcCopy, nil);
						}
					}
					SetGWorld((CGrafPtr) rythmsWin, nil);
				}
				
				Draw_Rythms_BPM();
				if(thePrefs.quantizeFlags & kQSmplPitchAlign)
				Delay(kBPMDelay - 1, &ticks);
				else
				Delay(kBPMDelay, &ticks);
				//Wait(kBPMDelay);
			}
			SetRect(&sourceRect, 611 + 265, 0 + 14, 611 + 276, 0 + 25);
			SetRect(&destRect, 265, 14, 276, 25);
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &destRect);
			if(thePrefs.quantizeFlags & kQSmplPitchAlign) {
				SetGWorld((CGrafPtr) mainWin, nil);
				RGBBackColor(&theBackColor);
				SetGWorld((CGrafPtr) rythmsWin, nil);
			}
			
			return true;
		}
		
		SetRect(&aRect, 265, 27, 276, 38);
		if(PtInRect(whereMouse, &aRect)) {
			SetRect(&sourceRect, 611 + 265, 0 + 27, 611 + 276, 0 + 38);
			SetRect(&destRect, 611 + 265, 151 + 27, 611 + 276, 151 + 38);
			if(!Tracking(picGWorld, picGWorld, (CGrafPort*) rythmsWin, 
				&sourceRect, &destRect, &aRect)) {
				SetRect(&sourceRect, 533, 112, 544, 133);
				if(thePrefs.RMaxVol == thePrefs.RTurbo)
				End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
				return false;
			}
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kRTurbo;
				tempRec[position].command = 0;
				tempRec[position].command_2 = 0;
			}
			
			if(thePrefs.RMaxVol == thePrefs.RTurbo) {
				thePrefs.RMaxVol = defaultMaxVolume;
				thePrefs.RVol = thePrefs.RVol / (thePrefs.RTurbo / defaultMaxVolume);
			}
			else {
				thePrefs.RMaxVol = thePrefs.RTurbo;
				thePrefs.RVol = thePrefs.RVol * (thePrefs.RTurbo / defaultMaxVolume);
			}
			//SCD_VolumeRythmsBox();
			
			SetRect(&sourceRect, 533, 112, 544, 133);
			if(thePrefs.RMaxVol == thePrefs.RTurbo)
			End_Tracking(picGWorld, (CGrafPort*) rythmsWin, &sourceRect, &aRect);
			
			return true;
		}
		
		return true;
	}
	
	return false;
}