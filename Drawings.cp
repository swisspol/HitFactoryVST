#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

//VARIABLES LOCALES:

Rect				sourceRect_2,
					destRect_2;
					
// FONCTIONS:

void Draw_Name(short num)
{
	ForeColor(blackColor);
	SetRect(&sourceRect_2, 0 + 2, 0 + 4, 0 + 66, 0 + 15);
	SetRect(&destRect_2, 2 + 80 * num, 4, 66 + 80 * num, 15);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
	MoveTo(6 + 80 * num,13);
	if(samples[c[num]].type == sampleType)
	RGBForeColor(&thePrefs.sampleColor);
	else
	RGBForeColor(&thePrefs.DTDColor);
	DrawString(samples[c[num]].name);
}

void Draw_Display(short num, Str255 text)
{
	ForeColor(blackColor);
	SetRect(&sourceRect_2, 0 + 3, 0 + 22, 0 + 76, 0 + 33);
	SetRect(&destRect_2, 3 + 80 * num, 22, 76 + 80 * num, 33);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
	RGBForeColor(&thePrefs.displayColor);
	MoveTo(7 + 80 * num,31);
	DrawString(text);
}
	
void Draw_Options(short num)
{
	if(thePrefs.SSourceMode[c[num]] == kSSpSourceMode_Localized) {
		SetRect(&sourceRect_2, 533, 100, 544, 111);
		SetRect(&destRect_2, 5 + 80 * num, 41, 16 + 80 * num, 52);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
	}
	if(thePrefs.SSourceMode[c[num]] == kSSpSourceMode_Ambient) {
		SetRect(&sourceRect_2, 533, 100, 544, 111);
		SetRect(&destRect_2, 40 + 80 * num, 41, 51 + 80 * num, 52);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
	}
	if(thePrefs.SLoop[c[num]]) {
		SetRect(&sourceRect_2, 533, 134, 544, 145);
		SetRect(&destRect_2, 5 + 80 * num, 54, 16 + 80 * num, 65);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
	}
	if(thePrefs.SMaxVol[c[num]] == thePrefs.STurbo) {
		SetRect(&sourceRect_2, 533, 112, 544, 133);
		SetRect(&destRect_2, 5 + 80 * num, 67, 16 + 80 * num, 78);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
	}
	
	if(samples[c[num]].fileID && samples[c[num]].totalSndNb > 1) {
		SetRect(&sourceRect_2, 451, 161, 463, 184);
		SetRect(&destRect_2, 57 + 80 * num, 54, 69 + 80 * num, 77);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
	}
}
	
