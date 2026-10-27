#pragma	SC once

//VARIABLES:

extern "C" {
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
					run,
					clockCount;
short				resID,
					whereClick,
					theKey,
					nbClicks,
					itemHit,
					dialogRunning,
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
Rect				dragRect = (**LMGetGrayRgn()).rgnBBox,
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
					activeInstrument,
					activeChan,
					activePart,
					activeRythmChan;
long				position,
					patternPos;
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

long				startTime,
					interTime,
					tempVol,
					oldOutputVol;

Boolean			recording,
					playing,
					recorded,
					isCMusFile,
					saved,
					samplerPaused,
					MIDIDriverEnabled,
					MBarShown,
					updatePattern;

FSSpec				musicFileSpec;

prefs				thePrefs,
					recordPrefs;

char				piano[pianoSize + maxNbInstruments + maxNbSamples + kPatternParts + pianoAdd],
					c[maxNbShowSamples],
					uc[maxNbSamples];
					
SndCommand			theCommand;
SCStatus			channelStatus;

short				prefVersion,
					pianoVersion,
					msicVersion,
					cmusVersion,
					pattVersion,
					curVersion;
					
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
SndChannelPtr		chanFinished;

short				editorPlugInsNumber,
					currentEditor;
editorPlugIn		editorPlugIns[kMaxEditorPlugsIns];

//VARIABLES QUANTIZE:

short				nbSamplesToStart;
short				samplesToStart[maxNbSamples];
short				nbSamplesToStop;
short				samplesToStop[maxNbSamples];
short				nbRBMutes;
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
}