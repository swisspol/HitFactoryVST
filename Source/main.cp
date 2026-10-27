#include			<Drag.h>

#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Record Codes.h"
#include			"Fonctions Keys.h"

//CONSTANTES LOCALES:

#define				kNbTicks				0L
#define				maxNbClicks				200
#define				supendResumeEvtMask 	2^32L
#define				resumeMask				1
#define				virtualMemoryOn			1L

#define				typeHitFactoryEvent		'HiFc'

#pragma noreturn(End)

//VARIABLES:

WindowPtr			mainWin,
					recordWin,
					instrumentsWin,
					rythmsWin,
					editWin,
					whichWin,
					backWin;
DialogPtr			theDialog,
					whichDialog;
GWorldPtr			picGWorld,
					tempGWorld;
Handle				dialogItem[20];
ControlHandle		samplerScrollBar;
GrafPtr				savePort;
EventRecord			event;
long				i,
					ticks,
					run = 1,
					clockCount = 0;
short				resID,
					whereClick,
					theKey,
					nbClicks = maxNbClicks,
					itemHit,
					dialogRunning = 0,
					nbShowSamples,
					offset,
					globalEventMask;
ResType			resType;

MenuHandle			menu[5],
					submenu[3],
					popUpMenu,
					popUpMenu2,
					patternMenu,
					loadMenus[kNbLoadMenus];
long				loadMenusDirID[kNbLoadMenus];
enum				{appleID = 1000, fileID, utilitiesID, windowID, editorID};
enum				{appleM = 0, fileM, utilitiesM, windowM, editorM};
Rect				dragRect,
					sourceRect,
					destRect;

sample				samples[maxNbSamples];
instrument			instruments[maxNbInstruments];
SndChannelPtr		insChannels[maxNbInsChannels];
char				currentNotes[maxNbInsChannels][2];
rythm				rythms[kPatternParts];
SndChannelPtr		rytChannels[kMaxRythmsChan];
Handle				noSound;

unsigned char		pattern[kNbPatterns][kPatternParts][kPatternLength];
Str63				patternName;
record				tempRec[nbNotes];

short				mainResFile,
					activeInstrument = 0,
					activeChan = 0,
					activePart = 0,
					activeRythmChan = 0;
long				position,
					patternPos = -1;
Str255				theString,
					prefName,
					pianoName;

SFTypeList			typeList_3,
					typeList_4;
Point				whereDLG,
					whereToShow;
OSErr				theError,
					interruptError;
char				theChar;

long				startTime = 0,
					interTime,
					tempVol,
					oldOutputVol;

Boolean			recording = false,
					playing = false,
					recorded = false,
					isCMusFile = false,
					saved = false,
					samplerPaused = false,
					MIDIDriverEnabled = false,
					MBarShown = true,
					updatePattern = false;

FSSpec				musicFileSpec;

prefs				thePrefs,
					recordPrefs;

char				piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd],
					c[maxNbShowSamples],
					uc[maxNbSamples];

SndCommand			theCommand;
SCStatus			channelStatus;

short				prefVersion = 321,
					pianoVersion = 321,
					msicVersion = 312,
					cmusVersion = 322,
					pattVersion = 312,
					curVersion = 330;
					
Point				newPoint,
					oldPoint;
short				button,
					trueButton,
					tempValue;
					
RgnHandle			mBarRgn;
PixPatHandle		backPat;
RGBColor			theBackColor,
					theBackColor2;
UniversalProcPtr	LoopTaskRoutine,
					ReadTaskRoutine,
					RythmeTaskRoutine,
					ODOC_Handler_Routine,
					QUIT_Handler_Routine,
					DialogHookRoutine,
					FileFilterRoutine;

ListHandle			instrumentsList,
					theList;
Rect				listSize,
					listRect;
Cell				theCell,
					actualCell;
long				posArea[kCycleSize];
long				writePos,
					readPos;

ExtTMTask			loopTask,
					readTask,
					rythmeTask;
					
// VARIABLES PLUG-INS:

short				importPlugInsNumber;
importPlugIn		importPlugIns[kMaxImportPlugsIns];
OSType				importFileTypes[kMaxImportPlugsIns * 2];

short				DTDPlugInsNumber;
DTDPlugIn			DTDPlugIns[kMaxDTDPlugsIns];
OSType				DTDFileTypes[kMaxDTDPlugsIns * 2];
SndChannelPtr		chanFinished = nil;

short				editorPlugInsNumber,
					currentEditor;
editorPlugIn		editorPlugIns[kMaxEditorPlugsIns];

//VARIABLES QUANTIZE:

short				nbSamplesToStart = 0;
short				samplesToStart[maxNbSamples];
short				nbSamplesToStop = 0;
short				samplesToStop[maxNbSamples];
short				nbRBMutes = 0;
short				RBMutes[kPatternParts];

// VARIABLES GRAPHIQUES:

Rect				aRect,
					picRect,
					shellRect,
					nameRect,
					playRect,
					volumeRect,
					pitchRect,
					panoramicRect,
					optionsRect;
					
Rect				recordRect,
					timeCodeRect,
					recRecRect,
					recEditRect,
					recStopRect,
					recPlayRect;
					
Rect				instrumentsRect,
					insListRect,
					insVolumeRect,
					insLoadRect,
					insOptionsRect;
					
Rect				rythmsRect,
					rytLoadRect,
					rytPlayRect,
					rytCurrentRect,
					rytNextRect,
					rytOptionsRect,
					rytNamesRect,
					rytMuteRect,
					rytPatternRect,
					rytPanoramicRect,
					rytVolumeRect;
					
//VARIABLES CURSEURS:

CCrsrHandle			mouseCursor,				
					waitCursor,
					handCursor;
#if(kCreateFileIDs)
HParamBlockRec		theBlock;
#endif
					
// CONSTANTES LOCALES:

#define				kProgress			6

//PROTOTYPES:

short				DM_Init();
void				DM_Dispose();

//FONCTIONS:

short Switch_Sample(short track)
{
	if(track > thePrefs.nbSamples || !samples[track].fileID)
	return false;
	
	if(event.modifiers & shiftKey) {
			if(event.modifiers & alphaLock) {
			if(samples[track].type == directToDiskType)
			return false;
			if(samples[track].totalSndNb == 1)
			return false;
			SF_LoadNextSample(track);
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSNext;
				tempRec[position].command = track;
				tempRec[position].command_2 = 0;
			}
			
			if(uc[track] != -1) {
				SetGWorld((CGrafPtr) mainWin, nil);
				SetRect(&aRect, 2 + 80 * uc[track], 1, 78 + 80 * uc[track], 18);
				InvalRect(&aRect);
			}
		}
		else {
			thePrefs.SVol[track] = thePrefs.SVol[track] + kProgress * (thePrefs.SMaxVol[track] / volPaddleLength);
			if(thePrefs.SVol[track] > thePrefs.SMaxVol[track])
			thePrefs.SVol[track] = thePrefs.SMaxVol[track];
			SCD_VolumeSample(track);
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSVolume;
				tempRec[position].command = track;
				tempRec[position].command_2 = thePrefs.SVol[track];
			}
			
			if(uc[track] != -1) {
				SetGWorld((CGrafPtr) mainWin, nil);
				SetRect(&aRect, 20 + 80 * uc[track], 82, 60 + 80 * uc[track], 208);
				InvalRect(&aRect);
			}
		}
		return true;
	}
	if(event.modifiers & controlKey) {
		if(event.modifiers & alphaLock) {
			if(samples[track].type == directToDiskType)
			return false;
			if(samples[track].totalSndNb == 1)
			return false;
			SF_LoadPrevSample(track);
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSPrev;
				tempRec[position].command = track;
				tempRec[position].command_2 = 0;
			}
			
			if(uc[track] != -1) {
				SetGWorld((CGrafPtr) mainWin, nil);
				SetRect(&aRect, 2 + 80 * uc[track], 1, 78 + 80 * uc[track], 18);
				InvalRect(&aRect);
			}
		}
		else {
			thePrefs.SVol[track] = thePrefs.SVol[track] - kProgress * (thePrefs.SMaxVol[track] / volPaddleLength);
			if(thePrefs.SVol[track] < 0)
			thePrefs.SVol[track] = 0;
			SCD_VolumeSample(track);
			
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSVolume;
				tempRec[position].command = track;
				tempRec[position].command_2 = thePrefs.SVol[track];
			}
			
			if(uc[track] != -1) {
				SetGWorld((CGrafPtr) mainWin, nil);
				SetRect(&aRect, 20 + 80 * uc[track], 82, 60 + 80 * uc[track], 208);
				InvalRect(&aRect);
			}
		}
		return true;
	}
	if(event.modifiers & optionKey) {
		thePrefs.SVol[track] = 0;
		SCD_VolumeSample(track);
		
		if(recording) {
			++position;
			tempRec[position].time = ActualTime;
			tempRec[position].action = kSVolume;
			tempRec[position].command = track;
			tempRec[position].command_2 = thePrefs.SVol[track];
		}
			
		if(uc[track] != -1) {
			SetGWorld((CGrafPtr) mainWin, nil);
			SetRect(&aRect, 20 + 80 * uc[track], 82, 60 + 80 * uc[track], 208);
			InvalRect(&aRect);
		}
		return true;
	}
	
	if(thePrefs.SPlaying[track]) {
		if(thePrefs.quantizeFlags & kQSmplStop && samples[track].type == sampleType) {
			for(i = 0; i < nbSamplesToStart; ++i)
			if(track == samplesToStart[i])
			return false;
			for(i = 0; i < nbSamplesToStop; ++i)
			if(track == samplesToStop[i])
			return false;
					
			samplesToStop[nbSamplesToStop] = track;
			++nbSamplesToStop;
		}
		else {
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSStop;
				tempRec[position].command = track;
				tempRec[position].command_2 = 0;
			}
			
			if(samples[track].type == sampleType)
			SCD_StopSample(track);
			else
			SCD_StopDirectToDisk(track);
		}
	}
	else {
		if(thePrefs.quantizeFlags & kQSmplStart && samples[track].type == sampleType) {
			for(i = 0; i < nbSamplesToStart; ++i)
			if(track == samplesToStart[i])
			return false;
			for(i = 0; i < nbSamplesToStop; ++i)
			if(track == samplesToStop[i])
			return false;
			
			samplesToStart[nbSamplesToStart] = track;
			++nbSamplesToStart;
		}
		else {
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kSStart;
				tempRec[position].command = track;
				tempRec[position].command_2 = 0;
			}
		
			if(samples[track].type == sampleType)
			SCD_StartSample(track);
			else
			SCD_StartDirectToDisk(track);
		}
	}
	if(uc[track] != -1) {
		SetGWorld((CGrafPtr) mainWin, nil);
		SetRect(&aRect, 39 + 80 * uc[track], 251, 79 + 80 * uc[track], 269);
		InvalRect(&aRect);
	}
	return true;
}
	