void Draw_Instrument_Options()
{
	if(thePrefs.synthMode == 2) {
		SetRect(&sourceRect_2, 533, 134, 544, 145);
		SetRect(&destRect_2, 209, 6, 220, 17);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(instrumentsWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
	}
	if(thePrefs.IMaxVol == thePrefs.ITurbo) {
		SetRect(&sourceRect_2, 533, 112, 544, 133);
		SetRect(&destRect_2, 209, 19, 220, 30);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(instrumentsWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
	}
}

void Draw_Number(short num)
{
	NumToString(c[num], theString);
	BlockMove(&theString[1], &theString[7], theString[0]);
	theString[0] = theString[0] + 6;
	theString[1] = 'T';
	theString[2] = 'r';
	theString[3] = 'a';
	theString[4] = 'c';
	theString[5] = 'k';
	theString[6] = ' ';
	Draw_Display(num, theString);
}

void Draw_Volume(short num)
{
	short temp = volPaddleStart + (thePrefs.SMaxVol[c[num]] - thePrefs.SVol[c[num]])
		/ (thePrefs.SMaxVol[c[num]] / volPaddleLength);
	
	SetRect(&sourceRect_2, 451, 120, 471, 160);
	SetRect(&destRect_2, 30 + 80 * num, temp - 20, 50 + 80 * num, temp + 20);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
}

void Draw_Pitch(short num)
{
	short temp = 40 + 80 * num + thePrefs.SRelativeRate[c[num]];
	
	SetRect(&sourceRect_2, 472, 120, 486, 136);
	SetRect(&destRect_2, temp - 7, 213, temp + 7, 229);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
}

void Draw_Panoramic(short num)
{
	short temp = 15 + 80 * num + (200 - thePrefs.SPan[c[num]]) / 4;
	
	SetRect(&sourceRect_2, 472, 120, 486, 136);
	SetRect(&destRect_2, temp - 7, 231, temp + 7, 247);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
}

void Draw_Instrument_Volume()
{
	short temp = insVolPaddleStart + (thePrefs.IMaxVol - thePrefs.IVol)
		/ (thePrefs.IMaxVol / insVolPaddleLength);
	
	SetRect(&sourceRect_2, 451, 120, 471, 160);
	SetRect(&destRect_2, 234, temp - 20, 254, temp + 20);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(instrumentsWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
}

void Draw_Rythms_Title()
{
	ForeColor(blackColor);
	SetRect(&sourceRect_2, 611 + 2, 0 + 4, 611 + 66, 0 + 15);
	SetRect(&destRect_2, 2, 4, 66, 15);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
	RGBForeColor(&thePrefs.sampleColor);
	MoveTo(6,13);
	DrawString(patternName);
}

void Draw_Rythms_BPM()
{
	ForeColor(blackColor);
	SetRect(&sourceRect_2, 611 + 279, 0 + 5, 611 + 312, 0 + 22);
	SetRect(&destRect_2, 279, 5, 312, 22);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
	RGBForeColor(&thePrefs.displayColor);
	TextSize(12);
	if(thePrefs.MIDIFlags & kUseHFClock) {
		MoveTo(281,18);
		NumToString(thePrefs.RBPM, theString);
		DrawString(theString);
	}
	else {
		MoveTo(282,18);
		DrawString("\p????");
	}
	TextSize(9);
}

void Draw_Rythms_Current()
{
	ForeColor(blackColor);
	SetRect(&sourceRect_2, 611 + 105, 0 + 19, 611 + 155, 0 + 38);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
		&sourceRect_2, &rytCurrentRect, srcCopy, nil);
	RGBForeColor(&thePrefs.displayColor);
	MoveTo(120,33);
	NumToString(thePrefs.RCurrent, theString);
	TextSize(12);
	DrawString(theString);
	TextSize(9);
}

void Draw_Rythms_Next()
{
	ForeColor(blackColor);
	SetRect(&sourceRect_2, 611 + 187, 0 + 19, 611 + 237, 0 + 38);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
		&sourceRect_2, &rytNextRect, srcCopy, nil);
	RGBForeColor(&thePrefs.displayColor);
	MoveTo(202,33);
	NumToString(thePrefs.RNext, theString);
	TextSize(12);
	DrawString(theString);
	TextSize(9);
}

void Draw_Rythms_Options()
{
	;
}

void Draw_Rythms_Names()
{
	short		temp;
	
	TextFont(3);
	for(i = 0; i < kPatternParts; ++i) {
		ForeColor(blackColor);
		SetRect(&sourceRect_2, 611 + 2, 0 + 48 + 10 * i, 611 + 64, 0 + 57 + 10 * i);
		SetRect(&destRect_2, 2, 48 + 10 * i, 64, 57 + 10 * i);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
		
		RGBForeColor(&thePrefs.sampleColor);
		MoveTo(3, 55 + 10 * i);
		if(MIDIDriverEnabled && !rythms[i].fileID) {
			NumToString(thePrefs.RPartPitch[i], theString);
			temp = theString[0];
			BlockMove(&theString[1], &theString[6], temp);
			BlockMove("\pMIDI ", &theString[0], 6);
			theString[0] += temp;
			DrawString(theString);
		}
		else
		DrawString(rythms[i].name);
	}
	TextFont(200);
}

void Draw_Rythms_Mute()
{
	SetRect(&sourceRect_2, 611 + 66, 0 + 47, 611 + 76, 0 + 147);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
		&sourceRect_2, &rytMuteRect, srcCopy, nil);
			
	SetRect(&sourceRect_2, 611 + 66, 151 + 47, 611 + 76, 151 + 57);
	for(i = 0; i < kPatternParts; ++i)
	if(thePrefs.RPartMute[i]) {
		SetRect(&destRect_2, 66, 47 + 10 * i, 76, 57 + 10 * i);
		CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
			&sourceRect_2, &destRect_2, srcCopy, nil);
	}
	if(thePrefs.quantizeFlags & kQRB) {
		SetRect(&sourceRect_2, 451, 185, 461, 195);
		for(i = 0; i < nbRBMutes; ++i) {
			SetRect(&destRect_2, 66, 47 + 10 * RBMutes[i], 76, 57 + 10 * RBMutes[i]);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect_2, &destRect_2, srcCopy, nil);
		}
	}
}

