Format du fichier d'enregistrement Hit Factory 2.1

OFFSET 0	num�ro de version (short)

OFFSET 2	stucture record param
#define		maxNbRythmes		30
#define		maxNbInstruments	20
#define		maxNbInsChannels	10
typedef struct recordParam
{
	short				nbRythmes;
	short				nbInstruments;
	short				nbInsChannels;
	
	short				synthMode;
	short				cycleSpeed;
	
	short				RStatus[maxNbRythmes];
	short				RVol[maxNbRythmes];
	short				RPan[maxNbRythmes];
	char				RNote[maxNbRythmes];
	short				RCycling[maxNbRythmes];
	Boolean			RPlaying[maxNbRythmes];
	short				RSndNumber[maxNbRythmes];
	
	short				IStatus[maxNbInsChannels];
	short				IVol;
};

OFFSET
structure record
#define			nbNotes				800
typedef struct record
{
	long				time;
	unsigned short	action;
	short				command;
	short				command_2;
};

OFFSET
structure FFSpec
bytesNumber = 70;
for(i = 0; i < maxNbRythmes; ++i)
FSWrite(fileID, &bytesNumber, &rythmes[i].spec);
for(i = 0; i < maxNbInstruments; ++i)
FSWrite(fileID, &bytesNumber, &instruments[i].spec);