Boolean Test_FonctionKey()
{
	switch(theKey) {
		case kF1: Switch_Sample(1); break;
		case kF2: Switch_Sample(2); break;
		case kF3: Switch_Sample(3); break;
		case kF4: Switch_Sample(4); break;
		
		case kF5: Switch_Sample(5); break;
		case kF6: Switch_Sample(6); break;
		case kF7: Switch_Sample(7); break;
		case kF8: Switch_Sample(8); break;
		
		case kF9: Switch_Sample(9); break;
		case kF10: Switch_Sample(10); break;
		case kF11: Switch_Sample(11); break;
		case kF12: Switch_Sample(12); break;
		
		case kF13: Switch_Sample(13); break;
		case kF14: Switch_Sample(14); break;
		case kF15: Switch_Sample(15); break;
		
		default: return false; break;
	}
	
	return true;
}

Boolean Test_Numeric()
{
	switch(theKey) {
		case 20992: i = 0; break;
		case 21248: i = 1; break;
		case 21504: i = 2; break;
		case 21760: i = 3; break;
		case 22016: i = 4; break;
		case 22272: i = 5; break;
		case 22528: i = 6; break;
		case 22784: i = 7; break;
		case 23296: i = 8; break;
		case 23552: i = 9; break;
		
		default: return false; break;
	}
	
	itemHit = thePrefs.RNext;
	if(itemHit > 10)
	itemHit = itemHit - 10; //!si plus de 19 patterns
	if(itemHit > 1)
	itemHit = 0;
	itemHit = itemHit * 10 + i;
	thePrefs.RNext = itemHit;
	
	if(recording) {
		++position;
		tempRec[position].time = ActualTime;
		tempRec[position].action = kRNext;
		tempRec[position].command = thePrefs.RNext;
		tempRec[position].command_2 = 0;
	}
	
	if(thePrefs.rythmsWin) {
		SetGWorld((CGrafPtr) rythmsWin, nil);
		Draw_Rythms_Next();
	}
	
	return true;
}
	
void Warm_Up()
{
	long		response;
	Handle		textItem;
	NumVersion	version;
	short*		versPtr;
	short		vers;
	
	Init_Toolbox();
	for(i = 0; i < 4; ++i)
	MoreMasters();
	
	LoopTaskRoutine = NewTimerProc(TM_LoopTask);
	ReadTaskRoutine = NewTimerProc(TM_ReadTask);
	RythmeTaskRoutine = NewTimerProc(RB_RythmeTaskFonction);
	ODOC_Handler_Routine = NewAEEventHandlerProc(ODOC_Handler);
	QUIT_Handler_Routine = NewAEEventHandlerProc(QUIT_Handler);
	DialogHookRoutine = NewDlgHookYDProc(DialogHook);
	FileFilterRoutine = NewFileFilterYDProc(FileFilter);
	
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, ODOC_Handler_Routine, 0, false);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, QUIT_Handler_Routine, 0, false);

	mainResFile = CurResFile();
	dragRect = (**LMGetGrayRgn()).rgnBBox;
	nbShowSamples = dragRect.right / 80;
	if(nbShowSamples < minNbShowSamples)
	nbShowSamples = minNbShowSamples;
	if(nbShowSamples > maxNbShowSamples)
	nbShowSamples = maxNbShowSamples;
	
	Init_Menus();
	
	mouseCursor = GetCCursor(1000);
	waitCursor = GetCCursor(2000);
	handCursor = GetCCursor(3000);
	GetIndString(prefName, 0, 1);
	GetIndString(pianoName, 0, 2);
	backPat = GetPixPat(1000);
	
	version = SndSoundManagerVersion(); //Sound Manager < 3.1?
	versPtr = (short*) &version;
	if(*versPtr < 0x0310) {
		Do_Error(-30004, 405);
		ExitToShell();
	}
	
	/*Gestalt(gestaltQuickTimeVersion, &response); //QuickTime < 2.1?
	vers = HiWord(response);
	if(vers < 0x0210)
	Do_Error(-30004, 404);*/
	
	Gestalt(gestaltVMAttr, &response); //Mémoire virtuelle?
	if(response & virtualMemoryOn)
	Do_Error(-30005, 406);
	
	Anim_WaitCursor();
	
	noSound = Get1Resource('snd ', 128);
	DetachResource(noSound);
	SCD_CleanUpSound(noSound, false);
	HLock(noSound);
	
	if(!Read_Pref())
	Create_Pref();
	
#if demo
	Show_Pub();
#else
	if(thePrefs.registed[0] == 0) {
		if(!Register())
		ExitToShell();
	}
#endif
	
	Anim_WaitCursor();
	theDialog = GetNewDialog(1000, nil, (WindowPtr) -1);
	SetPort(theDialog);
	DrawDialog(theDialog);
	TextFont(200);
	TextSize(10);
	ForeColor(whiteColor);
	//GetIndString(theString, 800, 5);
	//MoveTo(370, 283);
	//DrawString(theString);
	GetIndString(theString, 800, 6);
	MoveTo(8, 277);
	DrawString(theString);
#if 0 //defined(powerc) || defined (__powerc)
	GetIndString(theString, 800, 7);
	MoveTo(340, 12);
	DrawString(theString);
#endif
	GetDItem(theDialog, 2, nil, &textItem, &aRect);
	
	CheckItem(submenu[0], (1000 - thePrefs.loopSpeed * 4) / 100, true);
	DisableItem(menu[1], 4);
	DisableItem(menu[1], 5);
	DisableItem(menu[1], 6);
#if defined(powerc) || defined (__powerc)
	
#else
	DisableItem(submenu[2], 5);
#endif
	
	Anim_WaitCursor();
	if(!Read_Piano())
	Create_Piano();
	
	Anim_WaitCursor();
	Initialization();
	Anim_WaitCursor();
	GetIndString(theString, 800, 1);
	SetIText(textItem, theString);
	Init_GWorlds();
	Anim_WaitCursor();
	SetGWorld((CGrafPtr) theDialog, nil);
	Clear_Music();
	for(i = 0; i < kNbPatterns; ++i)
	RB_ClearPattern(i);
	
	Anim_WaitCursor();
	Init_Driver();
	
	GetIndString(theString, 800, 2);
	SetIText(textItem, theString);
	Init_ImportPlugIns();
	Init_DTDPlugIns();
	Init_EditorPlugIns();
	
#if demo

#else
	Anim_WaitCursor();
	GetIndString(theString, 800, 3);
	SetIText(textItem, theString);
	Init_LoadMenus();
#endif
	
	Anim_WaitCursor();
	GetIndString(theString, 800, 4);
	SetIText(textItem, theString);
	if(thePrefs.enableMIDIDriver)
		if(!MIDI_InstallDriver())
		thePrefs.enableMIDIDriver = false;
	
	Init_RythmsBoxWindow();
	Init_SynthetizerWindow();
	Init_RecorderWindow();
	Init_SamplerWindow();
	
	DisposeDialog(theDialog);
	
	TM_InstallLoopTask();
	
	for(i = 0; i < thePrefs.nbSamples; ++i)
	thePrefs.SSndNb[i] = samples[i].sndNb;
	BlockMove(&thePrefs, &recordPrefs, sizeof(thePrefs));
	
	DM_Init();
	
	SetCCursor(mouseCursor);
	SelectWindow(mainWin);
}
	