void Draw_Rythms_Pattern()
{
	short		pos;
	
	SetRect(&sourceRect_2, 611 + 78, 0 + 47, 611 + 238, 0 + 147);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
		&sourceRect_2, &rytPatternRect, srcCopy, nil);
		
	for(i = 0; i < kPatternParts; ++i)
		for(pos = 0; pos < kPatternLength; ++pos)
		if(pattern[thePrefs.RCurrent][i][pos]) {
			SetRect(&sourceRect_2, 611 + 78 + 10 * pos, 151 + 47 + 10 * i, 611 + 88 + 10 * pos, 151 + 57 + 10 * i);
			SetRect(&destRect_2, 78 + 10 * pos, 47 + 10 * i, 88 + 10 * pos, 57 + 10 * i);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect_2, &destRect_2, srcCopy, nil);
		}
}

void Draw_Rythms_Volume()
{
	short temp = kRytVolPaddleStart + (thePrefs.RMaxVol - thePrefs.RVol)
		/ (thePrefs.RMaxVol / kRytVolPaddleLength);
	
	SetRect(&sourceRect_2, 451, 120, 471, 160);
	SetRect(&destRect_2, 269, temp - 20, 289, temp + 20);
	CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
		&sourceRect_2, &destRect_2, srcCopy, nil);
}

