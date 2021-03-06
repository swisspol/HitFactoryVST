#pragma	SC once

//FONCTIONS:

//Fichier Init.cp
void			Init_Toolbox();
void			Init_Menus();
void			Init_ImportPlugIns();
void			Init_DTDPlugIns();
void			Init_EditorPlugIns();
void			Init_LoadMenus();
void			Init_SamplerWindow();
void			Init_RecorderWindow();
void			Init_SynthetizerWindow();
void			Init_RythmsBoxWindow();
void			Init_Driver();
void			Init_GWorlds();
void			Initialization();

//Fichier Action.cp
short			Act_Sampler(Point);
short			Act_Recorder(Point);
short			Act_Synthetizer(Point);
short			Act_RythmsBox(Point);

//Fichier main.cp
void			Event_Loop();
void			Do_Menu(long);
void			Do_MouseDown(EventRecord);
void			End();
pascal OSErr	ODOC_Handler(AppleEvent*, AppleEvent*, long);
pascal OSErr	QUIT_Handler(AppleEvent*, AppleEvent*, long);

//Fichier Miscellaneous.cp
void			Show_AboutDialog();
void			Show_QuickHelp();
void			Show_Pub();
short			Register();
void			Do_Error(long, short);
void			Move_Left();
void			Move_Right();
void			Move_Max_Left();
void			Move_Max_Right();
void			Reset_Windows();
long			Shorts_To_Long(short, long);
short			AskForSaving();
short			AskForSoundsFolder();
short			Check_StartUpFile();

//Fichier Drawing.cp
void			Draw_Name(short);
void			Draw_Display(short, Str255);
void			Draw_Options(short);
void			Draw_Instrument_Options();
void			Draw_Number(short);
void			Draw_Volume(short);
void			Draw_Pitch(short);
void			Draw_Panoramic(short);
void			Draw_Instrument_Volume();
void			Draw_Rythms_Title();
void			Draw_Rythms_BPM();
void			Draw_Rythms_Current();
void			Draw_Rythms_Next();
void			Draw_Rythms_Options();
void			Draw_Rythms_Names();
void			Draw_Rythms_Mute();
void			Draw_Rythms_Pattern();
void			Draw_Rythms_Volume();
void			Update_Windows();

//Fichier Record.cp
void			Clear_Music();
short			Save_Music();
short			Write_Music();
short			Open_Music(FSSpec*);
void			Play_Music(long);
Boolean		Write_Files(short);
Boolean		Read_Files(short);
short			Install_Music();

//Fichier Compile.cp
short			Compile_Music();
short			Decompile_Music(FSSpec*);

//Fichier Rythms Box.cp
void			RB_ClearPattern(short);
void			RB_CopyPattern(short, short);
short			RB_EditPart(short);
short			RB_OpenPatternSet();
short			RB_LoadPatternSet(FSSpec*);
short			RB_SavePatternSet();
pascal void	RB_RythmeTaskFonction();
void			RB_Start();
void			RB_Stop();
void			RB_PlaySound(short);
short			RB_LoadSound(short);
short			RB_CloseSound(short);
short			RB_WritePatternSet(short);
short			RB_ReadPatternSet(short);

//Fichier Sound Files.cp
short			SF_OpenBank(FSSpecPtr, short);
short			SF_OpenSample(short);
short			SF_FastLoad(short, Str63, long);
short			SF_OpenDirectToDisk(short);
short			SF_OpenInstrument(short);
short			SF_LoadNextSample(short);
short			SF_LoadPrevSample(short);
short			SF_CloseSample(short);
short			SF_CloseInstrument(short);

//Fichier Preferences.cp
short			Create_Pref();
short			Read_Pref();
short			Update_Pref();
void			Init_PrefDialog();
void			Update_PrefDialog();
void			Act_PrefDialog(short);
void			Dispose_PrefDialog();

//Fichier Piano.cp
short			Create_Piano();
short			Read_Piano();
short			Update_Piano();
void			Init_PianoDialog();
void			Act_PianoDialog(short);
void			Update_PianoDialog();
void			Dispose_PianoDialog();

//Fichier Edit.cp
void			Init_EditDialog();
void			Update_EditDialog();
void			Act_EditDialog(short);

//Fichier Colors.cp
void			Init_ColorsDialog();
void			Act_ColorsDialog(short);
void			Update_ColorsDialog();
void			Dispose_ColorsDialog();

//Fichier Effects.cp
void			Init_EffectsDialog();
void			Act_EffectsDialog(short);
void			Update_EffectsDialog();
void			Dispose_EffectsDialog();

//Fichier Quantize.cp
void			Init_QuantizeDialog();
void			Act_QuantizeDialog(short);
void			Update_QuantizeDialog();
void			Dispose_QuantizeDialog();

//Fichier Sound Driver.cp
Fixed			SCD_CleanUpSound(Handle, Boolean);
void			SCD_StartSample(short);
void			SCD_StopSample(short);
void			SCD_StartDirectToDisk(short);
void			SCD_StopDirectToDisk(short);
void			SCD_AlignSample(short);
void			SCD_PitchSample(short);
void			SCD_VolumeSample(short);
void			SCD_EffectsSample(short);
void			SCD_VolumeSynthetizer();
void			SCD_VolumeRythmsBox();
void			SCD_SetSynthetizerMode(short);
void			SCD_CreateTrack(short);
void			SCD_KillTrack(short);
void			SCD_ReInitTrack(short, long);
void			SCD_PlayNote(short, unsigned char);
void			SCD_StopNote(short, unsigned char);
void			SCD_PauseResumeSampler();

//Fichier Utils.cp
void			SH_ForceUpdate(RgnHandle);
void			GetMBarRgn(RgnHandle);
void			ToggleMBarState();
void			Hide_Menus();
void			Show_Menus();
void			Anim_WaitCursor();
void			Wait(long);
void			OutLine_Item(DialogPtr, short);
void			Draw_DefaultItem(DialogPtr, short);
pascal short	DialogHook(short, DialogPtr, openData*);
pascal Boolean FileFilter(ParmBlkPtr, openData*);

//Fichier Time.cp
pascal void	TM_LoopTask();
void			TM_InstallLoopTask();
void			TM_KillLoopTask();
pascal void	TM_ReadTask();
void			TM_InstallReadTask();
void			TM_KillReadTask();

//Fichier MIDI Driver.cp
Boolean		MIDI_SendNote(char, char, char);
Boolean		MIDI_InstallDriver();
Boolean		MIDI_KillDriver();
void			MIDI_Suspend();
void			MIDI_Resume();
void			Init_MIDIDialog();
void			Update_MIDIDialog();
void			Act_MIDIDialog(short);
void			Dispose_MIDIDialog();