void main()
{	
	Warm_Up();
	Check_StartUpFile();
	
	Event_Loop();
	
	End();
}

void Event_Loop()
{
	Boolean isEvent;
	
	while(run) {
		
		isEvent = WaitNextEvent(everyEvent, &event, kNbTicks, nil);
		
		if(isEvent)
		switch(event.what) {
			
			case mouseDown:
			Do_MouseDown(event);
			SetCCursor(mouseCursor);
			break;
			
			case keyDown:
			case autoKey:
			theChar = (event.message & charCodeMask);
			theKey = (event.message & keyCodeMask);
			
			if(dialogRunning == 1) {
				Act_PrefDialog(-1);
				break;
			}
			if(dialogRunning == 2) {
				Act_PianoDialog(-1);
				break;
			}
			if(dialogRunning == 3) {
				Act_EditDialog(-1);
				break;
			}
			if(dialogRunning == 4) {
				Act_MIDIDialog(-1);
				break;
			}
			if(dialogRunning == 5) {
				Act_ColorsDialog(-1);
				break;
			}
			if(dialogRunning == 6) {
				Act_EffectsDialog(-1);
				break;
			}
			if(dialogRunning == 7) {
				Act_QuantizeDialog(-1);
				break;
			}
			
			if(Test_FonctionKey())
			break;
			if(Test_Numeric())
			break;
			
			if(event.modifiers & cmdKey)
			Do_Menu(MenuKey(theChar));
			else {
				switch(theKey) {
					
					case kLeftArrowKey:
					if(event.modifiers & optionKey)
					Move_Max_Left();
					else
					Move_Left();
					break;
					
					case kRightArrowKey:
					if(event.modifiers & optionKey)
					Move_Max_Right();
					else
					Move_Right();
					break;
					
					case kDownArrowKey:
					GetPort(&savePort);
					SetGWorld((CGrafPtr) instrumentsWin, nil);
					BackColor(whiteColor);
					theCell.h = 0;
					theCell.v = activeInstrument;
					LSetSelect(false, theCell, instrumentsList);
					++activeInstrument;
					for(i = activeInstrument; i < maxNbInstruments; ++i)
					if(instruments[i].fileID) {
						activeInstrument = i;
						goto OK_1;
					}
					for(i = 0; i < activeInstrument; ++i)
					if(instruments[i].fileID) {
						activeInstrument = i;
						goto OK_1;
					}
					OK_1:
					theCell.v = activeInstrument;
					LSetSelect(true, theCell, instrumentsList);
					LAutoScroll(instrumentsList);
					RGBBackColor(&theBackColor);
					SetGWorld((CGrafPtr) savePort, nil);
					break;
					
					case kUpArrowKey:
					GetPort(&savePort);
					SetGWorld((CGrafPtr) instrumentsWin, nil);
					BackColor(whiteColor);
					theCell.h = 0;
					theCell.v = activeInstrument;
					LSetSelect(false, theCell, instrumentsList);
					--activeInstrument;
					for(i = activeInstrument; i > -1; --i)
					if(instruments[i].fileID) {
						activeInstrument = i;
						goto OK_2;
					}
					for(i = maxNbInstruments - 1; i > activeInstrument; --i)
					if(instruments[i].fileID) {
						activeInstrument = i;
						goto OK_2;
					}
					OK_2:
					theCell.v = activeInstrument;
					LSetSelect(true, theCell, instrumentsList);
					LAutoScroll(instrumentsList);
					RGBBackColor(&theBackColor);
					SetGWorld((CGrafPtr) savePort, nil);
					break;
					
					default:
					for(i = pianoSize + maxNbInstruments; i < pianoSize + maxNbInstruments + maxNbSamples; ++i)
						if(piano[i] == theChar) {
							Switch_Sample(i - pianoSize - maxNbInstruments);
							break;
						}
					for(i = pianoSize + maxNbInstruments + maxNbSamples; i < pianoSize + maxNbInstruments + maxNbSamples + kPatternParts; ++i)
					if(piano[i] == theChar) {
						if(thePrefs.quantizeFlags & kQRB && !(event.modifiers & optionKey)) {
							for(itemHit = 0; itemHit < nbRBMutes; ++itemHit)
							if(i - pianoSize - maxNbInstruments - maxNbSamples == RBMutes[itemHit])
							itemHit = 20000;
							
							if(itemHit != 20000) {
								RBMutes[nbRBMutes] = i - pianoSize - maxNbInstruments - maxNbSamples;
								++nbRBMutes;
								
								if(thePrefs.rythmsWin && thePrefs.rytWinExt) {
									SetGWorld((CGrafPtr) rythmsWin, nil);
									InvalRect(&rytMuteRect);
								}
							}
						}
						else {
							thePrefs.RPartMute[i - pianoSize - maxNbInstruments - maxNbSamples] = !thePrefs.RPartMute[i - pianoSize - maxNbInstruments - maxNbSamples];
		
							if(recording) {
								++position;
								tempRec[position].time = ActualTime;
								tempRec[position].action = kRMute;
								tempRec[position].command = i - pianoSize - maxNbInstruments - maxNbSamples;
								tempRec[position].command_2 = 0;
							}
							
							if(thePrefs.rythmsWin && thePrefs.rytWinExt) {
								SetGWorld((CGrafPtr) rythmsWin, nil);
								BackColor(whiteColor);
								ForeColor(blackColor);
								Draw_Rythms_Mute();
								RGBBackColor(&theBackColor);
							}
						}
						break;
					}
					if(theChar == piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 0]) {
						if(recording) {
							++position;
							tempRec[position].time = ActualTime;
							tempRec[position].action = kMPausRes;
							tempRec[position].command = 0;
							tempRec[position].command_2 = 0;
						}
						
						SCD_PauseResumeSampler();
						break;
					}
					if(theChar == piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + 1]) {
						if(thePrefs.RPlaying) {
							if(recording) {
								++position;
								tempRec[position].time = ActualTime;
								tempRec[position].action = kRStop;
								tempRec[position].command = 0;
								tempRec[position].command_2 = 0;
							}
							
							RB_Stop();
						}
						else {
							if(recording) {
								++position;
								tempRec[position].time = ActualTime;
								tempRec[position].action = kRPlay;
								tempRec[position].command = 0;
								tempRec[position].command_2 = 0;
							}
						
							RB_Start();
						}
						if(thePrefs.rythmsWin) {
							SetGWorld((CGrafPtr) rythmsWin, nil);
							InvalRect(&rytPlayRect);
						}
						break;
					}
					if(event.what == autoKey && (thePrefs.flags & kStopWhenKeyUp))
					break;
					for(i = pianoSize; i < pianoSize + maxNbInstruments; ++i)
					if(piano[i] == theChar && instruments[i - pianoSize].fileID) {
						if(recording) {
							++position;
							tempRec[position].time = ActualTime;
							tempRec[position].action = kIPlay;
							tempRec[position].command = i - pianoSize;
							tempRec[position].command_2 = baseKey;
						}
						
						SCD_PlayNote(i - pianoSize, baseKey);
					}
					if(!instruments[activeInstrument].fileID)
					break;
					for(i = 0; i < pianoSize; ++i)
					if(piano[i] == theChar) {
						if(recording) {
							++position;
							tempRec[position].time = ActualTime;
							tempRec[position].action = kIPlay;
							tempRec[position].command = activeInstrument;
							tempRec[position].command_2 = i;
						}
						
						SCD_PlayNote(activeInstrument, i);
						break;
					}
					break;
					
				}
			}
			break;
			
			case keyUp:
			if(!(thePrefs.flags & kStopWhenKeyUp))
			break;
			theChar = (event.message & charCodeMask);
			theKey = (event.message & keyCodeMask);
			for(i = pianoSize; i < pianoSize + maxNbInstruments; ++i)
			if(piano[i] == theChar && instruments[i - pianoSize].fileID) {
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kIStop;
					tempRec[position].command = i - pianoSize;
					tempRec[position].command_2 = baseKey;
				}
				
				SCD_StopNote(i - pianoSize, baseKey);
			}
			if(!instruments[activeInstrument].fileID)
			break;
			for(i = 0; i < pianoSize; ++i)
			if(piano[i] == theChar) {
				if(recording) {
					++position;
					tempRec[position].time = ActualTime;
					tempRec[position].action = kIStop;
					tempRec[position].command = activeInstrument;
					tempRec[position].command_2 = i;
				}
				
				SCD_StopNote(activeInstrument, i);
				break;
			}
			break;
			
			case osEvt:
			if(event.message & osEvtMessageMask & supendResumeEvtMask) {
				if(event.message & resumeMask) {
					HiliteControl(samplerScrollBar, 0);
					MIDI_Resume();
				}
				else {
					HiliteControl(samplerScrollBar, 255);
					MIDI_Suspend();
				}
			}
			break;
			
			/*case activateEvt:
			if((WindowPtr) event.message == mainWin) {
				if(event.modifiers & activeFlag)
				HiliteControl(samplerScrollBar, 0);
				else
				HiliteControl(samplerScrollBar, 255);
			}
			break;*/
			
			case updateEvt:
			Update_Windows();
			switch(dialogRunning) {
				
				case 1:
				Update_PrefDialog();
				break;
				
				case 2:
				Update_PianoDialog();
				break;
				
				case 3:
				Update_EditDialog();
				break;
				
				case 4:
				Update_MIDIDialog();
				break;
				
				case 5:
				Update_ColorsDialog();
				break;
				
				case 6:
				Update_EffectsDialog();
				break;
				
				case 7:
				Update_QuantizeDialog();
				break;
				
			}
			SetCCursor(mouseCursor);
			break;
			
			case kHighLevelEvent:
			if((OSType) event.message == typeAppleEvent)
			AEProcessAppleEvent(&event);
			break;
		
		}
		
		if(dialogRunning && IsDialogEvent(&event))
		if(DialogSelect(&event, &whichDialog, &itemHit)) {
			
			switch(dialogRunning) {
				
				case 1:
				Act_PrefDialog(itemHit);
				break;
				
				case 2:
				Act_PianoDialog(itemHit);
				break;
				
				case 3:
				Act_EditDialog(itemHit);
				break;
				
				case 4:
				Act_MIDIDialog(itemHit);
				break;
				
				case 5:
				Act_ColorsDialog(itemHit);
				break;
				
				case 6:
				Act_EffectsDialog(itemHit);
				break;
				
				case 7:
				Act_QuantizeDialog(itemHit);
				break;
				
			}
		}
			
		if(recording || playing) {
			if(playing && readPos != writePos) {
				Play_Music(posArea[readPos]);
				++readPos;
				if(readPos >= kCycleSize)
				readPos = 0;
			}
			
			SetGWorld(tempGWorld, nil);
			SetRect(&sourceRect, 451 + 3, 0 + 3, 451 + 100, 0 + 14);
			CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
				&sourceRect, &timeCodeRect, srcCopy, nil);
			//RGBForeColor(&theBackColor);
			//PaintRect(&timeCodeRect);
			RGBForeColor(&thePrefs.displayColor);
			MoveTo(6,12);
			NumToString(ActualTime, theString);
			DrawString(theString);
			ForeColor(blackColor);
			SetGWorld((CGrafPtr) recordWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(recordWin),
				&timeCodeRect, &timeCodeRect, srcCopy, nil);
			RGBBackColor(&theBackColor);
		}
		
		if(updatePattern) {
			if((thePrefs.quantizeFlags & kQSmplStart) || (thePrefs.quantizeFlags & kQSmplStop)) {
				SetGWorld((CGrafPtr) mainWin, nil);
				InvalRect(&playRect);
			}
			if(thePrefs.rythmsWin) {
				SetGWorld((CGrafPtr) rythmsWin, nil);
				if(thePrefs.rytWinExt) {
					BackColor(whiteColor);
					ForeColor(blackColor);
					Draw_Rythms_Pattern();
					if(thePrefs.quantizeFlags & kQRB)
					Draw_Rythms_Mute();
				}
				RGBBackColor(&theBackColor);
				Draw_Rythms_Current();
			}
			updatePattern = false;
		}
			
		if(chanFinished) {
			for(i = 0; i < thePrefs.nbSamples; ++i)
			if(samples[i].chan == chanFinished) {
				if(thePrefs.loopDTD)
				SCD_StartDirectToDisk(i);
				else {
					thePrefs.SPlaying[i] = false;
					itemHit = uc[i];
					if(itemHit != -1) {
						SetRect(&sourceRect, 0, 251, 80, 268);
						SetRect(&destRect, 80 * itemHit, 251, 80 + 80 * itemHit, 268);
						SetGWorld((CGrafPtr) mainWin, nil);
						BackColor(whiteColor);
						ForeColor(blackColor);
						CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(mainWin),
							&sourceRect, &destRect, srcCopy, nil);
						RGBBackColor(&theBackColor);
						Draw_Number(i);
					}
				}
				chanFinished = nil;
				break;
			}
		}
		
		/*if(!(thePrefs.MIDIFlags & kUseHFClock) && (thePrefs.quantizeFlags & kQSmplPitchAlign)) {
			SetGWorld((CGrafPtr) mainWin, nil);
			BackColor(whiteColor);
			ForeColor(blackColor);
			for(i = 0; i < nbShowSamples; ++i)
			if(samples[c[i]].fileID && samples[c[i]].type == sampleType) {
				SetRect(&sourceRect, 0, 213, 80, 229);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&sourceRect, &sourceRect, srcCopy, nil);
				itemHit = 40 + thePrefs.SRelativeRate[c[i]];
				SetRect(&sourceRect, 472, 120, 486, 136);
				SetRect(&destRect, itemHit - 7, 213, itemHit + 7, 229);
				CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
					&sourceRect, &destRect, srcCopy, nil);
				SetRect(&sourceRect, 0, 213, 80, 229);
				SetRect(&destRect, 80 * i, 213, 80 + 80 * i, 229);
				CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(mainWin),
					&sourceRect, &destRect, srcCopy, nil);
			}
			RGBBackColor(&theBackColor);
		}*/
		
		Str31		timeString;
		if(thePrefs.flags & kDTDTime) {
			for(i = 0; i < thePrefs.nbSamples; ++i)
			if(samples[i].type == directToDiskType && thePrefs.SPlaying[i]) {
				itemHit = uc[i];
				if(itemHit != -1) {
					BlockMove("\p00:00:00", timeString, 8);
					timeString[0] = 8;
					NumToString((TickCount() - samples[i].time) / 3600, theString); //Minutes
					BlockMove(&theString[1], &timeString[3 - theString[0]], theString[0]);
					NumToString((TickCount() - samples[i].time) / 60 % 60, theString); //Secondes
					BlockMove(&theString[1], &timeString[6 - theString[0]], theString[0]);
					NumToString(((TickCount() - samples[i].time) * 5 / 3) % 100, theString); //Centièmes
					BlockMove(&theString[1], &timeString[9 - theString[0]], theString[0]);
					
					SetGWorld(tempGWorld, nil);
					ForeColor(blackColor);
					SetRect(&sourceRect, 0 + 3, 0 + 22, 0 + 76, 0 + 33);
					CopyBits(GWBitMapPtr(picGWorld), GWBitMapPtr(tempGWorld),
						&sourceRect, &sourceRect, srcCopy, nil);
					RGBForeColor(&thePrefs.displayColor);
					MoveTo(7,31);
					DrawString(timeString);
					ForeColor(blackColor);
					SetGWorld((CGrafPtr) mainWin, nil);
					BackColor(whiteColor);
					ForeColor(blackColor);
					SetRect(&destRect, 3 + 80 * itemHit, 22, 76 + 80 * itemHit, 33);
					CopyBits(GWBitMapPtr(tempGWorld), GWBitMapPtr(mainWin),
						&sourceRect, &destRect, srcCopy, nil);
					RGBBackColor(&theBackColor);
				}
			}
		}
		
		if(interruptError) {
			Do_Error(interruptError, 508);
			interruptError = 0;
		}
		
	}
}