void Update_Windows()
{
	long			j;
	
	GetPort(&savePort);
	
	if((WindowPtr) event.message == mainWin) {
		SetGWorld((CGrafPtr) mainWin, nil);
		BackColor(whiteColor);
		ForeColor(blackColor);
		BeginUpdate(mainWin);
			SetRect(&sourceRect_2, 0, 0, 80, 270);
			for(i = 0; i < nbShowSamples; ++i) {
				SetRect(&destRect_2, i * 80, 0, 80 + i * 80, 270);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
					&sourceRect_2, &destRect_2, srcCopy, nil);
			}
			for(i = 0; i < nbShowSamples; ++i) {
				Draw_Options(i);
				Draw_Volume(i);
				if(!(!(thePrefs.MIDIFlags & kUseHFClock) && (thePrefs.quantizeFlags & kQSmplPitchAlign)))
				Draw_Pitch(i);
				if(!thePrefs.SLoop[c[i]])
					if(thePrefs.SStatus[c[i]] == initStereo || thePrefs.SStatus[c[i]] == initMono)
					Draw_Panoramic(i);
				if(thePrefs.SPlaying[c[i]]) {
					SetRect(&sourceRect_2, 451, 100, 491, 118);
					SetRect(&destRect_2, 39 + i * 80, 251, 79 + i * 80, 269);
					CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
						&sourceRect_2, &destRect_2, srcCopy, nil);
				}
				for(j = 0; j < nbSamplesToStart; ++j)
				if(c[i] == samplesToStart[j]) {
					SetRect(&sourceRect_2, 492, 119, 532, 137);
					SetRect(&destRect_2, 39 + i * 80, 251, 79 + i * 80, 269);
					CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
						&sourceRect_2, &destRect_2, srcCopy, nil);
					break;
				}
				for(j = 0; j < nbSamplesToStop; ++j)
				if(c[i] == samplesToStop[j]) {
					SetRect(&sourceRect_2, 492, 119, 532, 137);
					SetRect(&destRect_2, 39 + i * 80, 251, 79 + i * 80, 269);
					CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
						&sourceRect_2, &destRect_2, srcCopy, nil);
					break;
				}
			}
			
			//RGBBackColor(&theBackColor);
			for(i = 0; i < nbShowSamples; ++i) {
				Draw_Name(i);
				Draw_Number(i);
			}
			DrawControls(mainWin);
		EndUpdate(mainWin);
	}
	
	if((WindowPtr) event.message == instrumentsWin) {
		SetGWorld((CGrafPtr) instrumentsWin, nil);
		BackColor(whiteColor);
		ForeColor(blackColor);
		BeginUpdate(instrumentsWin);
			SetRect(&sourceRect_2, 162, 0, 450, 150);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(instrumentsWin),
				&sourceRect_2, &instrumentsRect, srcCopy, nil);
			if(thePrefs.insWinExt) {
				Draw_Instrument_Options();
				Draw_Instrument_Volume();
			}
			LUpdate((*instrumentsList)->port->visRgn, instrumentsList);
			RGBBackColor(&theBackColor);
		EndUpdate(instrumentsWin);
	}
	
	if((WindowPtr) event.message == rythmsWin) {
		SetGWorld((CGrafPtr) rythmsWin, nil);
		BackColor(whiteColor);
		ForeColor(blackColor);
		BeginUpdate(rythmsWin);
			SetRect(&sourceRect_2, 611, 0, 931, 150);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
				&sourceRect_2, &rythmsRect, srcCopy, nil);
			if(thePrefs.RPlaying) {
				SetRect(&sourceRect_2, 451, 100, 490, 117);
				SetRect(&destRect_2, 39, 22, 78, 39);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
					&sourceRect_2, &destRect_2, srcCopy, nil);
			}
			if(thePrefs.RMaxVol == thePrefs.RTurbo) {
				SetRect(&sourceRect_2, 533, 112, 544, 133);
				SetRect(&destRect_2, 265, 27, 276, 38);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(rythmsWin),
					&sourceRect_2, &destRect_2, srcCopy, nil);
			}
			Draw_Rythms_Volume();
			//RGBBackColor(&theBackColor);
			Draw_Rythms_Title();
			Draw_Rythms_BPM();
			Draw_Rythms_Current();
			Draw_Rythms_Next();
			Draw_Rythms_Options();
			if(thePrefs.rytWinExt) {
				Draw_Rythms_Names();
				BackColor(whiteColor);
				ForeColor(blackColor);
				Draw_Rythms_Mute();
				Draw_Rythms_Pattern();
			}
		EndUpdate(rythmsWin);
	}
	
	if((WindowPtr) event.message == recordWin) {
		SetGWorld((CGrafPtr) recordWin, nil);
		BackColor(whiteColor);
		ForeColor(blackColor);
		BeginUpdate(recordWin);
			SetRect(&sourceRect_2, 451, 0, 610, 41);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(recordWin),
				&sourceRect_2, &recordRect, srcCopy, nil);
			if(recording) {
				SetRect(&sourceRect_2, 492, 100, 531, 118);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(recordWin),
					&sourceRect_2, &recRecRect, srcCopy, nil);
			}
			if(playing) {
				SetRect(&sourceRect_2, 451, 100, 490, 118);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(recordWin),
					&sourceRect_2, &recPlayRect, srcCopy, nil);
			}
			
			RGBBackColor(&theBackColor);
			/*EraseRect(&timeCodeRect);
			NumToString(currentTime, theString);
			MoveTo(14,18);
			DrawString(theString);*/
		EndUpdate(recordWin);
	}
	
	if(!MBarShown && (WindowPtr) event.message == backWin) {
		SetGWorld((CGrafPtr) backWin, nil);
		BeginUpdate(backWin);
			SetRect(&aRect, 0, 0, dragRect.right, dragRect.bottom);
			FillCRect(&aRect, backPat);
		EndUpdate(backWin);
	}
	
	SetGWorld((CGrafPtr) savePort, nil);
}
