#include			"Constantes.h"
#include			"Structures.h"
#include			"Fonctions.h"
#include			"Variables.h"

//FONCTIONS:

typedef void (*SEPI_Proc)(Boolean, DialogPtr*);

void Init_EditDialog()
{
	SymClass			theClass;
	SEPI_Proc			theProcPtr;
	
	if(currentEditor == -1)
	return;
	
	UseResFile(mainResFile);
	GetIndString(theString, 0, 34);
	theError = FindSymbol(editorPlugIns[currentEditor].connID, theString, (char**) &theProcPtr, &theClass);
	if(theError) {
		Do_Error(theError, 105);
		return;
	}
	
	Hide_Menus();
	Anim_WaitCursor();
	
	(*theProcPtr)(editorPlugIns[currentEditor].update, &theDialog);
	editorPlugIns[currentEditor].update = false;
	/*if(theError) {
		SetCCursor(mouseCursor);
		Show_Menus();
		Do_Error(theError, 106);
		return;
	}*/
	
	SetCCursor(mouseCursor);
	dialogRunning = 3;
	saved = false;
	if(musicFileSpec.parID)
	EnableItem(menu[1], 4);
}

typedef void (*AEPI_Proc)(short, Point, short, Boolean*);

void Act_EditDialog(short item)
{
	SymClass			theClass;
	AEPI_Proc			theProcPtr;
	Boolean			finished = false;
	
	if(currentEditor == -1)
	return;
	
	UseResFile(mainResFile);
	GetIndString(theString, 0, 35);
	theError = FindSymbol(editorPlugIns[currentEditor].connID, theString, (char**) &theProcPtr, &theClass);
	if(theError) {
		Do_Error(theError, 105);
		return;
	}
	
	(*theProcPtr)(item, event.where, theKey, &finished);
	/*if(theError)
	Do_Error(theError, 106);*/
	
	if(finished) {
		SelectWindow(mainWin);
		dialogRunning = 0;
		Show_Menus();
	}
	
	if(theKey == kLeftArrowKey || theKey == kRightArrowKey)
	event.what = 0;
	if(theKey == kUpArrowKey || theKey == kDownArrowKey)
	event.what = 0;
}

typedef void (*UEPI_Proc)();

void Update_EditDialog()
{
	SymClass			theClass;
	UEPI_Proc			theProcPtr;
	
	if(currentEditor == -1)
	return;
	
	UseResFile(mainResFile);
	GetIndString(theString, 0, 36);
	theError = FindSymbol(editorPlugIns[currentEditor].connID, theString, (char**) &theProcPtr, &theClass);
	if(theError) {
		Do_Error(theError, 105);
		return;
	}
	
	(*theProcPtr)();
	/*if(theError)
	Do_Error(theError, 106);*/
}