OSErr Music2AIFF();

void Do_Menu(long result)
{
	short	theItem = LoWord(result);
	short	theMenu = HiWord(result);
	
	switch(theMenu) {
	
		case appleID:
		switch(theItem) {
			case 1:
#if demo
			Show_Pub();
#endif
			Show_AboutDialog();
			break;
			
			/*case 2:
			Register();
			break;*/
			
			default:
			Str255		name;
			
			GetMenuItemText(menu[appleM], theItem, name);
			OpenDeskAcc(name);
			break;
		}
		break;
		
		case fileID:
		switch(theItem) {
			
			case 1:
			if(!AskForSaving())
			break;
			Clear_Music();
			position = 0;
			recorded = false;
			saved = false;
			isCMusFile = false;
			nbSamplesToStart = 0;
			nbSamplesToStop = 0;
			nbRBMutes = 0;
			
			for(i = 0; i < thePrefs.nbSamples; ++i) { //Reset sampler
				SF_CloseSample(i);
				thePrefs.SGroup[i]			= -1;
				thePrefs.SRate[i]			= 0;
				thePrefs.SRelativeRate[i]	= 0;
				thePrefs.SPan[i]			= 100;
				thePrefs.SLoop[i]			= 0;
				thePrefs.SMaxVol[i]			= defaultMaxVolume;
				thePrefs.SVol[i]			= defaultMaxVolume;
				
#if defined(powerc) || defined (__powerc)
				thePrefs.SSourceMode[i]		= kSSpSourceMode_Unfiltered;
				SCD_EffectsSample(i);
#endif
			}
			
			theCommand.cmd		= quietCmd; //Reset synthétiseur
			theCommand.param1	= 0;
			theCommand.param2	= 0;
			for(i = 0; i < thePrefs.nbInsChannels; ++i)
			SndDoImmediate(insChannels[i], &theCommand);
			UseResFile(mainResFile);
			GetIndString(theString, 0, 11);
			theCell.h = 0;
			for(i = 0; i < maxNbInstruments; ++i) {
				SF_CloseInstrument(i);
				theCell.v = i;
				LSetCell(&theString[1], theString[0], theCell, instrumentsList);
			}
			thePrefs.IMaxVol = defaultMaxVolume;
			thePrefs.IVol = defaultMaxVolume;
			
			if(thePrefs.RPlaying) //Reset boîte à rythmes
			RB_Stop();
			theCommand.cmd		= quietCmd;
			theCommand.param1	= 0;
			theCommand.param2	= 0;
			for(i = 0; i < thePrefs.nbRytChannels; ++i)
			SndDoImmediate(rytChannels[i], &theCommand);
			thePrefs.RBPM = 120;
			interTime = -(15000000 / thePrefs.RBPM);
			thePrefs.RCurrent = 0;
			thePrefs.RNext = 0;
			thePrefs.RVol = defaultMaxVolume;
			thePrefs.RMaxVol = defaultMaxVolume;
			//SCD_VolumeRythmsBox();
			for(i = 0; i < kPatternParts; ++i) {
				thePrefs.RPartVol[i] = kDefaultPartVolume;
				thePrefs.RPartPan[i] = 100;
				thePrefs.RPartPitch[i] = baseKey;
				thePrefs.RPartMute[i] = false;
			}
			for(i = 0; i < kPatternParts; ++i)
			RB_CloseSound(i);
			UseResFile(mainResFile);
			GetIndString(patternName, 0, 15);
			for(i = 0; i < kNbPatterns; ++i)
			RB_ClearPattern(i);
		
			activeInstrument = 0; //Reset général
			musicFileSpec.vRefNum = 0;
			musicFileSpec.parID = 0;
			musicFileSpec.name[0] = 0;
			DisableItem(menu[1], 4);
			DisableItem(menu[1], 5);
			DisableItem(menu[1], 6);
			//CheckItem(menu[2], 7, false); -- ?
			
			SetGWorld((CGrafPtr) mainWin, nil); //Update
			InvalRect(&shellRect);
			if(thePrefs.instrumentsWin) {
				SetGWorld((CGrafPtr) instrumentsWin, nil);
				InvalRect(&instrumentsRect);
			}
			if(thePrefs.rythmsWin) {
				SetGWorld((CGrafPtr) rythmsWin, nil);
				InvalRect(&rythmsRect);
			}
			break;
			
			case 2:
			if(AskForSaving()) {
				StandardFileReply	theReply;
	
				UseResFile(mainResFile);
				CustomGetFile(nil, 2, typeList_3, &theReply, 20000, whereToShow, nil, nil, nil, nil, nil);
				if(theReply.sfGood) {
					if(theReply.sfType == 'Msic')
					Open_Music(&theReply.sfFile);
					if(theReply.sfType == 'CMus')
					Decompile_Music(&theReply.sfFile);
				}
			}
			break;
			
			case 4:
			if(recorded && musicFileSpec.parID)
			Write_Music();
			break;
			
			case 5:
			if(recorded)
			Save_Music();
			break;
			
			case 10:
			if(AskForSaving())
			run = false;
			break;
		
		}
		break;
		
		case utilitiesID:
		switch(theItem) {
			
			case 2:
			if(recording) {
				++position;
				tempRec[position].time = ActualTime;
				tempRec[position].action = kMPausRes;
				tempRec[position].command = 0;
				tempRec[position].command_2 = 0;
			}
			
			SCD_PauseResumeSampler();
			break;
			
			case 3:
			for(i = 0; i < thePrefs.nbSamples; ++i)
			if(samples[i].fileID && samples[i].type == sampleType) {
				SCD_AlignSample(i);
				if(thePrefs.SPlaying[i])
				SCD_PitchSample(i);
			}
			SetGWorld((CGrafPtr) mainWin, nil);
			InvalRect(&pitchRect);
			break;
			
			case 5:
			instruments[activeInstrument].spec.vRefNum	= 0;
			instruments[activeInstrument].spec.parID 	= 0;
			instruments[activeInstrument].spec.name[0]	= 0;
			if(instruments[activeInstrument].fileID) {
				SF_CloseInstrument(activeInstrument);
				GetPort(&savePort);
				SetGWorld((CGrafPtr) instrumentsWin, nil);
				BackColor(whiteColor);
				theCell.h = 0;
				theCell.v = activeInstrument;
				GetIndString(theString, 0, 11);
				LSetCell(&theString[1], theString[0], theCell, instrumentsList);
				LSetSelect(false, theCell, instrumentsList);
				++activeInstrument;
				for(i = activeInstrument; i < maxNbInstruments; ++i)
				if(instruments[i].fileID) {
					activeInstrument = i;
					goto OK_3;
				}
				for(i = 0; i < activeInstrument; ++i)
				if(instruments[i].fileID) {
					activeInstrument = i;
					goto OK_3;
				}
				OK_3:
				theCell.v = activeInstrument;
				LSetSelect(true, theCell, instrumentsList);
				LAutoScroll(instrumentsList);
				RGBBackColor(&theBackColor);
				SetGWorld((CGrafPtr) savePort, nil);
			}
			break;
			
			case 7:
			for(i = 0; i < thePrefs.nbSamples; ++i)
			thePrefs.SSndNb[i] = samples[i].sndNb;
			BlockMove(&thePrefs, &recordPrefs, sizeof(thePrefs));
			saved = false;
			if(musicFileSpec.parID && recorded)
			EnableItem(menu[1], 4);
			break;
			
			case 8:
			Install_Music();
			SetGWorld((CGrafPtr) mainWin, nil);
			InvalRect(&shellRect);
			if(thePrefs.instrumentsWin) {
				SetGWorld((CGrafPtr) instrumentsWin, nil);
				InvalRect(&instrumentsRect);
			}
			if(thePrefs.rythmsWin) {
				SetGWorld((CGrafPtr) rythmsWin, nil);
				InvalRect(&rythmsRect);
			}
			break;
			
			case 10:
			ToggleMBarState();
			break;
			
		}
		break;
		
		case windowID:
		switch(theItem) {
			
			/*case 1:
			if(thePrefs.mainWin && (thePrefs.recordWin || thePrefs.instrumentsWin)) {
				DisposeControl(samplerScrollBar);
				DisposeWindow(mainWin);
				thePrefs.mainWin = false;
				CheckItem(menu[3], 1, false);
			}
			if(!thePrefs.mainWin)
			Init_SamplerWindow();
			break;*/
			
			case 1:
			if(!recording && !playing) {
				if(thePrefs.recordWin) {
					HideWindow(recordWin);
					//DisposeWindow(recordWin);
					thePrefs.recordWin = false;
					CheckItem(menu[3], 1, false);
					SetGWorld((CGrafPtr) mainWin, nil);
				}
				else {
					ShowWindow(recordWin);
					SelectWindow(recordWin);
					SetGWorld((CGrafPtr) recordWin, nil);
					InvalRect(&recordRect);
					thePrefs.recordWin = true;
					CheckItem(menu[3], 1, true);
					//Init_RecorderWindow();
				}
			}
			break;
			
			case 2:
			if(!recording && !playing) {
				if(thePrefs.instrumentsWin) {
					HideWindow(instrumentsWin);
					//DisposeWindow(instrumentsWin);
					thePrefs.instrumentsWin = false;
					CheckItem(menu[3], 2, false);
					SetGWorld((CGrafPtr) mainWin, nil);
				}
				else {
					ShowWindow(instrumentsWin);
					SelectWindow(instrumentsWin);
					SetGWorld((CGrafPtr) instrumentsWin, nil);
					InvalRect(&instrumentsRect);
					thePrefs.instrumentsWin = true;
					CheckItem(menu[3], 2, true);
					//Init_SynthetizerWindow();
				}
			}
			break;
			
			case 3:
			if(!recording && !playing) {
				if(thePrefs.rythmsWin) {
					HideWindow(rythmsWin);
					//DisposeWindow(instrumentsWin);
					thePrefs.rythmsWin = false;
					CheckItem(menu[3], 3, false);
					SetGWorld((CGrafPtr) mainWin, nil);
				}
				else {
					ShowWindow(rythmsWin);
					SelectWindow(rythmsWin);
					SetGWorld((CGrafPtr) rythmsWin, nil);
					InvalRect(&rythmsRect);
					thePrefs.rythmsWin = true;
					CheckItem(menu[3], 3, true);
					//Init_SynthetizerWindow();
				}
			}
			break;
			
			case 5:
			Reset_Windows();
			break;
		
		}
		break;
		
		case editorID:
		CheckItem(menu[4], currentEditor + 1, false);
		currentEditor = theItem - 1;
		CheckItem(menu[4], currentEditor + 1, true);
		break;
		
		case kHMHelpMenuID:
		if(theItem == 5)
		Show_QuickHelp();
		break;
		
		case 100:
		switch(theItem) {
			case 1:
			if(recorded)
			Compile_Music();
			break;
			
			case 2:
			Music2AIFF();
			break;
			
		}
		break;
		
		case 110:
		thePrefs.loopSpeed = (1000 - theItem * 100) / 4;
		if(recording) {
			++position;
			tempRec[position].time = ActualTime;
			tempRec[position].action = kMLoopSpeed;
			tempRec[position].command = theItem;
			tempRec[position].command_2 = 0;
		}
		for(i = 1; i < 10; ++i)
		CheckItem(submenu[0], i, false);
		CheckItem(submenu[0], theItem, true);
		break;
		
		case 120:
		switch(theItem) {
			case 1:
			Init_PrefDialog();
			break;
			
			case 2:
			Init_PianoDialog();
			break;
			
			case 3:
			Init_ColorsDialog();
			break;
			
			case 4:
#if demo
			Do_Error(-30008, 904);
#else
			Init_MIDIDialog();
#endif
			break;
			
			case 5:
#if demo
			Do_Error(-30008, 904);
#else
			Init_EffectsDialog();
#endif
			break;
			
			case 6:
			Init_QuantizeDialog();
			break;
			
		}
		break;
		
	}
	HiliteMenu(0);		
}

