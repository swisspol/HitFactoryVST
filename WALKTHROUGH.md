# Hit Factory VST: walkthrough of the original codebase

This document describes the original classic Mac OS sources in `Source/` and the files shipped on the original CD-ROM (repo root). Function names are given as `File.cp: Function` so they're easy to search for.

Contents:

1. [What the app is](#1-what-the-app-is)
2. [Repository layout](#2-repository-layout)
3. [Build configuration and platform](#3-build-configuration-and-platform)
4. [Global state and core data structures](#4-global-state-and-core-data-structures)
5. [Startup, event loop and shutdown](#5-startup-event-loop-and-shutdown)
6. [Sound engine](#6-sound-engine)
7. [The Sampler window](#7-the-sampler-window)
8. [The Synthesizer window](#8-the-synthesizer-window)
9. [The Rhythm Box window](#9-the-rhythm-box-window)
10. [Quantize ("help mixing system")](#10-quantize-help-mixing-system)
11. [The Recorder: recording, playback, editing](#11-the-recorder-recording-playback-editing)
12. [Keyboard control and the Piano map](#12-keyboard-control-and-the-piano-map)
13. [MIDI](#13-midi)
14. [Files and file formats](#14-files-and-file-formats)
15. [Export: compiled music and AIFF](#15-export-compiled-music-and-aiff)
16. [Plug-ins](#16-plug-ins)
17. [Menus, dialogs and UI chrome](#17-menus-dialogs-and-ui-chrome)
18. [Resources and assets](#18-resources-and-assets)
19. [Quirks and dead code](#19-quirks-and-dead-code)

---

## 1. What the app is

**Hit Factory VST 3.3** (resource `vers` 3.3.1, © Pierre-Olivier Latour 1996–97, distributed by Syclone Multimedia) is a live "techno studio" for PowerMac. In this app "VST" is part of the product name; it has nothing to do with Steinberg VST plug-ins. The app has four parts:

| Part | What it does |
|---|---|
| **Sampler** | 8–19 tracks (16 by default; see §4). Each track loops one sound, or a bank of sounds you can step through, or streams a file from disk ("Direct To Disk"). Each track has play/stop, volume, pitch (±~25 % playback rate, which changes pitch and speed together like tape), pan, auto-pan "loop", "turbo" (overdrive volume), mono/stereo/left/right routing, and optional reverb. |
| **Synthesizer** | Up to 20 instruments: single sounds that are repitched to play notes from the computer keyboard or MIDI. 1–10 voices of polyphony, a global volume, a turbo switch, and a mono/stereo-alternating voice mode. |
| **Rhythm Box** | A 16-step drum machine. 20 patterns, 10 parts (one sound per part, each with volume, pan, pitch and mute), BPM 20–240, a "current" and a "next" pattern, and 1–10 voices. |
| **Recorder** | Records every user action as a timestamped event (up to 3800 events, timed to ~1/96 s). Plays the events back with the UI animating ("paddles move, buttons light on"), and can hand the events to editor plug-ins for editing. |

On top of those, **Quantize** delays sample start/stop and drum-part mutes to the next pattern boundary, and can auto-adjust sample pitch so loops match the BPM. Songs can be saved, compiled into a self-contained file or standalone player app, or rendered to an AIFF file.

The intended use, from the `TEXT 128 "Pub"` resource, is live mixing and remixing of dance, techno and trance loops.

**The user manual** (`Manual/index.html`, converted from the English DOCMaker manual on the CD) is the user-side reference for this document. It covers every window, every preference, the hidden modifier-key shortcuts ("Extras" chapter) and the version history. Where the manual and the code disagree, this document follows the code and says so. Requirements per the manual: a PowerMac, System 7.1, Sound Manager 3.2 recommended (3.1 minimum), 1200 KB of memory plus the sounds, SoundSprocket Filter for reverb, MIDI Manager or OMS for MIDI.

## 2. Repository layout

The repo root holds:

- `Source/`: the original sources (table below), flat. Text files are **MacRoman** with **French** comments. Read them with `iconv -f MACROMAN -t UTF-8`.
- **The shipped install**, copied from the original CD-ROM:
  - `Hit Factory VST 3.3`: the app. Its fork has the same resources as `Source/Resources file.rsrc`, plus the compiled Balloon Help (`hmnu`, `hdlg`, `hwin`, `hrct`) and `kind`/`open`.
  - `Hit Factory VST 3.3 Manual`: the English DOCMaker manual. `Manual/index.html` is its HTML conversion.
  - `Plug-Ins/`: VST Interface (window artwork, §18), AIFF / System 7 Sound Import and Direct To Disk, Digital Editor.
  - The sound library: `Instruments/`, `Voices/`, `Drum Kits/`, `Samples/`, `Complex Samples/`, `Rhythms/`, `Complex Rhythms/`. These hold `'sfil'` sounds (one `snd ` each) and `'Bank'` files (several `snd `s).
  - `Finder Info.json`: the Finder type, creator and flags of every imported file.
- `Manual/`: the HTML manual.

Source files:

| File | Lines | Role |
|---|---|---|
| `main.cp` | ~2300 | Defines every global; `Warm_Up` (startup), `Event_Loop`, `Do_Menu`, `Do_MouseDown`, `End`; per-sample keyboard shortcuts (`Switch_Sample`, `Test_FonctionKey`, `Test_Numeric`); Apple Event handlers (`ODOC_Handler`, `QUIT_Handler`); Drag Manager (`DM_Init`, `TrackingHandler`, `ReceiveHandler`) |
| `Init.cp` | ~990 | Toolbox and menu setup, the four windows, GWorlds, sound channels (`Init_Driver`), plug-in loading, the "Fast load" sounds-folder menus (`Init_LoadMenus`) |
| `Actions.cp` | ~1900 | Mouse handling inside each window: `Act_Sampler`, `Act_Recorder`, `Act_Synthetizer`, `Act_RythmsBox`; button tracking helpers |
| `Drawings.cp` | ~440 | `Draw_*` functions that blit pieces of the interface PICT and draw text and numbers; `Update_Windows` |
| `Sound Driver.cp` | ~530 | Sound Manager wrapper (`SCD_*`): start/stop, pitch, volume/pan, notes, track init modes, pause, pitch alignment |
| `Sound Files.cp` | ~600 | Loading sounds (`SF_*`): sample or bank, fast load, Direct To Disk, instruments, next/previous sound in a bank, close |
| `Record.cp` | ~1160 | Songs: `Clear_Music`, `Save_Music`/`Write_Music`, `Open_Music`, `Write_Files`/`Read_Files` (sound file aliases), `Install_Music` (apply the song's snapshot), `Play_Music` (UI side of playback) |
| `Time.cp` | ~360 | Time Manager tasks: `TM_LoopTask` (auto-pan) and `TM_ReadTask` (event playback) |
| `Rythms Box.cp` | ~850 | Patterns, the sequencer tasks (`RB_RythmeTaskFonction`, `RB_QuantizeTaskFunction`), part editor dialog, pattern set files |
| `Quantize.cp` | ~160 | Quantize preferences dialog |
| `Compile.cp` | ~930 | Compiled music (`'CMus'`) and standalone player export; sound format conversion; `Decompile_Music` |
| `AIFF.cp` | ~1140 | AIFF export by inserting a custom Sound Manager output component (`NoiseMaker`) |
| `MIDI Driver.cp` | ~1160 | MIDI in/out through Apple MIDI Manager or OMS; MIDI clock sync; MIDI dialog |
| `Preferences.cp` | ~580 | Preferences file and General Preferences dialog |
| `Piano.cp` | ~480 | Keyboard map file and Piano Preferences dialog |
| `Colors.cp`, `Effects.cp`, `Edit.cp` | small | Colors dialog, Reverb (SoundSprocket) dialog, editor plug-in bridge |
| `Miscellaneous.cp` | ~880 | About, Quick Help, demo nag (`Show_Pub`), serial registration, `Do_Error`, sampler scrolling (`Move_*`), save prompt, sounds-folder chooser, startup file |
| `Utils.cp` | ~310 | Menu bar hide/show, wait cursor, dialog outline helpers, custom StandardFile hook and filter |
| `Constantes.h` | | Limits and flags (see §4) |
| `Structures.h` | | `sample`, `instrument`, `rythm`, `record`, `prefs`, plug-in structs |
| `Fonctions.h` | | Prototypes grouped by file |
| `Variables.h` | | `extern "C"` copy of the globals defined in `main.cp` |
| `Record Codes.h` | | Recorder event opcodes |
| `Fonctions Keys.h` | | Key codes for F1–F15 |
| `Codage HF 2.1.cp` | | **Not code**: notes on the old v2.1 file format |
| `Resources.r`, `* Balloons.r` | | Rez sources: Finder `kind`/`open` resources and Balloon Help |
| `Resources file.rsrc` (+ `.rsrc_fork`) | | Main resource file: UI pictures, dialogs, menus, strings, cursors, fonts, one sound |
| `HF VST Player - FAT.rsrc` | | The standalone player app, stored as two `'Plyr'` resources (its data fork and resource fork) |
| `PPC Only.rsrc` | | A small `CODE`/`ALRT` stub, probably the "requires PowerPC" alert shown on 68K |

Every file is stored as its data fork (often 0 bytes) plus a `<name>.rsrc_fork` sidecar file holding the resource fork, if it has one.

On the CD, the folders ` Instruments` and ` Voices` start with a space. That is **functional**: `Init_LoadMenus` leaves folders whose name starts with a space out of the Fast Load menu (§17). The repo copies have lost the space, so their names don't show this. Names were also copied through a FAT volume, which can't store some characters: `Voices/Oder Was_` was probably "Oder Was?" (its `snd ` resource is named that way).

## 3. Build configuration and platform

- **Compiler:** Symantec C++ (`#pragma SC once`, `#pragma noreturn`), C with a few C++ features (declarations mid-block, `extern "C"`).
- **Targets:** PowerPC, with 68K branches still in the code. PPC-only code sits behind `#if defined(powerc) || defined(__powerc)`, mainly the SoundSprocket "reverb" (3D localization filter).
- **Product flags** set by the old IDE project, not in the source:
  - `demo`: saving, compiling, AIFF export, MIDI and reverb are disabled (error `-30008` / string 904 "not available in the demo"). There's a nag screen and a click countdown (`nbClicks`, `maxNbClicks = 200`) that quits the app.
  - `pro`: AIFF export at 44.1 kHz 16-bit stereo instead of 22 kHz 16-bit mono, plus MIDI "slave" sync.
- **Required system:** Sound Manager 3.1 or later, checked at startup. Virtual memory triggers a warning because interrupt-time code can't handle page faults.
- **Interrupt-time code:** Time Manager tasks, MIDI read hooks and Drag Manager callbacks restore the app's A5 world (`kA5savy`) and then call into the Sound Manager directly. A lot of logic runs at interrupt time and shares globals with the main thread without any locking.

## 4. Global state and core data structures

### Limits (`Constantes.h`)

| Constant | Value | Meaning |
|---|---|---|
| `nbNotes` | 3800 | Maximum recorder events |
| `maxNbSamples` | 30 | Sampler tracks. The live count is `thePrefs.nbSamples`, default 16. The General Preferences dialog clamps it to **8–19** (`Preferences.cp`), although the arrays hold 30 and the manual says "8 to 30". |
| `minNbShowSamples` / `maxNbShowSamples` | 8 / 16 | Track columns visible at once (screen width / 80 px) |
| `maxNbInstruments` | 20 | Synth instruments |
| `maxNbInsChannels` | 10 | Synth voices (`nbInsChannels`, default 2) |
| `kPatternLength` / `kPatternParts` / `kNbPatterns` | 16 / 10 / 20 | Rhythm box grid |
| `kMaxRythmsChan` | 10 | Rhythm voices (`nbRytChannels`, default 2) |
| `defaultMaxVolume` | 256 | Unity volume (Sound Manager scale: 0x100 = 1.0) |
| `kMinRelativeRate` / `kMaxRelativeRate` | −28 / +28 | Pitch paddle steps |
| `baseKey` | 60 | Middle C; the note at which a sound plays at its recorded pitch |
| `pianoSize` | 120 | Notes in the keyboard map |
| `loopValue` | 10 | Auto-pan step size |
| `timeFactor` | 1.6 | `ActualTime = (TickCount() - startTime) * 1.6`, so one time unit is 1/96 s ≈ 10.4 ms |

Flag bytes:
- `thePrefs.flags`: `kByPassHeaders` (rewrite loop points and base note on load), `kNoInterruptBank` (don't swap bank sounds at interrupt time during playback), `kExtendedOpenDLG`, `kStopWhenKeyUp` (synth notes stop on key release), `kDTDTime` (show elapsed time for Direct To Disk tracks).
- `thePrefs.quantizeFlags`: `kQSmplStart`, `kQSmplPitchAlign`, `kQSmplStop`, `kQRB`.
- `thePrefs.MIDIFlags`: `kMIDIForeGround`, `kMIDIAutoConnect`, `kUseMidiMgr` (otherwise OMS), `kUseHFClock` (internal clock; when clear, follow MIDI clock).

### Structures (`Structures.h`)

```c
struct sample     { short fileID; short type; short sndNb; short totalSndNb; long time;
                    Handle snd; SndChannelPtr chan; Str63 name; FSSpec spec; };
struct instrument { short fileID; Handle snd; Str63 name; FSSpec spec; };
struct rythm      { short fileID; Handle snd; Str63 name; FSSpec spec; };
struct record     { long time; unsigned short action; long command; long command_2; };
```

- `sample.type` is `sampleType` (1) or `directToDiskType` (2).
- `sample.fileID` holds the open resource file refnum for a bank, `-1` for a single sound loaded by a plug-in, or `0` when the track is empty.
- `sndNb` / `totalSndNb` give the current position within a bank.
- `sndNb == DTDCode (-1)` in a saved song means "Direct To Disk". Otherwise it's 1-based: 1 is a single sound or the first sound of a bank (0 = empty).

`prefs` is one large struct that combines the user's preferences with **the full live mixer state**. It's saved to the prefs file and also embedded in every song:

- General: registration, user name, sounds folder `FSSpec`, `flags`, `quantizeFlags`, `loopDTD`.
- Window visibility, "extended" (expanded) state and position for each of the four windows.
- Configuration: `nbSamples`, `nbInsChannels`, `nbRytChannels`, `bufferSize` (Direct To Disk buffer), `outputVol`, `loopSpeed` (auto-pan Time Manager period).
- MIDI: enable flag, `MIDIFlags`, `MIDIDriverMode`, and the channels for sampler, synth and rhythm box.
- Sampler, one entry per track (arrays of 30):
  - `SSourceMode` (reverb mode)
  - `SStatus` (Sound Manager init: `initStereo`, `initMono`, `initChanLeft`, `initChanRight`)
  - `SGroup`
  - `SSndNb`
  - `SVol` (0..`SMaxVol`)
  - `SPan` (0..200, where 200 = left, 100 = centre, 0 = right)
  - `SRate` (the sound's native rate relative to 22 kHz, in `Fixed`)
  - `SRelativeRate` (pitch paddle position, −28..28)
  - `SLoop` (auto-pan speed and direction: 0 or ±`loopValue`)
  - `SPlaying`
  - `SMaxVol` (256, or `STurbo` when turbo is on)
- Plus `SReverb` and `STurbo`.
- Synth: `synthMode` (1 = all voices stereo, 2 = alternate voices left/right), `IReverb`, `ISourceMode`, `ITurbo`, `IVol`, `IMaxVol`.
- Rhythm box: `RReverb`, `RSourceMode`, `RTurbo`, `RPlaying`, `RBPM`, `RCurrent`, `RNext`, `RVol`, `RMaxVol`, and per-part `RPartVol` (0..200, 100 = unity), `RPartPan`, `RPartPitch` (MIDI note, 60 = native pitch), `RPartMute`.
- Groups: `enableGroups` and `G*`. These are declared but **never used anywhere**.
- Colors: `sampleColor`, `DTDColor`, `displayColor`.
- Effects: `SSpLocalizationData localization`, which is 196 raw bytes on 68K.

### Two copies of the mixer state

- `thePrefs` is the live state.
- `recordPrefs` is a snapshot taken when recording starts ("Freeze settings", ⌘F, takes one manually). It's saved with a song, and `Install_Music` restores it before playback ("Restore settings", ⌘R).

### Other important globals (defined in `main.cp`)

- `samples[30]`, `instruments[20]`, `rythms[10]`.
- `insChannels[10]` and `rytChannels[10]` are the voice pools. `currentNotes[10][2]` records which (instrument, note) each synth voice is playing.
- `pattern[20][10][16]` holds the rhythm grid (`255` = hit, `0` = off). `patternName` is the set's name.
- `tempRec[3800]` is the song (events). `position` is the record or playback cursor.
- `recording`, `playing`, `recorded`, `saved` and `isCMusFile` are the song state flags.
- `c[]` / `uc[]` map visible column ↔ track index for the scrolling sampler. `uc[track] == -1` means the track isn't on screen.
- `samplesToStart[]`, `samplesToStop[]` and `RBMutes[]` are the quantize queues.
- `piano[pianoSize + 20 + 30 + 10 + 2]` is the keyboard map (§12).
- `interTime` is the rhythm step period in negative µs, as Time Manager expects: `-(15000000 / BPM)`, one 16th note.
- `patternPos` is the current step (−1 before the first).
- `posArea[kCycleSize]`, `readPos` and `writePos` form a ring buffer from the playback interrupt to the main loop.
- `chanFinished` is set by the Direct To Disk plug-in callback when a stream ends.
- `interruptError` is an error raised at interrupt time and reported later by the main loop.

## 5. Startup, event loop and shutdown

`main()` runs `Warm_Up()`, then `Check_StartUpFile()`, then `Event_Loop()`, then `End()`.

**`main.cp: Warm_Up`**

1. `Init_Toolbox`. Create UPPs for the Time Manager tasks, Apple Event handlers and StandardFile hooks. Install `odoc` / `quit` handlers.
2. Work out how many sampler columns fit on screen (`screen width / 80`, clamped to 8–16). `Init_Menus`. Load cursors, prefs and piano file names, and the background pattern.
3. Check for Sound Manager 3.1 or later (otherwise error 405 and quit). Warn if virtual memory is on.
4. Load `snd 128` "No sound – 16Bits" (22 KB of silence) as `noSound`. Stopping a sample means playing this buffer (`SCD_StopSample`).
5. `Read_Pref` or `Create_Pref`. Demo builds show `Show_Pub`; registered builds require `Register()` the first time.
6. Show the splash dialog 1000 with status strings `STR# 800` ("Loading interface…", "Loading Plug-Ins…", "Scanning sound folders…", "Initialization…").
7. `Read_Piano` or `Create_Piano`, then `Initialization()` (window rects and other state), `Init_GWorlds` (offscreen copy of the interface PICT), `Clear_Music`, clear all patterns.
8. `Init_Driver`: allocate a Sound Manager channel for each sampler track (`SCD_CreateTrack`), the synth voices and the rhythm voices, and set volumes.
9. Load plug-ins (§16), then `Init_LoadMenus` (not in demo builds), then `MIDI_InstallDriver` if enabled.
10. Create the Rhythm Box, Synth, Recorder and Sampler windows. Start the auto-pan task `TM_InstallLoopTask`. Snapshot `thePrefs` into `recordPrefs`. `DM_Init` (drag and drop).

**`main.cp: Event_Loop`** is a `WaitNextEvent` loop with a sleep of 0, so it spins. On each pass it handles:

- `mouseDown`: `Do_MouseDown` routes to the menu bar, window drag, or the window's `Act_*` handler.
- `keyDown` / `autoKey`:
  - If a modeless dialog is open (`dialogRunning` 1–7), send the key to it.
  - Otherwise try F1–F15 (`Test_FonctionKey`, which toggles samples 1–15), then the numeric keypad (`Test_Numeric`, which sets the next pattern), then ⌘-keys (menus).
  - Otherwise ← / → scroll the sampler (with ⌥ to jump to the end), ↑ / ↓ change the active instrument, and anything else goes through the **piano map** (§12).
- `keyUp`: only used when `kStopWhenKeyUp` is set, to stop synth notes.
- `osEvt` suspend/resume: dim the scroll bar and suspend MIDI.
- `updateEvt`: redraw windows and the open dialog.
- `kHighLevelEvent`: Apple Events.
- Every pass, outside the event switch:
  - If recording or playing, **draw the time code** in the Recorder window.
  - If playing, empty the `posArea` ring buffer by calling `Play_Music(pos)` for each event the interrupt task has played, which animates the UI.
  - If `updatePattern` is set (a new pattern started), redraw the rhythm grid, play buttons and mutes.
  - If `chanFinished` is set (a Direct To Disk stream ended), either restart it (`loopDTD`) or mark it stopped.
  - Draw the elapsed time on Direct To Disk tracks.
  - Report any pending `interruptError`.

**`main.cp: End`** stops tasks, disposes channels, restores the output volume, saves prefs (`Update_Pref`), closes plug-ins and MIDI, and calls `ExitToShell`.

## 6. Sound engine

Everything runs on the classic **Sound Manager**: one `SndChannel` for each sampler track, synth voice and rhythm voice, all using the `sampledSynth`. That's up to 50 channels, mixed by Sound Manager 3.x. Sounds are `'snd '` resources held in memory as handles.

**Preparing a sound: `Sound Driver.cp: SCD_CleanUpSound(handle, forSampler)`**

1. Strip the `snd` resource header so the handle starts at the `SoundHeader` (standard or extended).
2. If `kByPassHeaders` is set, rewrite the header. Sampler sounds loop the whole sound (`loopStart = 0`, `loopEnd = numFrames`). Synth and drum sounds don't loop (`loopEnd = 0`). `baseFrequency` is set to 60.
3. Return the sound's sample rate divided by 22 254.5 Hz (`rate22khz`) as a `Fixed`. This becomes `thePrefs.SRate[n]`, so a 44 kHz sound gets ≈2.0.

**Sampler voice control**

| Function | What it does |
|---|---|
| `SCD_StartSample(n)` | `soundCmd` (install the sound), `freqCmd 1` (start looping playback), then `rateCmd SRate + SRelativeRate * 600`. One paddle step is 600/65536 ≈ 0.92 % of the 22 kHz rate, so ±28 steps is about ±25 %. Sets `SPlaying`. |
| `SCD_StopSample(n)` | `bufferCmd noSound`: plays silence instead of using `quietCmd`. Clears `SPlaying`. |
| `SCD_PitchSample(n)` | Sends `rateCmd` again with the new value. |
| `SCD_VolumeSample(n)` | Combines volume and pan into one `volumeCmd` with separate left and right levels. For `SPan > 100` (left), right = `SVol * (200 - SPan) / 100` and left = `SVol`. Otherwise left = `SVol * SPan / 100` and right = `SVol`. This is a linear balance law where the centre (100) is full volume on both sides. Volume above 256 means gain above unity (turbo). |
| `SCD_ReInitTrack(n, mode)` | `reInitCmd` with `initStereo`, `initMono`, `initChanLeft` or `initChanRight`. |
| `SCD_AlignSample(n)` | **Tempo fitting** (see below). |
| `SCD_EffectsSample(n)` | Sets the SoundSprocket localization mode (`kSSpSourceMode_Unfiltered` or `_Ambient`) on the channel. "Reverb" is really SoundSprocket's room simulation. Its parameters (wall distance, attenuation, reverb dB, source distance) are set in the Reverb dialog (`Effects.cp`). |
| `SCD_PauseResumeSampler` | Pauses every playing sample by sending `rateCmd 0`, which freezes the playhead where it is. Resume sends the normal rate again. Direct To Disk tracks go through the plug-in's Pause/Resume entry points. |

**Tempo fitting: `SCD_AlignSample`.** A pattern lasts `240000 / BPM` ms (16 sixteenth notes, which is 4 beats). Given the sound's length L in ms:

1. If L is under half a pattern, or already a whole number of patterns, do nothing.
2. Otherwise, if the remainder is less than half a pattern, speed the sound up so it fits in fewer patterns. If not, slow it down to fill more.
3. The offset is spread over `times` repetitions until the required `SRelativeRate` fits inside ±28.

"Align samples pitchs" (⌘A) runs this on every track. It also runs automatically when a sound loads, when BPM changes, and on MIDI clock (`AlignSample2`) if `kQSmplPitchAlign` is set.

**Auto-pan: `Time.cp: TM_LoopTask`.** Every `loopSpeed` ms (menu "Set loop speed" 1–9 sets `loopSpeed = (1000 − item × 100) / 4`, about 225–25 ms), each playing track with `SLoop ≠ 0` moves `SPan` by `SLoop` and bounces between 0 and 200.

**Synth voices.**
- `SCD_PlayNote(instr, note)` takes the next voice round-robin (`activeChan`), cutting off whatever it was playing (`quietCmd`). It then sends `soundCmd` with the instrument and `freqCmd note`. The Sound Manager repitches the sound relative to `baseFrequency 60`.
- `SCD_StopNote` finds voices playing that (instr, note) and silences them.
- `SCD_SetSynthetizerMode` reinitialises voices as all stereo, or as alternating left and right.
- Synth volume (`IVol`) is applied to every voice.

**Rhythm voices.** These also round-robin (`activeRythmChan`). Each hit sets volume and pan from `RVol * RPartVol / 200` and `RPartPan`, then `soundCmd` and `freqCmd RPartPitch` (§9).

**Direct To Disk** tracks stream a file through a DTD plug-in (§16). The plug-in owns a double buffer (`bufferSize`, default 250 000 bytes) and refills it from its own callback. When the stream ends, the plug-in sets `chanFinished`.

**In summary**, the Sound Manager features in use are resampling by rate with continuous rate changes during playback, loop points, per-channel stereo gain, voices that cut each other off, and pause/resume. "Turbo" is gain above 1.0, clipped on output.

## 7. The Sampler window

Window `WIND 1000`. It's 80 px per track column, as many columns as fit (8–16), and a horizontal scroll bar (`CNTL 1000`) when `nbSamples` is larger than the visible count. All drawing blits from the offscreen copy of the interface PICT.

Rects for each column (`Actions.cp: Act_Sampler`, `Drawings.cp`):

| Area | Interaction |
|---|---|
| **Name button** (top) | Click opens the pop-up `MENU 2000`: Load sample or bank… / Load Direct To Disk… / Close file / Fast load ▸ (submenus listing the sounds folder) / Stereo, Mono, Left only, Right only. ⌥-click loads a sample directly; ⌃-click loads Direct To Disk directly. The name is drawn in `sampleColor` or `DTDColor`. |
| **Options** strip | Several small toggles: two reverb lights, **R1** (`kSSpSourceMode_Localized`) and **R2** (`kSSpSourceMode_Ambient`), each toggling `SSourceMode` between itself and unfiltered (only if `SReverb`); **loop** (auto-pan, `SLoop`, not available on Left/Right tracks); **turbo** (`SMaxVol` toggles between 256 and `STurbo`, default 512, with `SVol` scaled to match); **next** / **previous** sound in a bank (`SF_LoadNextSample` / `SF_LoadPrevSample`). |
| **Play** button | Toggles start/stop. With quantize on (and no ⌥), it queues the track in `samplesToStart` or `samplesToStop` and draws a "pending" state. |
| **Volume** paddle | Vertical drag, 84 px (`volPaddleStart` 102 to `volPaddleEnd` 186), maps to `SVol` 0..`SMaxVol`. Shows "Volume: n" in the track display while dragging. |
| **Pitch** paddle | Horizontal drag over ±28 steps (`SRelativeRate`). ⌥ while clicking centres it (0). The manual says ±20 %; its editor chapter says −32..32, but the code clamps to ±28. |
| **Pan** paddle | Horizontal drag, `SPan` 200..0. ⌥ while clicking centres it (100). Only in Stereo or Mono mode. |
| **Display** | The track number, plus text feedback while dragging and the elapsed time for Direct To Disk tracks. The name is drawn in `sampleColor` (yellow by default) for sounds in memory and `DTDColor` (red) for Direct To Disk. |

Each change also writes a recorder event when `recording` is set (§11).

**Keyboard shortcuts for a track (`main.cp: Switch_Sample`)**, used by F-keys, the piano map and MIDI:
- Plain: toggle play (with quantize).
- ⇧: volume up 6 steps. ⇧ + Caps Lock: next sound in bank.
- ⌃: volume down. ⌃ + Caps Lock: previous sound in bank.
- ⌥: mute (volume to 0).

Scrolling (`Miscellaneous.cp: Move_Left/Right/Max_*`) shifts the `c[]` / `uc[]` mapping and redraws.

## 8. The Synthesizer window

Window `WIND 3000`:

- **Instrument list**: a List Manager list of 20 cells, "-" when empty. Double-click loads a sound as an instrument (`SF_OpenInstrument`). Single click, or ↑ / ↓, picks the **active instrument**, the one the piano keys play. "Close active instrument" is in the Utilities menu.
- **Volume** paddle (64 px), sets `IVol`.
- **Options**:
  - Mono/stereo voice mode (`synthMode`, recorded as `kMSynthMode`).
  - **Turbo** (`IMaxVol` toggles between 256 and `ITurbo`).

Note-on comes from the piano map, MIDI, or playback of `kIPlay`. The 120 piano keys cover note numbers 0–119 (10 octaves C0…B9, Apple's convention where 60 = C5, as the manual says). There are also 20 "direct instrument" keys that play a specific instrument at note 60 whatever instrument is active.

## 9. The Rhythm Box window

Window `WIND 6000`. It has a compact mode and an extended mode (`rytWinExt`).

| Area | Interaction |
|---|---|
| **Load** button | Shows the pattern set name. Clicking opens `MENU 5000`: Open pattern set… / Save pattern set… / Copy pattern to next one / Clear pattern. |
| **Play / Stop** | `RB_Start` / `RB_Stop`. Also bound to a piano key (default Enter). Starting again resumes at the step where it stopped, unless *current* was changed. ⌥-click on Stop makes the next start begin the pattern from step 0 (useful with MIDI sync). |
| **Current** / **Next** pattern | Pop-up `MENU 6000` (0–19). Typing two digits on the numeric keypad sets *next*. When a pattern ends, `RCurrent = RNext`. |
| **Options** | BPM +/− (±1, or ±10 with a modifier; 20–240; only when using the internal clock). Changing BPM re-aligns sample pitch when quantize pitch-align is on. Also **Turbo**. |
| **Part names** (10 rows) | **While the rhythm box runs** (or a song records or plays), clicking a name is a live hit. It plays the part (`RB_PlaySound`, or a MIDI note if no sound is loaded). If not recording or playing, it also **writes a hit into the pattern at the current step**, which is live step recording. When recording it logs `kRInstrument`. **While stopped**: ⌥-click loads a sound (`RB_LoadSound`), ⌃-click auditions the part, and a plain click opens the **Edit Part** dialog `DLOG 7000` (`RB_EditPart`: load, listen, close, volume 0–200, pan 200–0, pitch 0–127). Parts with no sound and MIDI enabled show "MIDI *note*". |
| **Mute** column | Toggles `RPartMute`, quantized when `kQRB` is set. Also on piano keys. |
| **Pattern grid** | 10 × 16 cells of 10 px. Click toggles a step on the current pattern. Not allowed while recording. Pattern edits aren't recorded. |
| **Volume** paddle | Sets `RVol`. |

**Sequencer: `Rythms Box.cp: RB_RythmeTaskFonction`.** This is a Time Manager task re-primed every `interTime` (one 16th note). At each step:

1. `++patternPos`. At 16, wrap to 0, set `RCurrent = RNext`, set `updatePattern` for the UI, and fire the **quantize task** immediately.
2. For each of the 10 parts with a hit at this step that isn't muted:
   - If the part has a sound loaded: take the next rhythm voice, `quietCmd`, set volume and pan, `soundCmd`, and `freqCmd RPartPitch`.
   - Otherwise, if MIDI is enabled: send note-off for the previous part's note, then note-on on **channel 10** (index 9) with note = `RPartPitch` and velocity = `RVol * RPartVol / 810`. The rhythm box can drive an external drum machine this way.

When following MIDI clock (`kUseHFClock` clear), `MIDI Driver.cp: RB_Fonction` (a copy of the same logic) runs on every 6th MIDI clock tick (24 ppqn / 6 = one 16th). The tempo measured between ticks feeds `AlignSample2`.

**Pattern sets** (`'Patt'` files): §14.

## 10. Quantize ("help mixing system")

Settings are in the Quantize dialog (`DLOG 3800`, `Quantize.cp`), stored in `quantizeFlags`:

- **Sampler start / stop** (`kQSmplStart`, `kQSmplStop`): a start or stop request goes into `samplesToStart[]` or `samplesToStop[]` instead of happening immediately. Holding ⌥ bypasses this. Direct To Disk tracks are never quantized.
- **Pitch align** (`kQSmplPitchAlign`): run `SCD_AlignSample` on load and on BPM change.
- **Rhythm box mutes** (`kQRB`): mute toggles go into `RBMutes[]`.

`Rythms Box.cp: RB_QuantizeTaskFunction` runs when each pattern starts (step 0). It applies all queued starts, stops and mutes together, records them, and clears the queues. **This only works while the rhythm box is running.** The queues are flushed only at pattern boundaries, so with the rhythm box stopped, queued actions wait.

## 11. The Recorder: recording, playback, editing

Window `WIND 2000`: a time-code display and Record / Edit / Stop / Play buttons (`Actions.cp: Act_Recorder`).

**Record.**
1. Clears `tempRec`, sets `position = -1`, snapshots `thePrefs` into `recordPrefs` (including each track's current bank index `SSndNb`), and resets `startTime = TickCount()`.
2. Disables menus that would change the setup: New, Open, Save, close instrument, and so on.
3. From then on **every action site** appends an event:

```c
if(recording) {
    ++position;
    tempRec[position].time = ActualTime;   // 1/96 s units
    tempRec[position].action = kSVolume;   // from Record Codes.h
    tempRec[position].command = track;
    tempRec[position].command_2 = value;
}
```

This block is copy-pasted throughout `main.cp`, `Actions.cp`, `Rythms Box.cp` and `MIDI Driver.cp`.

**Event opcodes (`Record Codes.h`).** `command` / `command_2` mean:

| Code | Value | Args |
|---|---|---|
| `kSStart` / `kSStop` | 10 / 11 | track |
| `kSNext` / `kSPrev` | 15 / 16 | track (step through bank) |
| `kSVolume` | 20 | track, volume |
| `kSPitch` | 21 | track, relative rate |
| `kSPano` | 25 | track, pan |
| `kSLoop` | 26 | track (toggle auto-pan) |
| `kSTurbo` | 27 | track (toggle) |
| `kSEffects` | 28 | track, source mode |
| `kSInit` | 29 | track, init mode |
| `kIPlay` / `kIStop` | 30 / 33 | instrument, note |
| `kIVolume` | 31 | volume |
| `kITurbo` | 32 | (toggle) |
| `kRPlay` / `kRStop` | 40 / 41 | |
| `kRVolume` | 42 | volume |
| `kRBPM` | 43 | BPM |
| `kRInstrument` | 44 | part (audition hit) |
| `kRTurbo` | 45 | (toggle) |
| `kRMute` | 46 | part (toggle) |
| `kRNext` / `kRCurrent` | 47 / 48 | pattern |
| `kMPausRes` | 80 | (pause/resume sampler) |
| `kMLoopSpeed` | 81 | menu item 1–9 |
| `kMSynthMode` | 82 | mode |
| `kEnd` | 90 | end of song |

Pattern *edits* are **not** recorded. A `kRPattern` case exists in playback but is commented out. Patterns are saved as a whole with the song.

**Stop** appends `kEnd` when recording. When playing, it kills the read task and silences everything.

**Play** (`Record.cp: Install_Music`, then `Time.cp: TM_InstallReadTask`):
1. `Install_Music` resets the mixer to `recordPrefs`: reload each bank's sound at the saved index, restore init mode, volume, pan, pitch, loop, turbo and effects, and start the tracks that were playing when recording began. It also resets the synth and the rhythm box (BPM, current/next pattern, volumes, part settings).
2. `TM_ReadTask` (Time Manager, interrupt time) applies `tempRec[position]` to the sound engine. It pushes `position` into the `posArea` ring buffer, then re-primes itself for `(next.time − this.time) × 10.4` ms.
3. On `kEnd` it stops everything.
4. The main loop drains the ring buffer and calls `Play_Music(pos)`, which **animates the UI** for that event: moving paddles, lighting buttons, redrawing names. For Direct To Disk starts it actually starts the stream, because that can't happen at interrupt time.

So the audio side of playback runs at interrupt time and the visual side runs on the main thread.

**Edit** opens the current **editor plug-in** (Editors menu) on the event list. The host passes `tempRec` to `EditorPlugIn_Init` and then forwards events to it (`Edit.cp`). The shipped editor is `Plug-Ins/Digital Editor` (PowerPC code, reference only). It shows the events as a list of time code (displayed as 1/100 s), action code and two parameters, with Insert / Delete and an action pop-up (`MENU 4000`, which includes `33 - Stop note`). The manual's "The editor" chapter documents each action's parameters. It requires events sorted by time, no two with the same time code, and `kEnd` last.

**Freeze / Restore settings** (Utilities ⌘F / ⌘R) copy `thePrefs` into `recordPrefs`, or apply `recordPrefs` through `Install_Music`, outside recording.

## 12. Keyboard control and the Piano map

The `piano[]` char array maps keyboard characters to actions. It's stored in the "Hit Factory VST Piano Prefs" file (type `'Data'`) in the Preferences folder and edited in `DLOG 8000` (`Piano.cp`). Index ranges:

| Indices | Count | Action |
|---|---|---|
| 0–119 | 120 | Synth notes C0–B9 on the active instrument |
| 120–139 | 20 | Play instrument *n* at note 60 |
| 140–169 | 30 | Toggle sampler track *n* (`Switch_Sample`) |
| 170–179 | 10 | Toggle rhythm part mute *n* |
| 180 | 1 | Pause/resume sampler |
| 181 | 1 | Start/stop rhythm box |

Defaults (`Create_Piano`) assume a **French AZERTY** keyboard:
- Notes 55–65 (G4…F5) on the home row `q s d f g h j k l m ù`.
- Part mutes on the top row `& é " ' ( § è ! ç à`.
- Pause on space; rhythm box start/stop on Enter (char 3).
- Sampler tracks have no default keys. F1–F15 are hard-wired to tracks 1–15 (the code calls `Switch_Sample(1..15)`, so F1 is track **index 1**, the second track; see §19).

The numeric keypad (key codes in `Test_Numeric`) types the next pattern number in two digits.

## 13. MIDI

`MIDI Driver.cp` supports **Apple MIDI Manager** (`AMM_*`, ports `'Inpt'` / `'Oupt'`, auto-connect to the modem port) or **Opcode OMS** (`OMS_*`). The settings are in `DLOG 3500`.

**Input: `MIDI_ReadHook`**, at interrupt time. It's ignored in the background when `kMIDIForeGround` is set.

- **Realtime** (when following external clock): Clock (every 6th tick runs one rhythm step and measures tempo), Start (reset to step −1 and start), Continue, Stop. These are also recorded as `kRPlay` / `kRStop`.
- **Note-on routing** by `MIDIDriverMode`:
  - `kMIDIAllToSmpl` (0): `note mod nbSamples` picks the track. Toggles start/stop (with quantize). Pitch stays as it is.
  - `kMIDIAllToSmplChan` (1): the MIDI channel picks the track (1–16). `note − 60` sets `SRelativeRate`, clamped to ±28. The same note again stops the track; a different note changes pitch.
  - `kMIDIAllToSynth` (5) / `kMIDIAllToSynthChan` (6): play the active instrument, or the instrument selected by channel, at the received note. Note-off stops it.
  - `kMIDISmplAndSynth` (10): separate channels for sampler, synth and rhythm box (`MIDISamplerChannel`, `MIDISynthChannel`, `MIDIRBChannel`).
  - `kMIDIAllToRBox` (20): notes trigger rhythm parts.
  - All of these are recorded like the matching UI actions.

**Output:** `MIDI_SendNote(channel, note, velocity)`. It's used by the rhythm box for parts with no sound loaded (§9).

## 14. Files and file formats

The creator code for all of these is `'HiFc'`. Finder kinds come from `Resources.r`.

| Type | Kind | Written by | Notes |
|---|---|---|---|
| `'Msic'` | Hit Factory music | `Record.cp: Write_Music` | Score plus mixer snapshot plus references to sound files |
| `'CMus'` | Compiled music | `Compile.cp: Compile_Music` | Score plus snapshot with **sounds embedded** as resources |
| `'APPL'` / `'HFPl'` | Standalone player | `Compile_Music` with "stand-alone" ticked | Player app plus compiled music |
| `'Bank'` | Bank | (made by the separate "Bank Factory" app) | Resource file holding several `'snd '` resources (empty data fork, sometimes a `vers`). Resource order is bank order; the shipped banks name their sounds `<Name>-1`, `<Name>-2`… |
| `'Patt'` | Pattern set | `RB_SavePatternSet` | Patterns plus rhythm settings plus drum sound references |
| `'Data'` | Prefs / Piano prefs | `Preferences.cp`, `Piano.cp` | In the System Preferences folder |
| `'sfil'`, `'AIFF'` | Sounds | (input) | Read through import plug-ins |
| `'ImPI'`, `'DDPI'`, `'EdPI'`, `'ItPI'` | Plug-ins | | §16 |

**`'Msic'` data fork** (all big-endian, raw struct dumps with **PowerPC alignment**; verified against the demo files below):

```
short   msicVersion           (312)
short   numberNotes           (always nbNotes = 3800)
record  tempRec[3800]         (16 bytes each: long time, ushort action, 2 pad, long command, long command_2)
short   prefVersion           (321, or 314 in older files: same layout; Open_Music doesn't check it)
prefs   recordPrefs           (1528 bytes; Toolbox structs FSSpec/Point/RGBColor keep 2-byte alignment)
-- Write_Files:
30 × { long aliasSize; Alias data[aliasSize] }   sampler sounds (size 0 = empty)
20 × { long aliasSize; Alias data[aliasSize] }   synth instruments
-- RB_WritePatternSet:
short   pattVersion           (312)
uchar   pattern[20][10][16]
short   prefVersion
prefs   thePrefs              (yes, a second full prefs copy)
10 × { long aliasSize; Alias data[aliasSize] }   rhythm sounds
```

Each alias is a minimal Alias Manager record (`NewAliasMinimal`, version 2): volume name at +10 (Str27), parent directory ID at +46, file name at +50 (Str63), file type/creator at +122/+126. No folder names are stored.

**Saving bug:** "Save as" deletes and recreates the file, but "Save" (`Write_Music`) rewrites it from offset 0 without `SetEOF`. A shorter save leaves the old file's tail behind (two demo songs end with 154 and 1386 stale bytes). Readers must stop after the last block, not at end of file.

**Event order:** events are appended from both the main loop and interrupt tasks (quantize, MIDI), so times can repeat (48 pairs in one demo song) or go back by a few units. `TM_ReadTask` primes the next wait with `(next.time − this.time) × 10.4`; zero fires at once and a negative count means microseconds to the Time Manager, so such events also play immediately. The manual's editor rule ("sorted, no two with the same time code") isn't met by the app's own files.

Loading (`Open_Music`):
1. Check `msicVersion` exactly. Truncate at 3800 events, adding `kEnd`. `prefVersion` is read but not checked.
2. Resolve each alias. The code first looks for a file with the alias's name in the song's own folder, then calls `ResolveAlias` (the manual describes this as "the parent folder, then the whole volume"), so renamed or moved sounds are still found on the same volume. If a file can't be found, ask the user to locate it (`DLOG 10000`); ⌥ forces this for every file. A song named like one of its sound files fails with "Missing Import Plug-In" (per the manual).
3. Load each sound with the matching import plug-in or as a bank. Tracks saved as `SSndNb == -1` reopen as Direct To Disk.
4. Read the pattern set, then `Install_Music`.

Songs saved with more sampler tracks than the current `nbSamples` are rejected (string 402).

**`'Patt'` data fork:** the same block as `RB_WritePatternSet` above. It holds a full `prefs`, but `RB_ReadPatternSet` only takes BPM, current/next pattern, rhythm box volume and part vol/pan/pitch/mute from it, then loads the drum sounds (same name-in-folder-then-alias search). Pattern cells are 0 or 255.

**Prefs file:** `short prefVersion` followed by `prefs`.
**Piano file:** `short pianoVersion` followed by `piano[]`.

**Version constants** (`main.cp`): `prefVersion 321`, `pianoVersion 321`, `msicVersion 312`, `cmusVersion 322`, `pattVersion 312`, `curVersion 330`.

The old 2.1 format is described in `Codage HF 2.1.cp`: 30 "rythmes" tracks, 800 events with short args.

### Demo files

`Demo music Files/` (copied from the CD) holds 7 songs and 9 pattern sets, plus a readme saying the CD must be in the drive: every alias points to the volume "Hit Factory CD".

- **Songs:** "Happy HardCore Mix", "HardCore - Megamix", "Join the crowd", "Lost Genesis Remix - BEST ONE!", "Raise Ya Power", "This is", "What's that mix?" (FAT copy: `What's that mix_`). 78–1070 events each, 4½ to 9½ minutes, 16 tracks, BPM 140 or 160. All parse and validate.
- **Pattern sets:** Core, Disco, Foo, Happy, Progressive, Progressive II, Techno set, Techno set II, Trance. 2–9 patterns used, BPM 140 or 160.
- **Events used:** mostly sampler volume (2813 of 4043), rhythm box next pattern, volume, mute and BPM, sampler start/stop, synth notes. Two songs step through banks (`SSndNb` 10 on a `Jungle - BANK` track).
- **Prefs inside:** `recordPrefs` keeps everything, including the saver's registration code and user name, window positions, MIDI settings and colours. `SRate` values match `rate / 22 254.5 Hz` (64934 = 22 050 Hz, 129867 = 44 100 Hz).
- **Missing sounds:** 14 references to 13 distinct sounds aren't in the shipped library: "Techno Techno...", "I'm crazy...", "Emotions", "Maybe...", "Long Trance", "Fugees-2", "Experience Piano", "Dreams Bells", "Name", "Favorite" (`Samples`), "Break down!", "Hyper Track", "It's absolutely..." (`Voices`) and "Director" (`Complex Rhythms`).
  - **The CD really lacks them**, which was checked against the original disk: every sound folder matches the import file for file, and no file or bank sound anywhere has these names.
  - **Their aliases point to the same folder IDs** as sounds that are found (`Samples` 599, `Voices` 324, `Complex Rhythms` 397, on "Hit Factory CD"). So the songs (June 1997) were made against a pre-release version of the CD's folders, and these sounds were dropped or renamed before the August 1997 master. Several look like commercial samples ("Fugees-2", "Emotions", "I'm crazy..."), which suggests a rights clean-up.
  - **One is probably renamed:** "Dreams Bells" is likely today's `Samples/Dream Bells`; the saved rate matches (22 050 Hz). The others have no plausible stand-in ("Long Trance" was 44.1 kHz; the near-named "Trance" sounds are 22 kHz).
  - **FAT renaming:** a few more sounds only differ by it ("Oh sacrilege." → `Oh sacrilege`).

## 15. Export: compiled music and AIFF

**Compiled music** (`Compile.cp: Compile_Music`, Export ▸ "Compiled music…", `DLOG 9000`):
1. Refuse songs that use banks (`kSNext` / `kSPrev`) or Direct To Disk (string 408).
2. The options dialog offers:
   - Convert every sound to 8 or 16 bit, 11/22/44 kHz, mono or stereo (`MENU 400/410/420`, needs Sound Manager 3.2).
   - "Stand-alone application" with player polyphony for sampler, synth and rhythm box, volume, and auto-play.
3. For a standalone player, create an `APPL` / `HFPl` file and copy in the player's data fork (`Plyr 128`) and resource fork (`Plyr 129`). Add a `'Pref' 128` "Player options" resource (`playerPrefs`: `smpPoly`, `sytPoly`, `rytPoly`, `volume`, flags `kAutoPlay` / `kAutoQuit` / `kLoop`).
4. Write a `'Msic' 128` "Music score" resource: version, note count, `tempRec`, a compression marker, prefs version, `recordPrefs`, and `pattern`.
5. Write the sounds as resources `'Smpl' n`, `'Syth' n` and `'RBox' n`, converted if requested (`Convert_Sound` uses `SoundConverter`).
6. `Decompile_Music` loads a CMus back in. It sets `isCMusFile`, after which saving as `'Msic'` is refused (string 409).

**AIFF export** (`AIFF.cp: Music2AIFF`, Export ▸ "AIFF sound file…", `DLOG 9100`):
1. Ask for file type and creator, a "listen while exporting" checkbox, and buffer size. Check free disk space and write an AIFF header (`SetupAIFFHeader`). The format is 44.1 kHz/16-bit/stereo for `pro`, otherwise 22 kHz/16-bit/mono.
2. **Register a custom Sound Manager output device component** (`NoiseMaker`, manufacturer `'Pol '`). Recreate every sound channel so it mixes into this component instead of the hardware. The component's `PlaySourceBuffer` / interrupt routine copies the mixed samples into two ping-pong buffers.
3. Play the song **in real time** (`Install_Music` then `TM_InstallReadTask`), optionally audible through a second channel. The main loop writes each filled buffer to the file and updates a progress bar (`CNTL 9110`, `CDEF 700`) until the song ends or the user clicks.
4. Restore the normal channels.

## 16. Plug-ins

Import, Direct To Disk and Editor plug-ins are Code Fragment Manager shared libraries in the "Plug-Ins" folder next to the app (`STR# 0` item 21). They're found by file type and called through `FindSymbol`, using the C++-mangled names in `STR# 0` items 22–37. The shipped plug-ins are in `Plug-Ins/`; the code ones are PowerPC fragments in their data forks (reference only).

| Kind | Type | Limit | Entry points | Purpose |
|---|---|---|---|---|
| Import | `'ImPI'` (`Resources.r`'s `kind` says `'IpPI'`, a typo) | 15 × 3 file types | `ImportPlugIn_Init(importPlugInInfos*)`, `ImportPlugIn_LoadFile(FSSpec*, Handle* snd, long flags, Fixed* rate)`, `ImportPlugIn_Quit()` | Convert a sound file into an in-memory `snd` (flags `kLoopSound`, `kNewHeader`) |
| Direct To Disk | `'DDPI'` | 15 × 3 file types | `DTDPlugIn_Init`, `_OpenFile(FSSpec*, long, Fixed*, DTDPlugInParameters*)`, `_Start`, `_Pause`, `_Resume`, `_Stop`, `_CloseFile`, `_Quit` | Stream a file into a channel through a double buffer |
| Editor | `'EdPI'` | 5 | `EditorPlugIn_Init(editorPlugInInfos*, short, short, record*)`, `_Show(Boolean update, GrafPort**)`, `_Act(short item, Point, short key, Boolean* finished)`, `_Update()`, `_Quit()` | Edit the recorded event list in their own dialog; listed in the Editors menu |
| Interface | `'ItPI'` | 1 | none (plain resource file) | **Required.** Holds the window artwork PICTs (see §18). Without it the app quits with "Can't find any Interface Plug-In". Shipped as `Plug-Ins/VST Interface`. |

Shipped in `Plug-Ins/` (type, creator `'HiFc'`):

| Plug-in | Type | Contents |
|---|---|---|
| VST Interface | `'ItPI'` | 19 PICTs, `vers`. Empty data fork. |
| AIFF Import, System 7 Sound Import | `'ImPI'` | PowerPC code in the data fork, `vers` |
| AIFF Direct To Disk, System 7 Sound Direct To Disk | `'DDPI'` | PowerPC code in the data fork, `vers` |
| Digital Editor | `'EdPI'` | PowerPC code, plus `DLOG`/`DITL`/`MENU 4000` (action list), banner PICTs 128/129, `PRMS`/`TMPL` |

Interface plug-ins are **skins**: the manual says they "change the look of the program", and only the first one found is loaded. `Screenshot.gif` shows a different, lighter skin (Platinum-style controls, blue gradient pitch strip, white-to-red pan strip, the "purple" volume paddle the manual's text mentions) on the same pixel layout. The shipped "VST Interface" is the dark skin introduced in 3.2.3 ("New VST (Virtual Studio) interface"), which is the one the manual's screenshots show.

Bank files have no import plug-in: the app reads them itself (`SF_*` in `Sound Files.cp`).

## 17. Menus, dialogs and UI chrome

Menus come from `MENU` resources; ⌘ shortcuts are in brackets:

- **Apple** (1000): About Hit Factory VST…
- **File** (1001): New music… [N] · Open music… [O] · Save music [S] · Save music as… · Export music ▸ (`MENU 100`: Compiled music… / AIFF sound file…) · Preferences ▸ (`MENU 120`: General… / Piano… / Colors… / MIDI… / Reverb… / Quantize…) · Quit [Q]
- **Utilities** (1002): Set loop speed ▸ (`MENU 110`: 1–9) · Pause Sampler [P] · Align samples pitchs [A] · Close active instrument · Freeze settings [F] · Restore settings [R] · Show background [B]
- **Windows** (1003): Recorder · Synthesizer · Rhythms box · Reset windows positions. Windows can't be toggled while recording or playing.
- **Editors** (1004): filled with the loaded editor plug-ins.
- **Help**: Balloon Help plus "Quick help" (`DLOG 5000`, `TEXT 100/200/300`).

"New music" asks to save, then resets everything: closes all sounds, sets BPM to 120, and restores default part settings.

"Show background" (`ToggleMBarState`) hides the menu bar and covers the desktop with a patterned window (`WIND 10000`, `ppat 1000`) for live performance.

**Dialogs** (`DLOG` id: title):
- 128 Pub (demo nag)
- 129 Serial Number
- 1000 About / splash
- 2000 Error
- 3000 General Preferences: max volume, sounds folder, track count, turbo values, Direct To Disk loop, polyphonies, header bypass, interrupt bank loading, Direct To Disk buffer
- 3500 MIDI
- 3600 Reverb
- 3700 Colors
- 3800 Quantize
- 5000 Quick Help
- 6000 Save changes?
- 7000 Edit Part
- 8000 Piano
- 9000 Export Compile
- 9100 / 9110 AIFF export and progress
- 10000 Select Lost File
- 20000–30100 custom open/save dialogs
- 21000 Select sounds folder

Preferences dialogs are **modeless** and are handled by the main loop through `dialogRunning`. The rest are modal.

**Errors:** `Miscellaneous.cp: Do_Error(OSErr, code)`. The OSErr is looked up in `STR` resources (`-30008`…`0`), and `code` = `STR#` id × 100 + index (for example `405` → `STR# 400` item 5, "You need at least the Sound Manager 3.1…").

**Balloon Help** text for every control is in `STR#` 1000–13000 and the `*.r` files. It's the best available description of what each control does.

**Fast load:** `Init_LoadMenus` scans up to 10 subfolders of the chosen sounds folder and builds submenus (`MENU 200+`) listing their files. It **skips folders whose name starts with a space** and the Plug-Ins folder; the manual's Extras chapter documents the space trick, and the CD uses it to hide ` Instruments` and ` Voices` (sounds meant for the synthesizer). Picking an item calls `SF_FastLoad`, which loads it without a dialog. Per the manual, it replaces whatever the track was playing and starts the new sound immediately.

**Startup file:** a song named "Startup" in the app's folder is loaded at launch (`Check_StartUpFile`).

**Drag and drop:** dropping sound or bank files on the Sampler window loads each one into the **first empty track**. The drop is refused if every track is full (`main.cp: ReceiveHandler`).

## 18. Resources and assets

> **The main window artwork is in the Interface Plug-In**, `Plug-Ins/VST Interface`. `Init.cp: Init_GWorlds` opens the **Interface Plug-In** (the first `'ItPI'` file in the Plug-Ins folder) as a resource file. It draws these PICTs into a 940×302, 8-bit offscreen GWorld, `picGWorld`, which every `Draw_*` / `Act_*` function blits from with hard-coded rects:
>
> | PICT | Placed at (l,t,r,b) | Content |
> |---|---|---|
> | 1000 / 1100 | 0,0,80,270 / 81,0,161,270 | Sampler track column, normal / pressed |
> | 2000 / 2100 | 162,0,450,150 / 162,151,450,301 | Synthesizer, normal / pressed |
> | 3000 / 3100 | 451,0,610,41 / 451,42,610,83 | Recorder, normal / pressed |
> | 4000 / 4100 | 611,0,931,150 / 611,151,931,301 | Rhythm Box, normal / pressed |
> | 100, 105, 110 | 451,100… | Green, dark-green play and red record buttons |
> | 120, 130, 140 | 533,100… | Red, green and yellow check lights |
> | 150 / 160 | 451,161… | Bank prev/next buttons, normal / pressed |
> | 200 / 210 | 451,120… / 472,120… | Big and little fader knobs |
> | 300 | 451,185… | Rhythm box quantize-mute indicator |
>
> Every pressed or lit state is a `CopyBits` from the "invert" copy (red R1/R2 lights, red record button, orange rhythm grid cells…). `PICT 130` (11×11, the green light) is drawn into an 11×21 rect, so it is stretched in the GWorld. The drawing code blits that 11×21 rect back into an 11×11 destination (`Draw_Options`), so on screen it looks like the original PICT.
>
> The interface PICTs only contain static art. Text (names, numbers, time codes) is drawn with `FOND 200` in the colours from the Colors preferences. The pitch and pan knobs are `PICT 210` and the volume knob is `PICT 200`, drawn at positions computed from the values.

`Source/Resources file.rsrc.rsrc_fork` holds:

- **PICT** (22):
  - `PICT 10000` (500×284): the "Hit Factory VST – the ultimate techno solution" splash, used in the About/splash dialog 1000.
  - `PICT 128`–`142` (220×50): title banners for the dialogs, e.g. "PREFERENCES / general, piano, colors, reverb, midi, quantize", "RHYTHMS BOX / edit part", "EXPORT MUSIC / compiled music, aiff sound file", "OPEN FILE / music, sound, direct to disk, pattern set", "SAVE FILE / music, pattern set".
  - `PICT 143`: the Syclone Multimedia logo. `PICT 144`: "Get it Now!" (demo nag).
  - `PICT 200` / `210`: small "Hit Factory VST" logos for the dialogs. These have the same IDs as the fader PICTs in the interface plug-in, but are different pictures.
  - `PICT 0` / `10`: 2×1 and 1×2 pixel spacers.
- **Icons:** `ICN#`, `icl8`, `ics#`, `ics8`, `ICON`, `cicn` (Finder and document icons, `BNDL`/`FREF`).
- **Cursors:** `crsr 1000` (mouse), `2000`–`2007` (animated wait cursor), `3000` (hand), `128`/`129`.
- **Font:** `FOND 200`, a bitmap font with 9, 10 and 12 pt strikes (`NFNT 12345`, `10628`, `7021`). It's used for the on-screen displays and names (`TextFont(200)`, mostly 9 pt).
- **Cursors:** `crsr 3000` (hand), which `Warm_Up` loads, isn't in the resource file.
- **Custom controls and windows:** `CDEF 0` "3D buttons", `CDEF 700` "Progress Bar", `CDEF 1000` "Balloon help button", `WDEF 0` "Pol's Windows PPC" (compiled code; the look has to be recreated).
- **Colour tables:** `dctb`, `wctb`, `mctb`, plus `ppat 1000`.
- **Text:** `TEXT` (quick help, the full product description "Pub", license, scrolling credits), `STR`, `STR#`, `MENU`, `DLOG`/`DITL`, `WIND`, `CNTL`.
- **Sound:** `snd 128` "No sound – 16Bits" (silence, used to stop a track).

`Source/HF VST Player - FAT.rsrc.rsrc_fork` holds `'Plyr' 128/129`, the player app's binary forks (not needed). `PPC Only.rsrc.rsrc_fork` is a 68K stub (not needed).

### The shipped sound library

The CD's seven sound folders hold 534 `'sfil'` files and 25 `'Bank'` files: 629 sounds in all, plus a text file (`Drum Kits/Abbréviations`, a French key to the drum-kit name abbreviations: Acs = acoustic, Bss = bass, Csd = closed…). The CD's songs and pattern sets are in a separate `Demo music Files` folder (§14).

| Folder | Content |
|---|---|
| `Instruments/` (CD: ` Instruments`) | Single hits and tones for the synthesizer. Hidden from Fast Load. |
| `Voices/` (CD: ` Voices`) | Vocal shots. Hidden from Fast Load. |
| `Drum Kits/` | Drum hits for the rhythm box |
| `Samples/`, `Complex Samples/` | Loops for the sampler |
| `Rhythms/`, `Complex Rhythms/` | Drum loops, many as Banks of variations (`Jungle - BANK`, `Speed - BANK`…) |

What the sounds' headers show:

- **Encoding:** every sound uses an extended sound header with **16-bit** samples. 620 are mono, 9 are stereo. None are compressed.
- **Sample rates:** mostly 22 050 Hz (375) and 44 100 Hz (223), but also 22 254.5, 22 326.5, 11 127, 16 726, 17 162, 17 446, 21 073 and 28 130 Hz. Playback must resample from any rate.
- **Lengths:** 0.06 s to 12.3 s, median 1.3 s.
- **Loop points:** only 5 sounds have a non-zero `loopEnd` (`Instruments/Rap`, `Instruments/Reaper`, `Voices/Excellent`, `Voices/Funkiness`, and `Complex Rhythms/Jungle`, which loops frames 150–27058). With "ByPass sound headers" on (the default), `SCD_CleanUpSound` ignores them anyway and loops the whole sound.
- **Base note:** 60 for 617 sounds. Twelve use 72, 76 or 77; header bypass resets these to 60 too.
- **Creators:** mostly SoundEdit-style `'√SFx'`, plus `'movr'`, `'AUDI'`, `'jB•x'`. The app doesn't care.

## 19. Quirks and dead code

- **F-key off-by-one:** `Test_FonctionKey` maps F1 to `Switch_Sample(1)`, which is the second track. Track 0 has no F-key. Probably a bug.
- **`Test_Numeric`** builds a two-digit pattern number from the previous *next* value, with odd wrap logic (`//!si plus de 19 patterns`).
- **Groups** (`SGroup`, `enableGroups`, `G*`) are declared and reset but never used.
- **`kRPattern`** (recording pattern edits) is commented out.
- **Most `#if 0` blocks** are dead code, e.g. the volume lookup by name in `Init_GWorlds`.
- **Timing:** recorder time is `TickCount() × 1.6` (60 Hz ticks), so recording resolution is ~16 ms despite the "1/100 s" claim.
- **Threading:** interrupt tasks and the main loop share every global with no synchronisation.
- **Stopping a sample** plays a silent buffer instead of stopping the channel. Starting it again **restarts from the beginning**. Only *Pause Sampler* (rate 0) keeps the playhead.
- **Starting a sample** (`freqCmd 1`) plays the loop from the beginning of the sound. `loopStart` / `loopEnd` decide whether the whole sound or only part of it loops.
- **The pan law** is linear balance, not equal-power.
- **Volumes above 256** (turbo) clip on output in the original.
- **Sample-rate handling:** everything is relative to 22 254.5 Hz (`rate22khz`), the classic Mac output rate.
- **The four windows** have fixed pixel layouts drawn from the Interface Plug-In sprite sheet (§18). The control rects in `Actions.cp` / `Drawings.cp` give the exact layout.
- **Track count:** the prefs dialog clamps tracks to 8–19 even though the code supports 30 and the manual says 30 (§4).
- **Fast Load folder rule:** a leading space hides a folder from the Fast Load menu (§17). The repo copies have lost the spaces.
- **Manual vs code:** the manual's text describes the older (pre-VST) skin's colours, and gives pitch limits of ±20 % / −32..32 where the code uses ±28 steps (≈±25 %). Trust the code.