void Do_MouseDown(EventRecord theEvent)
{
#if demo
		--nbClicks;
		if(!nbClicks) {
			Do_Error(-30001, 905);
			End();
		}
#endif
	
	whereClick = FindWindow(theEvent.where, &whichWin);
	
	switch(whereClick) {
		
		case inDrag:
		Point		tempPoint = {0,0};
			
		if(!dialogRunning) {
			DragWindow(whichWin, theEvent.where, &dragRect);
			
			GetPort(&savePort);
			SetGWorld((CGrafPtr) whichWin, nil);
			GlobalToLocal(&tempPoint);
			if(whichWin == mainWin) {
				thePrefs.smpWinPos.h = - tempPoint.h;
				thePrefs.smpWinPos.v = - tempPoint.v;
			}
			if(whichWin == recordWin) {
				thePrefs.recWinPos.h = - tempPoint.h;
				thePrefs.recWinPos.v = - tempPoint.v;
			}
			if(whichWin == instrumentsWin) {
				thePrefs.insWinPos.h = - tempPoint.h;
				thePrefs.insWinPos.v = - tempPoint.v;
			}
			if(whichWin == rythmsWin) {
				thePrefs.rytWinPos.h = - tempPoint.h;
				thePrefs.rytWinPos.v = - tempPoint.v;
			}
			SetGWorld((CGrafPtr) savePort, nil);
		}
		else
			if(whichWin == theDialog)
			DragWindow(whichWin, theEvent.where, &dragRect);
		break;
		
		case inMenuBar:
		if(!dialogRunning)
		Do_Menu(MenuSelect(theEvent.where));
		else {
			MenuSelect(theEvent.where);
			HiliteMenu(0);
		}
		break;
		
		case inContent:
		if(dialogRunning)
		break;
		if(whichWin == mainWin) {
			SetGWorld((CGrafPtr) mainWin, nil);
			GlobalToLocal(&event.where);
			if(!Act_Sampler(event.where))
			SelectWindow(mainWin);
		}
		if(whichWin == instrumentsWin) {
			SetGWorld((CGrafPtr) instrumentsWin, nil);
			GlobalToLocal(&event.where);
			if(!Act_Synthetizer(event.where))
			SelectWindow(instrumentsWin);
		}
		if(whichWin == rythmsWin) {
			SetGWorld((CGrafPtr) rythmsWin, nil);
			GlobalToLocal(&event.where);
			if(!Act_RythmsBox(event.where))
			SelectWindow(rythmsWin);
		}
		if(whichWin == recordWin) {
			SetGWorld((CGrafPtr) recordWin, nil);
			GlobalToLocal(&event.where);
			Act_Recorder(event.where);
			//if(!Act_Recorder(event.where))
			//SelectWindow(recordWin);
		}
		break;
		
		case inGoAway:
		if(dialogRunning)
		break;
		
		/*if(whichWin == mainWin) {
			if(TrackGoAway(mainWin, event.where) && (thePrefs.recordWin || thePrefs.instrumentsWin)) {
				DisposeControl(samplerScrollBar);
				DisposeWindow(mainWin);
				thePrefs.mainWin = false;
				CheckItem(menu[3], 1, thePrefs.mainWin);
			}
		}*/
		if(whichWin == recordWin) {
			if(TrackGoAway(recordWin, event.where) && !recording  && !playing) {
				HideWindow(recordWin);
				//DisposeWindow(recordWin);
				thePrefs.recordWin = false;
				CheckItem(menu[3], 1, thePrefs.recordWin);
				SetGWorld((CGrafPtr) mainWin, nil);
			}
		}
		if(whichWin == instrumentsWin) {
			if(TrackGoAway(instrumentsWin, event.where) && !recording  && !playing) {
				HideWindow(instrumentsWin);
				//DisposeWindow(instrumentsWin);
				thePrefs.instrumentsWin = false;
				CheckItem(menu[3], 2, thePrefs.instrumentsWin);
				SetGWorld((CGrafPtr) mainWin, nil);
			}
		}
		if(whichWin == rythmsWin) {
			if(TrackGoAway(rythmsWin, event.where) && !recording  && !playing) {
				HideWindow(rythmsWin);
				//DisposeWindow(instrumentsWin);
				thePrefs.rythmsWin = false;
				CheckItem(menu[3], 3, thePrefs.rythmsWin);
				SetGWorld((CGrafPtr) mainWin, nil);
			}
		}
		break;
		
		case inZoomIn:
		case inZoomOut:
		if(dialogRunning)
		break;
		
		if(whichWin == instrumentsWin) {
			if(TrackBox(instrumentsWin, event.where, whereClick)) {
				if(thePrefs.insWinExt)
				SizeWindow(instrumentsWin, 205, 150, true);
				else
				SizeWindow(instrumentsWin, 288, 150, true);
				thePrefs.insWinExt = !thePrefs.insWinExt;
			}
		}
		if(whichWin == rythmsWin) {
			if(TrackBox(rythmsWin, event.where, whereClick)) {
				if(thePrefs.rytWinExt)
				SizeWindow(rythmsWin, 320, 41, true);
				else
				SizeWindow(rythmsWin, 320, 150, true);
				thePrefs.rytWinExt = !thePrefs.rytWinExt;
			}
		}
		break;
	}
}
	
typedef OSErr (*QIPI_Proc)();
typedef OSErr (*QDDPI_Proc)();
typedef OSErr (*QEPI_Proc)();

void End()
{
	SymClass			theClass;
	QIPI_Proc			theProcPtr;
	QDDPI_Proc			theProcPtr_2;
	QEPI_Proc			theProcPtr_3;
	
	//Dispose VBL Tasks
	TM_KillLoopTask();
	if(playing)
	TM_KillReadTask();
	if(thePrefs.RPlaying)
	RB_Stop();
	
	if(!MBarShown)
	ToggleMBarState();
	
	Update_Pref();
	
	//Dispose GWorld
	UnlockPixels(GetGWorldPixMap(picGWorld));
	DisposeGWorld(picGWorld);
	UnlockPixels(GetGWorldPixMap(tempGWorld));
	DisposeGWorld(tempGWorld);
	
	//Dispose DragManager
	DM_Dispose();
	
	//Dispose Interface
	DisposeControl(samplerScrollBar);
	DisposeWindow(mainWin);
	DisposeWindow(recordWin);
	DisposeWindow(instrumentsWin);
	DisposeWindow(rythmsWin);
	
	//Dispose Sound Channels
	for(i = 0; i < thePrefs.nbSamples; ++i)
	SndDisposeChannel(samples[i].chan, true);
	for(i = 0; i < thePrefs.nbInsChannels; ++i)
	SndDisposeChannel(insChannels[i], true);
	for(i = 0; i < thePrefs.nbRytChannels; ++i)
	SndDisposeChannel(rytChannels[i], true);
	SetDefaultOutputVolume(oldOutputVol);
	
	//Dispose MIDI
	if(MIDIDriverEnabled)
	MIDI_KillDriver();
	
	HUnlock(noSound);
	DisposeHandle(noSound);
	
	//Dispose Apple Events
	AERemoveEventHandler(kCoreEventClass, kAEOpenDocuments, ODOC_Handler_Routine, false);
	AERemoveEventHandler(kCoreEventClass, kAEQuitApplication, QUIT_Handler_Routine, false);

	//Dispose Import Plug-Ins
	for(i = 0; i < importPlugInsNumber; ++i) {
		GetIndString(theString, 0, 24);
		if(!FindSymbol(importPlugIns[i].connID, theString, (char**) &theProcPtr, &theClass))
		theError = (*theProcPtr)();
		CloseConnection(&importPlugIns[i].connID);
	}
	
	//Dispose Direct To Disk Plug-Ins
	for(i = 0; i < DTDPlugInsNumber; ++i) {
		GetIndString(theString, 0, 32);
		if(!FindSymbol(DTDPlugIns[i].connID, theString, (char**) &theProcPtr_2, &theClass))
		theError = (*theProcPtr_2)();
		CloseConnection(&DTDPlugIns[i].connID);
	}
	
	//Dispose Editor Plug-Ins
	for(i = 0; i < editorPlugInsNumber; ++i) {
		GetIndString(theString, 0, 37);
		if(!FindSymbol(editorPlugIns[i].connID, theString, (char**) &theProcPtr_3, &theClass))
		theError = (*theProcPtr_3)();
		CloseConnection(&editorPlugIns[i].connID);
		CloseResFile(editorPlugIns[i].resFile);
	}
	
	//Restore Event Mask
	SetEventMask(globalEventMask);
	
	ExitToShell();
}

//APPLE EVENTS ROUTINES:

pascal OSErr ODOC_Handler(AppleEvent* theAppleEvent, AppleEvent* reply, long handlerRefCon)
{	AEDescList		docList;
	AEKeyword		keyword;
	DescType		returnedType;
	FSSpec			theFSSpec;
	Size			actualSize;
	FInfo			fileInfo;
	//long			itemsInList;
	
	theError = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if(theError)
	return theError;
	/*theError = AECountItems(&docList, &itemsInList);
	if(theError)
	return theError;*/
	theError = AEGetNthPtr(&docList, 1, typeFSS, &keyword, &returnedType, (Ptr) &theFSSpec, sizeof(FSSpec), &actualSize);
	if(theError)
	return theError;
	theError = FSpGetFInfo(&theFSSpec, &fileInfo);
	if(theError)
	return theError;
	
	if(fileInfo.fdType == 'Msic' && !(playing || recording))
		if(AskForSaving())
		Open_Music(&theFSSpec);
		
	if(fileInfo.fdType == 'CMus' && !(playing || recording))
		if(AskForSaving())
		Decompile_Music(&theFSSpec);
		
	if(fileInfo.fdType == 'Patt' && !thePrefs.RPlaying)
	RB_LoadPatternSet(&theFSSpec);
	
	AEDisposeDesc(&docList);
	
	return nil;
}

pascal OSErr QUIT_Handler(AppleEvent* theAppleEvent, AppleEvent* reply, long handlerRefCon)
{	if(AskForSaving())
	End();
	
	return nil;
}

//DRAG MANAGER ROUTINES:

Boolean			dragEnabled = false,
					canAcceptDrag = false;
UniversalProcPtr	TrackingHandlerRoutine,
					ReceiveHandlerRoutine;

Boolean FileTypeAvailable(DragReference theDrag)
{
	unsigned short items,
					index;
	FlavorFlags		theFlags;
	ItemReference	theItem;
	
	CountDragItems(theDrag, &items);
	
	for(index = 1; index <= items; ++index) {
		GetDragItemReferenceNumber(theDrag, index, &theItem);
		if(!GetFlavorFlags(theDrag, theItem, 'hfs ', &theFlags))
		continue;
		
		return false;
	}
	
	return true;
}

OSErr TrackingHandler(DragTrackingMessage theMessage, WindowPtr theWindow, Ptr appA5, DragReference theDrag)
{
#if kA5savy
	long			oldA5 = SetA5((long) appA5);
#endif    
	Point			mouse,
					windowPos = {0,0};
	DragAttributes	attributes;
	RgnHandle		hiliteRgn;
	Rect			tempRect;
	GrafPtr			oldPort;
	
	GetPort(&oldPort);
	SetGWorld((CGrafPtr) theWindow, nil);
	LocalToGlobal(&windowPos);
	
	GetDragAttributes(theDrag, &attributes);
	switch(theMessage) {
		
		case dragTrackingEnterHandler:
		break;
		
        case dragTrackingEnterWindow:
		canAcceptDrag = FileTypeAvailable(theDrag);
		if(playing || recording)
		canAcceptDrag = false;
		if(theWindow == rythmsWin && thePrefs.RPlaying)
		canAcceptDrag = false;
		break;
            
		case dragTrackingInWindow:
		if(!canAcceptDrag)
		break;
		GetDragMouse(theDrag, &mouse, 0L);
		if(attributes & dragHasLeftSenderWindow) {
			if(theWindow == mainWin)
			tempRect = shellRect;
			if(theWindow == instrumentsWin)
			tempRect = instrumentsRect;
			if(theWindow == rythmsWin)
			tempRect = rythmsRect;
			tempRect.left += windowPos.h;
			tempRect.right += windowPos.h;
			tempRect.top += windowPos.v;
			tempRect.bottom += windowPos.v;
			
			if(PtInRect(mouse, &tempRect)) {
				hiliteRgn = NewRgn();
				tempRect.left -= windowPos.h;
				tempRect.right -= windowPos.h;
				tempRect.top -= windowPos.v;
				tempRect.bottom -= windowPos.v;
				RectRgn(hiliteRgn, &tempRect);
				ShowDragHilite(theDrag, hiliteRgn, true);
				DisposeRgn(hiliteRgn);
			}
			else
			HideDragHilite(theDrag);
		}
		break;
		
        case dragTrackingLeaveWindow:
		if(canAcceptDrag)
		HideDragHilite(theDrag);
		canAcceptDrag = false;
		break;
		
		case dragTrackingLeaveHandler:
		break;
    }
    SetGWorld((CGrafPtr) oldPort, nil);
#if kA5savy
	oldA5 = SetA5(oldA5);
#endif    
    return nil;
}

typedef OSErr (*LIPI_Proc)(FSSpecPtr, Handle*, long, Fixed*);

OSErr ReceiveHandler(WindowPtr theWindow, Ptr appA5, DragReference theDrag)
{
#if kA5savy
	long			oldA5 = SetA5((long) appA5);
#endif
	unsigned short items,
					index;
	short			num,
					tempID;
	ItemReference	theItem;
	FlavorFlags		theFlags;
	Size			dataSize;
	Cell			tempCell;
	HFSFlavor		theFlavor;
	Boolean		refuse = false;
	GrafPtr			oldPort;
	long			j;
	SymClass		theClass;
	LIPI_Proc		theProcPtr;
	Boolean		loop;
	Fixed			rate;
	
	GetPort(&oldPort);
	SetGWorld((CGrafPtr) theWindow, nil);
	CountDragItems(theDrag, &items);
	
	if(!canAcceptDrag)
	refuse = true;
	else
	for(index = 1; index <= items; ++index) {
		GetDragItemReferenceNumber(theDrag, index, &theItem);
		if(!GetFlavorFlags(theDrag, theItem, 'hfs ', &theFlags)) {
			GetFlavorDataSize(theDrag, theItem, 'hfs ', &dataSize);
			GetFlavorData(theDrag, theItem, 'hfs ', &theFlavor, &dataSize, 0L);
			
			if(theWindow == mainWin) {
				//Trouve une place
				for(num = 0; num < thePrefs.nbSamples; ++num)
					if(!samples[num].fileID)
					break;
				if(num == thePrefs.nbSamples) {
					refuse = true;
					continue;
				}
				/*if(isCMusFile) {
					refuse = true;
					continue;
				}*/
					
				if(theFlavor.fileType != 'Bank') {
					//Trouve le Plug-In adéquat
					i = 0; j = 0; loop = true;
					while(loop) {
						for(j = 0; j < kTypesPerImport; ++j)
						if(importPlugIns[i].importFileTypes[j] == theFlavor.fileType)
						loop = false;
						
						if(loop)
						++i;
						if(i >= importPlugInsNumber) {
							SysBeep(0);
							refuse = true;
							loop = false;
						}
					}
					
					if(refuse)
					continue;
					
					//Appelle le Plug-In
					GetIndString(theString, 0, 23);
					theError = FindSymbol(importPlugIns[i].connID, theString, (char**) &theProcPtr, &theClass);
					if(theError) {
						SysBeep(0); //Do_Error(theError, 105);
						continue;
					}
					if(thePrefs.flags & kByPassHeaders)
					theError = (*theProcPtr)(&theFlavor.fileSpec, &samples[num].snd, kLoopSound + kNewHeader, &thePrefs.SRate[num]);
					else
					theError = (*theProcPtr)(&theFlavor.fileSpec, &samples[num].snd, kLoopSound, &thePrefs.SRate[num]);
					if(theError) {
						SysBeep(0); //Do_Error(theError, 106);
						continue;
					}
					thePrefs.SRelativeRate[num] = 0;
					HLock(samples[num].snd);
					UseResFile(mainResFile);
					
					BlockMove(theFlavor.fileSpec.name, samples[num].name, sizeof(Str63));
					while(StringWidth(samples[num].name) > maxWidthSmpl)
					--samples[num].name[0];
					
					samples[num].fileID = -1;
					samples[num].sndNb = 1;
					samples[num].totalSndNb = 1;
				}
				else
				SF_OpenBank(&theFlavor.fileSpec, num);
	
				samples[num].type = sampleType;
				samples[num].spec.vRefNum = theFlavor.fileSpec.vRefNum;
				samples[num].spec.parID = theFlavor.fileSpec.parID;
				BlockMove(theFlavor.fileSpec.name, samples[num].spec.name, sizeof(Str63));
				
				theCommand.cmd		= soundCmd;
				theCommand.param1	= 0;
				theCommand.param2	= (long) *samples[num].snd;
				SndDoImmediate(samples[num].chan, &theCommand);
	
				if(thePrefs.quantizeFlags & kQSmplPitchAlign)
				SCD_AlignSample(num);
				
#if(kCreateFileIDs)
				theBlock.fidParam.ioNamePtr = theFlavor.fileSpec.name;
				theBlock.fidParam.ioVRefNum = theFlavor.fileSpec.vRefNum;
				theBlock.fidParam.ioSrcDirID = theFlavor.fileSpec.parID;
				PBCreateFileIDRef(&theBlock, false);
#endif
				InvalRect(&shellRect);
			}
			
			if(theWindow == instrumentsWin) {
				//Trouve le Plug-In adéquat
				i = 0; j = 0; loop = true;
				while(loop) {
					for(j = 0; j < kTypesPerImport; ++j)
					if(importPlugIns[i].importFileTypes[j] == theFlavor.fileType)
					loop = false;
					
					if(loop)
					++i;
					if(i >= importPlugInsNumber) {
						SysBeep(0);
						refuse = true;
						loop = false;
					}
				}
				
				//Trouve une place
				for(num = 0; num < maxNbInstruments; ++num)
					if(!instruments[num].fileID)
					break;
				if(num == maxNbInstruments)
				refuse = true;
				/*if(isCMusFile) {
					refuse = true;
					continue;
				}*/
				
				if(refuse)
				continue;
					
				//Appelle le Plug-In
				GetIndString(theString, 0, 23);
				theError = FindSymbol(importPlugIns[i].connID, theString, (char**) &theProcPtr, &theClass);
				if(theError) {
					SysBeep(0); //Do_Error(theError, 105);
					continue;
				}
				if(thePrefs.flags & kByPassHeaders)
				theError = (*theProcPtr)(&theFlavor.fileSpec, &instruments[num].snd, kNewHeader, &rate);
				else
				theError = (*theProcPtr)(&theFlavor.fileSpec, &instruments[num].snd, 0L, &rate);
				if(theError) {
					SysBeep(0); //Do_Error(theError, 106);
					continue;
				}
				HLock(instruments[num].snd);
				UseResFile(mainResFile);
				
				BlockMove(theFlavor.fileSpec.name, instruments[num].name, sizeof(Str63));
				
				instruments[num].fileID = -1;
				instruments[num].spec.vRefNum = theFlavor.fileSpec.vRefNum;
				instruments[num].spec.parID = theFlavor.fileSpec.parID;
				BlockMove(theFlavor.fileSpec.name, instruments[num].spec.name, 64);
				
				theCommand.cmd		= soundCmd;
				theCommand.param1	= 0;
				theCommand.param2	= (long) *instruments[num].snd;
				for(j = 0; j < thePrefs.nbInsChannels; ++j)
				SndDoImmediate(insChannels[j], &theCommand);
			
#if(kCreateFileIDs)
				theBlock.fidParam.ioNamePtr = theFlavor.fileSpec.name;
				theBlock.fidParam.ioVRefNum = theFlavor.fileSpec.vRefNum;
				theBlock.fidParam.ioSrcDirID = theFlavor.fileSpec.parID;
				PBCreateFileIDRef(&theBlock, false);
#endif
					
				tempCell.h = 0;
				tempCell.v = activeInstrument;
				LSetSelect(false, tempCell, instrumentsList);
				activeInstrument = num;
				tempCell.v = activeInstrument;
				LSetSelect(true, tempCell, instrumentsList);
				LSetCell(&instruments[activeInstrument].name[1], 
					instruments[activeInstrument].name[0], tempCell, instrumentsList);
				saved = false;
				if(musicFileSpec.parID && recorded)
				EnableItem(menu[1], 4);
				InvalRect(&insListRect);
			}
			
			if(theWindow == rythmsWin) {
				if(theFlavor.fileType == 'Patt')
				;//RB_LoadPatternSet(&theFlavor.fileSpec);
				else {
					//Trouve le Plug-In adéquat
					i = 0; j = 0; loop = true;
					while(loop) {
						for(j = 0; j < kTypesPerImport; ++j)
						if(importPlugIns[i].importFileTypes[j] == theFlavor.fileType)
						loop = false;
						
						if(loop)
						++i;
						if(i >= importPlugInsNumber) {
							SysBeep(0);
							refuse = true;
							loop = false;
						}
					}
					
					//Trouve une place
					for(num = 0; num < kPatternParts; ++num)
						if(!rythms[num].fileID)
						break;
					if(num == kPatternParts)
					refuse = true;
					/*if(isCMusFile) {
						refuse = true;
						continue;
					}*/
					
					if(refuse)
					continue;
					
					//Appelle le Plug-In
					GetIndString(theString, 0, 23);
					theError = FindSymbol(importPlugIns[i].connID, theString, (char**) &theProcPtr, &theClass);
					if(theError) {
						SysBeep(0); //Do_Error(theError, 105);
						continue;
					}
					if(thePrefs.flags & kByPassHeaders)
					theError = (*theProcPtr)(&theFlavor.fileSpec, &rythms[num].snd, kNewHeader, &rate);
					else
					theError = (*theProcPtr)(&theFlavor.fileSpec, &rythms[num].snd, 0L, &rate);
					if(theError) {
						SysBeep(0); //Do_Error(theError, 106);
						continue;
					}
					HLock(rythms[num].snd);
					UseResFile(mainResFile);
					
					//TextFont(3);
					TextSize(9);
					TextFace(0);
					BlockMove(theFlavor.fileSpec.name, rythms[num].name, sizeof(Str63));
					while(StringWidth(rythms[num].name) > kMaxWidthRyth)
					--rythms[num].name[0];
					//TextFont(3);
					//TextSize(10);
					//TextFace(1);
						
					rythms[num].fileID = -1;
					rythms[num].spec.vRefNum = theFlavor.fileSpec.vRefNum;
					rythms[num].spec.parID = theFlavor.fileSpec.parID;
					BlockMove(theFlavor.fileSpec.name, rythms[num].spec.name, 64);
					
					theCommand.cmd		= soundCmd; //Joue le son
					theCommand.param1	= 0;
					theCommand.param2	= (long) *rythms[num].snd;
					for(j = 0; j < thePrefs.nbRytChannels; ++j)
					SndDoImmediate(rytChannels[j], &theCommand);
					
#if(kCreateFileIDs)
					theBlock.fidParam.ioNamePtr = theFlavor.fileSpec.name;
					theBlock.fidParam.ioVRefNum = theFlavor.fileSpec.vRefNum;
					theBlock.fidParam.ioSrcDirID = theFlavor.fileSpec.parID;
					PBCreateFileIDRef(&theBlock, false);
#endif
					InvalRect(&rythmsRect);
				}
			}
			
        }
	}
	SetGWorld((CGrafPtr) oldPort, nil);
#if kA5savy
	oldA5 = SetA5(oldA5);
#endif    
	return refuse;
}

short DM_Init()
{  
	long			gestaltResponse;
	
	Gestalt(gestaltDragMgrAttr, &gestaltResponse);
	if(!(gestaltResponse & (1 << gestaltDragMgrPresent)))
	return false;

	TrackingHandlerRoutine = NewDragTrackingHandlerProc(TrackingHandler);
	ReceiveHandlerRoutine = NewDragReceiveHandlerProc(ReceiveHandler);
	
	theError = InstallTrackingHandler(TrackingHandlerRoutine, mainWin, (Ptr) SetCurrentA5());
	theError = InstallReceiveHandler(ReceiveHandlerRoutine, mainWin, (Ptr) SetCurrentA5());
	theError = InstallTrackingHandler(TrackingHandlerRoutine, instrumentsWin, (Ptr) SetCurrentA5());
	theError = InstallReceiveHandler(ReceiveHandlerRoutine, instrumentsWin, (Ptr) SetCurrentA5());
	theError = InstallTrackingHandler(TrackingHandlerRoutine, rythmsWin, (Ptr) SetCurrentA5());
	theError = InstallReceiveHandler(ReceiveHandlerRoutine, rythmsWin, (Ptr) SetCurrentA5());

    if(theError)
    Do_Error(theError, 809);
    dragEnabled = true;
    if(theError)
    return false;
    
    return true;
}

void DM_Dispose()
{
	if(!dragEnabled)
	return;

	RemoveTrackingHandler(TrackingHandlerRoutine, mainWin);
	RemoveReceiveHandler(ReceiveHandlerRoutine, mainWin);
	RemoveTrackingHandler(TrackingHandlerRoutine, instrumentsWin);
	RemoveReceiveHandler(ReceiveHandlerRoutine, instrumentsWin);
	RemoveTrackingHandler(TrackingHandlerRoutine, rythmsWin);
	RemoveReceiveHandler(ReceiveHandlerRoutine, rythmsWin);
}