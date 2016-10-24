/* praat_objectMenus.cpp
 *
 * Copyright (C) 1992-2012,2013,2014,2015,2016 Paul Boersma
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include "praatP.h"
#include "praat_script.h"
#include "ScriptEditor.h"
#include "ButtonEditor.h"
#include "DataEditor.h"
#include "site.h"
#include "GraphicsP.h"
//#include <string>

#undef iam
#define iam iam_LOOP

#define EDITOR  theCurrentPraatObjects -> list [IOBJECT]. editors

/********** Callbacks of the fixed buttons. **********/

DIRECT (PRAAT_Remove) {
	WHERE_DOWN (SELECTED)
		praat_removeObject (IOBJECT);
	praat_show ();
END }

FORM3 (MODIFY_Rename, U"Rename object", U"Rename...") {
	LABEL (U"rename object", U"New name:")
	TEXTFIELD (U"newName", U"")
OK
	int IOBJECT; WHERE (SELECTED) SET_STRING (U"newName", NAME)
DO
	char32 *string = GET_STRING (U"newName");
	if (theCurrentPraatObjects -> totalSelection == 0)
		Melder_throw (U"Selection changed!\nNo object selected. Cannot rename.");
	if (theCurrentPraatObjects -> totalSelection > 1)
		Melder_throw (U"Selection changed!\nCannot rename more than one object at a time.");
	WHERE (SELECTED) break;
	praat_cleanUpName (string);   // this is allowed because "string" is local and dispensible
	#if 0
	std::u32string newFullName = std::u32string (Thing_className (OBJECT) + U" " + string;
	if (newFullName != std::u32string (FULL_NAME)) {
		Melder_free (FULL_NAME), FULL_NAME = Melder_dup_f (newFullName.c_str());
		praat_list_renameAndSelect (IOBJECT, (std::u32string (Melder_integer (ID)) + U". " + newFullName). c_str());
		for (int ieditor = 0; ieditor < praat_MAXNUM_EDITORS; ieditor ++)
			if (EDITOR [ieditor]) Thing_setName (EDITOR [ieditor], newFullName.c_str());
		Thing_setName (OBJECT, string);
	}
	#else
	static MelderString fullName { 0 };
	MelderString_copy (& fullName, Thing_className (OBJECT), U" ", string);
	if (! str32equ (fullName.string, FULL_NAME)) {
		Melder_free (FULL_NAME), FULL_NAME = Melder_dup_f (fullName.string);
		autoMelderString listName;
		MelderString_append (& listName, ID, U". ", fullName.string);
		praat_list_renameAndSelect (IOBJECT, listName.string);
		for (int ieditor = 0; ieditor < praat_MAXNUM_EDITORS; ieditor ++)
			if (EDITOR [ieditor]) Thing_setName (EDITOR [ieditor], fullName.string);
		Thing_setName (OBJECT, string);
	}
	#endif
END }

FORM3 (NEW1_Copy, U"Copy object", U"Copy...") {
	LABEL (U"copy object", U"Name of new object:")
	TEXTFIELD (U"newName", U"")
OK
	int IOBJECT; WHERE (SELECTED) SET_STRING (U"newName", NAME)
DO
	if (theCurrentPraatObjects -> totalSelection == 0)
		Melder_throw (U"Selection changed!\nNo object selected. Cannot copy.");
	if (theCurrentPraatObjects -> totalSelection > 1)
		Melder_throw (U"Selection changed!\nCannot copy more than one object at a time.");
	WHERE (SELECTED) {
		char32 *name = GET_STRING (U"newName");
		praat_new (Data_copy ((Daata) OBJECT), name);
	}
END2 }

DIRECT (INFO_Info) {
	if (theCurrentPraatObjects -> totalSelection == 0)
		Melder_throw (U"Selection changed!\nNo object selected. Cannot query.");
	if (theCurrentPraatObjects -> totalSelection > 1)
		Melder_throw (U"Selection changed!\nCannot query more than one object at a time.");
	WHERE (SELECTED) Thing_infoWithIdAndFile (OBJECT, ID, & theCurrentPraatObjects -> list [IOBJECT]. file);
END }

DIRECT (WINDOW_Inspect) {
	if (theCurrentPraatObjects -> totalSelection == 0)
		Melder_throw (U"Selection changed!\nNo object selected. Cannot inspect.");
	if (theCurrentPraatApplication -> batch) {
		Melder_throw (U"Cannot inspect data from batch.");
	} else {
		WHERE (SELECTED) {
			autoDataEditor editor = DataEditor_create (ID_AND_FULL_NAME, OBJECT);
			praat_installEditor (editor.get(), IOBJECT);
			editor.releaseToUser();
		}
	}
END }

/********** The fixed menus. **********/

static GuiMenu praatMenu, editMenu, windowMenu, newMenu, readMenu, goodiesMenu, preferencesMenu, technicalMenu, applicationHelpMenu, helpMenu;

GuiMenu praat_objects_resolveMenu (const char32 *menu) {
	return
		str32equ (menu, U"Praat") || str32equ (menu, U"Control") ? praatMenu :
		#if cocoa
			str32equ (menu, U"Edit") ? editMenu :
			str32equ (menu, U"Window") ? windowMenu :
		#endif
		str32equ (menu, U"New") || str32equ (menu, U"Create") ? newMenu :
		str32equ (menu, U"Open") || str32equ (menu, U"Read") ? readMenu :
		str32equ (menu, U"Help") ? helpMenu :
		str32equ (menu, U"Goodies") ? goodiesMenu :
		str32equ (menu, U"Preferences") ? preferencesMenu :
		str32equ (menu, U"Technical") ? technicalMenu :
		#ifdef macintosh
			str32equ (menu, U"ApplicationHelp") ? applicationHelpMenu :
		#else
			str32equ (menu, U"ApplicationHelp") ? helpMenu :
		#endif
		newMenu;   // default
}

/********** Callbacks of the Praat menu. **********/

DIRECT (WINDOW_About) {
	praat_showLogo (false);
END }

DIRECT (WINDOW_praat_newScript) {
	autoScriptEditor editor = ScriptEditor_createFromText (nullptr, nullptr);
	editor.releaseToUser();
END }

DIRECT (WINDOW_praat_openScript) {
	autoScriptEditor editor = ScriptEditor_createFromText (nullptr, nullptr);
	TextEditor_showOpen (editor.get());
	editor.releaseToUser();
END }

static ButtonEditor theReferenceToTheOnlyButtonEditor;

static void cb_ButtonEditor_destruction (Editor /* editor */) {
	theReferenceToTheOnlyButtonEditor = nullptr;
}

DIRECT (WINDOW_praat_editButtons) {
	if (theReferenceToTheOnlyButtonEditor) {
		Editor_raise (theReferenceToTheOnlyButtonEditor);
	} else {
		autoButtonEditor editor = ButtonEditor_create ();
		Editor_setDestructionCallback (editor.get(), cb_ButtonEditor_destruction);
		theReferenceToTheOnlyButtonEditor = editor.get();
		editor.releaseToUser();
	}
END2 }

FORM3 (PRAAT_addMenuCommand, U"Add menu command", U"Add menu command...") {
	WORD (U"Window", U"Objects")
	WORD (U"Menu", U"New")
	SENTENCE (U"Command", U"Hallo...")
	SENTENCE (U"After command", U"")
	INTEGER (U"Depth", U"0")
	LABEL (U"", U"Script file:")
	TEXTFIELD (U"Script", U"/u/miep/hallo.praat")
OK
DO
	praat_addMenuCommandScript (GET_STRING (U"Window"), GET_STRING (U"Menu"),
		GET_STRING (U"Command"), GET_STRING (U"After command"),
		GET_INTEGER (U"Depth"), GET_STRING (U"Script"));
END2 }

FORM3 (PRAAT_hideMenuCommand, U"Hide menu command", U"Hide menu command...") {
	WORD (U"Window", U"Objects")
	WORD (U"Menu", U"New")
	SENTENCE (U"Command", U"Hallo...")
OK
DO
	praat_hideMenuCommand (GET_STRING (U"Window"), GET_STRING (U"Menu"), GET_STRING (U"Command"));
END2 }

FORM3 (PRAAT_showMenuCommand, U"Show menu command", U"Show menu command...") {
	WORD (U"Window", U"Objects")
	WORD (U"Menu", U"New")
	SENTENCE (U"Command", U"Hallo...")
OK
DO
	praat_showMenuCommand (GET_STRING (U"Window"), GET_STRING (U"Menu"), GET_STRING (U"Command"));
END2 }

FORM3 (PRAAT_addAction, U"Add action command", U"Add action command...") {
	WORD (U"Class 1", U"Sound")
	INTEGER (U"Number 1", U"0")
	WORD (U"Class 2", U"")
	INTEGER (U"Number 2", U"0")
	WORD (U"Class 3", U"")
	INTEGER (U"Number 3", U"0")
	SENTENCE (U"Command", U"Play reverse")
	SENTENCE (U"After command", U"Play")
	INTEGER (U"Depth", U"0")
	LABEL (U"", U"Script file:")
	TEXTFIELD (U"Script", U"/u/miep/playReverse.praat")
OK
DO
	praat_addActionScript (GET_STRING (U"Class 1"), GET_INTEGER (U"Number 1"),
		GET_STRING (U"Class 2"), GET_INTEGER (U"Number 2"), GET_STRING (U"Class 3"),
		GET_INTEGER (U"Number 3"), GET_STRING (U"Command"), GET_STRING (U"After command"),
		GET_INTEGER (U"Depth"), GET_STRING (U"Script"));
END2 }

FORM3 (PRAAT_hideAction, U"Hide action command", U"Hide action command...") {
	WORD (U"Class 1", U"Sound")
	WORD (U"Class 2", U"")
	WORD (U"Class 3", U"")
	SENTENCE (U"Command", U"Play")
OK
DO
	praat_hideAction_classNames (GET_STRING (U"Class 1"), GET_STRING (U"Class 2"), GET_STRING (U"Class 3"), GET_STRING (U"Command"));
END2 }

FORM3 (PRAAT_showAction, U"Show action command", U"Show action command...") {
	WORD (U"Class 1", U"Sound")
	WORD (U"Class 2", U"")
	WORD (U"Class 3", U"")
	SENTENCE (U"Command", U"Play")
OK
DO
	praat_showAction_classNames (GET_STRING (U"Class 1"), GET_STRING (U"Class 2"), GET_STRING (U"Class 3"), GET_STRING (U"Command"));
END }

/********** Callbacks of the Preferences menu. **********/

FORM3 (PREFS_TextInputEncodingSettings, U"Text reading preferences", U"Unicode") {
	RADIO_ENUM (U"Encoding of 8-bit text files", kMelder_textInputEncoding, DEFAULT)
OK
	SET_ENUM (U"Encoding of 8-bit text files", kMelder_textInputEncoding, Melder_getInputEncoding ())
DO
	Melder_setInputEncoding (GET_ENUM (kMelder_textInputEncoding, U"Encoding of 8-bit text files"));
END }

FORM3 (PREFS_TextOutputEncodingSettings, U"Text writing preferences", U"Unicode") {
	RADIO_ENUM (U"Output encoding", kMelder_textOutputEncoding, DEFAULT)
OK
	SET_ENUM (U"Output encoding", kMelder_textOutputEncoding, Melder_getOutputEncoding ())
DO
	Melder_setOutputEncoding (GET_ENUM (kMelder_textOutputEncoding, U"Output encoding"));
END }

FORM3 (PREFS_GraphicsCjkFontStyleSettings, U"CJK font style preferences", nullptr) {
	OPTIONMENU_ENUM (U"CJK font style", kGraphics_cjkFontStyle, DEFAULT)
OK
	SET_ENUM (U"CJK font style", kGraphics_cjkFontStyle, theGraphicsCjkFontStyle)
DO
	theGraphicsCjkFontStyle = GET_ENUM (kGraphics_cjkFontStyle, U"CJK font style");
END }

/********** Callbacks of the Goodies menu. **********/

FORM3 (STRING_praat_calculator, U"Calculator", U"Calculator") {
	LABEL (U"", U"Type any numeric formula or string formula:")
	TEXTFIELD (U"expression", U"5*5")
	LABEL (U"", U"Note that you can include many special functions in your formula,")
	LABEL (U"", U"including statistical functions and acoustics-auditory conversions.")
	LABEL (U"", U"For details, click Help.")
OK
DO
	struct Formula_Result result;
	if (! interpreter) {
		autoInterpreter tempInterpreter = Interpreter_create (nullptr, nullptr);
		Interpreter_anyExpression (tempInterpreter.get(), GET_STRING (U"expression"), & result);
	} else {
		Interpreter_anyExpression (interpreter, GET_STRING (U"expression"), & result);
	}
	switch (result. expressionType) {
		case kFormula_EXPRESSION_TYPE_NUMERIC: {
			Melder_information (result. result.numericResult);
		} break;
		case kFormula_EXPRESSION_TYPE_STRING: {
			Melder_information (result. result.stringResult);
			Melder_free (result. result.stringResult);
		} break;
		case kFormula_EXPRESSION_TYPE_NUMERIC_VECTOR: {
		} break;
		case kFormula_EXPRESSION_TYPE_NUMERIC_MATRIX: {
		}
	}
END }

FORM3 (INFO_reportDifferenceOfTwoProportions, U"Report difference of two proportions", U"Difference of two proportions") {
	INTEGER (U"left Row 1", U"71")
	INTEGER (U"right Row 1", U"39")
	INTEGER (U"left Row 2", U"93")
	INTEGER (U"right Row 2", U"27")
OK
DO
	double a = GET_INTEGER (U"left Row 1"), b = GET_INTEGER (U"right Row 1");
	double c = GET_INTEGER (U"left Row 2"), d = GET_INTEGER (U"right Row 2");
	double n = a + b + c + d;
	double aexp, bexp, cexp, dexp, crossDifference, x2;
	REQUIRE (a >= 0 && b >= 0 && c >= 0 && d >= 0, U"Numbers must not be negative.")
	REQUIRE ((a > 0 || b > 0) && (c > 0 || d > 0), U"Row totals must be positive.")
	REQUIRE ((a > 0 || c > 0) && (b > 0 || d > 0), U"Column totals must be positive.")
	MelderInfo_open ();
	MelderInfo_writeLine (U"Observed row 1 =    ", lround (a), U"    ", lround (b));
	MelderInfo_writeLine (U"Observed row 2 =    ", lround (c), U"    ", lround (d));
	aexp = (a + b) * (a + c) / n;
	bexp = (a + b) * (b + d) / n;
	cexp = (a + c) * (c + d) / n;
	dexp = (b + d) * (c + d) / n;
	MelderInfo_writeLine (U"");
	MelderInfo_writeLine (U"Expected row 1 =    ", aexp, U"    ", bexp);
	MelderInfo_writeLine (U"Expected row 2 =    ", cexp, U"    ", dexp);
	/*
	 * Continuity correction:
	 * bring the observed numbers closer to the expected numbers by 0.5 (if possible).
	 */
	if (a < aexp) { a += 0.5; if (a > aexp) a = aexp; }
	else if (a > aexp) { a -= 0.5; if (a < aexp) a = aexp; }
	if (b < bexp) { b += 0.5; if (b > bexp) b = bexp; }
	else if (b > bexp) { b -= 0.5; if (b < bexp) b = bexp; }
	if (c < cexp) { c += 0.5; if (c > cexp) c = cexp; }
	else if (c > cexp) { c -= 0.5; if (c < cexp) c = cexp; }
	if (d < dexp) { d += 0.5; if (d > dexp) d = dexp; }
	else if (d > dexp) { d -= 0.5; if (d < dexp) d = dexp; }
	MelderInfo_writeLine (U"");
	MelderInfo_writeLine (U"Corrected observed row 1 =    ", a, U"    ", b);
	MelderInfo_writeLine (U"Corrected observed row 2 =    ", c, U"    ", d);
	
	n = a + b + c + d;
	crossDifference = a * d - b * c;
	x2 = n * crossDifference * crossDifference / (a + b) / (c + d) / (a + c) / (b + d);
	MelderInfo_writeLine (U"");
	MelderInfo_writeLine (U"Chi-square =    ", x2);
	MelderInfo_writeLine (U"Two-tailed p =    ", NUMchiSquareQ (x2, 1));
	MelderInfo_close ();
END }

/********** Callbacks of the Technical menu. **********/

FORM3 (PRAAT_debug, U"Set debugging options", nullptr) {
	LABEL (U"", U"If you switch Tracing on, Praat will write lots of detailed ")
	LABEL (U"", U"information about what goes on in Praat")
	structMelderDir dir;
	Melder_getPrefDir (& dir);
	structMelderFile file;
	#ifdef UNIX
		MelderDir_getFile (& dir, U"tracing", & file);
	#else
		MelderDir_getFile (& dir, U"Tracing.txt", & file);
	#endif
	LABEL (U"", Melder_cat (U"to ", Melder_fileToPath (& file), U"."))
	BOOLEAN (U"Tracing", false)
	LABEL (U"", U"Setting the following to anything other than zero")
	LABEL (U"", U"will alter the behaviour of Praat")
	LABEL (U"", U"in unpredictable ways.")
	INTEGER (U"Debug option", U"0")
OK
	SET_INTEGER (U"Tracing", Melder_isTracing)
	SET_INTEGER (U"Debug option", Melder_debug)
DO
	Melder_setTracing (GET_INTEGER (U"Tracing"));
	Melder_debug = GET_INTEGER (U"Debug option");
END }

DIRECT (INFO_listReadableTypesOfObjects) {
	Thing_listReadableClasses ();
END }

FORM3 (INFO_praat_library_createCHeader, U"PraatLib: Create C header", nullptr) {
	BOOLEAN (U"Include \"Create\" API", true)
	BOOLEAN (U"Include \"Read\" API", true)
	BOOLEAN (U"Include \"Save\" API", true)
	BOOLEAN (U"Include \"Query\" API", true)
	BOOLEAN (U"Include \"Modify\" API", true)
	BOOLEAN (U"Include \"To\" API", true)
	BOOLEAN (U"Include \"Record\" API", true)
	BOOLEAN (U"Include \"Play\" API", true)
	BOOLEAN (U"Include \"Draw\" API", true)
	BOOLEAN (U"Include \"Help\" API", false)
	BOOLEAN (U"Include \"Window\" API", false)
	BOOLEAN (U"Include \"Demo\" API", false)
OK
DO
	praat_library_createCHeader (
		GET_INTEGER (U"Include \"Create\" API"),
		GET_INTEGER (U"Include \"Read\" API"),
		GET_INTEGER (U"Include \"Save\" API"),
		GET_INTEGER (U"Include \"Query\" API"),
		GET_INTEGER (U"Include \"Modify\" API"),
		GET_INTEGER (U"Include \"To\" API"),
		GET_INTEGER (U"Include \"Record\" API"),
		GET_INTEGER (U"Include \"Play\" API"),
		GET_INTEGER (U"Include \"Draw\" API"),
		GET_INTEGER (U"Include \"Help\" API"),
		GET_INTEGER (U"Include \"Window\" API"),
		GET_INTEGER (U"Include \"Demo\" API"));
END }

DIRECT (INFO_reportSystemProperties) {
	praat_reportSystemProperties ();
END }

DIRECT (INFO_reportGraphicalProperties) {
	praat_reportGraphicalProperties ();
END }

DIRECT (INFO_reportIntegerProperties) {
	praat_reportIntegerProperties ();
END }

DIRECT (INFO_reportMemoryUse) {
	praat_reportMemoryUse ();
END }

DIRECT (INFO_reportTextProperties) {
	praat_reportTextProperties ();
END }

/********** Callbacks of the Open menu. **********/

static void readFromFile (MelderFile file) {
	autoDaata object = Data_readFromFile (file);
	if (! object.get()) return;
	if (Thing_isa (object.get(), classManPages) && ! Melder_batch) {
		ManPages manPages = (ManPages) object.get();
		ManPage firstPage = manPages -> pages.at [1];
		autoManual manual = Manual_create (firstPage -> title, object.releaseToAmbiguousOwner(), true);
		if (manPages -> executable)
			Melder_warning (U"These manual pages contain links to executable scripts.\n"
				"Only navigate these pages if you trust their author!");
		manual.releaseToUser();
		return;
	}
	if (Thing_isa (object.get(), classScript) && ! Melder_batch) {
		autoScriptEditor editor = ScriptEditor_createFromScript_canBeNull (nullptr, (Script) object.get());
		if (! editor) {
			(void) 0;   // the script was already open, and the user has been notified of that
		} else {
			editor.releaseToUser();
		}
		return;
	}
	praat_newWithFile (object.move(), file, MelderFile_name (file));
	praat_updateSelection ();
}

FORM_READ (READMANY_Data_readFromFile, U"Read Object(s) from file", 0, true) {
	readFromFile (file);
END }

/********** Callbacks of the Save menu. **********/

FORM_SAVE (SAVE_Data_writeToTextFile, U"Save Object(s) as one text file", nullptr, nullptr) {
	if (theCurrentPraatObjects -> totalSelection == 1) {
		LOOP {
			iam (Daata);
			Data_writeToTextFile (me, file);
		}
	} else {
		autoCollection set = praat_getSelectedObjects ();
		Data_writeToTextFile (set.get(), file);
	}
END }

FORM_SAVE (SAVE_Data_writeToShortTextFile, U"Save Object(s) as one short text file", nullptr, nullptr) {
	if (theCurrentPraatObjects -> totalSelection == 1) {
		LOOP {
			iam (Daata);
			Data_writeToShortTextFile (me, file);
		}
	} else {
		autoCollection set = praat_getSelectedObjects ();
		Data_writeToShortTextFile (set.get(), file);
	}
END }

FORM_SAVE (SAVE_Data_writeToBinaryFile, U"Save Object(s) as one binary file", nullptr, nullptr) {
	if (theCurrentPraatObjects -> totalSelection == 1) {
		LOOP {
			iam (Daata);
			Data_writeToBinaryFile (me, file);
		}
	} else {
		autoCollection set = praat_getSelectedObjects ();
		Data_writeToBinaryFile (set.get(), file);
	}
END }

FORM3 (PRAAT_ManPages_saveToHtmlDirectory, U"Save all pages as HTML files", nullptr) {
	LABEL (U"", U"Type a directory name:")
	TEXTFIELD (U"directory", U"")
OK
	structMelderDir currentDirectory { { 0 } };
	Melder_getDefaultDir (& currentDirectory);
	SET_STRING (U"directory", Melder_dirToPath (& currentDirectory))
DO
	char32 *directory = GET_STRING (U"directory");
	LOOP {
		iam (ManPages);
		ManPages_writeAllToHtmlDir (me, directory);
	}
END }

DIRECT (WINDOW_ManPages_view) {
	LOOP {
		iam (ManPages);
		ManPage firstPage = my pages.at [1];
		autoManual manual = Manual_create (firstPage -> title, me, false);
		if (my executable)
			Melder_warning (U"These manual pages contain links to executable scripts.\n"
				"Only navigate these pages if you trust their author!");
		praat_installEditor (manual.get(), IOBJECT);
		manual.releaseToUser();
	}
END }

/********** Callbacks of the Help menu. **********/

FORM3 (HELP_SearchManual, U"Search manual", U"Manual") {
	LABEL (U"", U"Search for strings (separate with spaces):")
	TEXTFIELD (U"query", U"")
OK
DO
	if (theCurrentPraatApplication -> batch)
		Melder_throw (U"Cannot view a manual from batch.");
	autoManual manual = Manual_create (U"Intro", theCurrentPraatApplication -> manPages, false);
	Manual_search (manual.get(), GET_STRING (U"query"));
	manual.releaseToUser();
END }

FORM3 (HELP_GoToManualPage, U"Go to manual page", nullptr) {
	static long numberOfPages;
	static const char32 **pages = ManPages_getTitles (theCurrentPraatApplication -> manPages, & numberOfPages);
	LIST (U"Page", numberOfPages, pages, 1)
OK
DO
	if (theCurrentPraatApplication -> batch)
		Melder_throw (U"Cannot view a manual from batch.");
	autoManual manual = Manual_create (U"Intro", theCurrentPraatApplication -> manPages, false);
	HyperPage_goToPage_i (manual.get(), GET_INTEGER (U"Page"));
	manual.releaseToUser();
END }

FORM3 (HELP_WriteManualToHtmlDirectory, U"Save all pages as HTML files", nullptr) {
	LABEL (U"", U"Type a directory name:")
	TEXTFIELD (U"directory", U"")
OK
	structMelderDir currentDirectory { { 0 } };
	Melder_getDefaultDir (& currentDirectory);
	SET_STRING (U"directory", Melder_dirToPath (& currentDirectory))
DO
	char32 *directory = GET_STRING (U"directory");
	ManPages_writeAllToHtmlDir (theCurrentPraatApplication -> manPages, directory);
END }

/********** Menu descriptions. **********/

void praat_show () {
	/*
	 * (De)sensitivize the fixed buttons as appropriate for the current selection.
	 */
	praat_sensitivizeFixedButtonCommand (U"Remove", theCurrentPraatObjects -> totalSelection != 0);
	praat_sensitivizeFixedButtonCommand (U"Rename...", theCurrentPraatObjects -> totalSelection == 1);
	praat_sensitivizeFixedButtonCommand (U"Copy...", theCurrentPraatObjects -> totalSelection == 1);
	praat_sensitivizeFixedButtonCommand (U"Info", theCurrentPraatObjects -> totalSelection == 1);
	praat_sensitivizeFixedButtonCommand (U"Inspect", theCurrentPraatObjects -> totalSelection != 0);
	praat_actions_show ();
	if (theCurrentPraatApplication == & theForegroundPraatApplication && theReferenceToTheOnlyButtonEditor)
		Editor_dataChanged (theReferenceToTheOnlyButtonEditor);
}

/********** Menu descriptions. **********/

void praat_addFixedButtons (GuiWindow window) {
	praat_addFixedButtonCommand (window, U"Rename...", MODIFY_Rename, 8, 70);
	praat_addFixedButtonCommand (window, U"Copy...", NEW1_Copy, 98, 70);
	praat_addFixedButtonCommand (window, U"Inspect", WINDOW_Inspect, 8, 40);
	praat_addFixedButtonCommand (window, U"Info", INFO_Info, 98, 40);
	praat_addFixedButtonCommand (window, U"Remove", PRAAT_Remove, 8, 10);
}

static void searchProc () {
	HELP_SearchManual (nullptr, 0, nullptr, nullptr, nullptr, nullptr, false, nullptr);
}

static MelderString itemTitle_about { 0 };

static autoDaata scriptRecognizer (int nread, const char *header, MelderFile file) {
	const char32 *name = MelderFile_name (file);
	if (nread < 2) return autoDaata ();
	if ((header [0] == '#' && header [1] == '!') || str32str (name, U".praat") == name + str32len (name) - 6
	    || str32str (name, U".html") == name + str32len (name) - 5)
	{
		return Script_createFromFile (file);
	}
	return autoDaata ();
}

static void cb_openDocument (MelderFile file) {
	try {
		readFromFile (file);
	} catch (MelderError) {
		Melder_flushError ();
	}
}

#if cocoa
DIRECT2 (praat_cut) {
	[[[NSApp keyWindow] fieldEditor: YES forObject: nil] cut: nil];
END2 }
DIRECT2 (praat_copy) {
	[[[NSApp keyWindow] fieldEditor: YES forObject: nil] copy: nil];
END2 }
DIRECT2 (praat_paste) {
	[[[NSApp keyWindow] fieldEditor: YES forObject: nil] pasteAsPlainText: nil];
END2 }
DIRECT2 (praat_minimize) {
	[[NSApp keyWindow] performMiniaturize: nil];
END2 }
DIRECT2 (praat_zoom) {
	[[NSApp keyWindow] performZoom: nil];
END2 }
DIRECT2 (praat_close) {
	[[NSApp keyWindow] performClose: nil];
END2 }
#endif

void praat_addMenus (GuiWindow window) {
	Melder_setSearchProc (searchProc);

	Data_recognizeFileType (scriptRecognizer);

	/*
	 * Create the menu titles in the bar.
	 */
	if (! theCurrentPraatApplication -> batch) {
		#ifdef macintosh
			praatMenu = GuiMenu_createInWindow (nullptr, U"\024", 0);
			#if cocoa
				editMenu = GuiMenu_createInWindow (nullptr, U"Edit", 0);
				windowMenu = GuiMenu_createInWindow (nullptr, U"Window", 0);
			#endif
		#else
			praatMenu = GuiMenu_createInWindow (window, U"Praat", 0);
		#endif
		newMenu = GuiMenu_createInWindow (window, U"New", 0);
		readMenu = GuiMenu_createInWindow (window, U"Open", 0);
		praat_actions_createWriteMenu (window);
		#ifdef macintosh
			applicationHelpMenu = GuiMenu_createInWindow (nullptr, U"Help", 0);
		#endif
		helpMenu = GuiMenu_createInWindow (window, U"Help", 0);
	}
	
	MelderString_append (& itemTitle_about, U"About ", praatP.title, U"...");
	#ifdef macintosh
		praat_addMenuCommand (U"Objects", U"Praat", itemTitle_about.string, nullptr, praat_UNHIDABLE, WINDOW_About);
		#if cocoa
			/*
			 * HACK: give the following command weird names,
			 * because otherwise they may be called from a script.
			 * (we add three alt-spaces)
			 */
			praat_addMenuCommand (U"Objects", U"Edit", U"Cut   ", nullptr, praat_UNHIDABLE | 'X' | praat_NO_API, DO_praat_cut);
			praat_addMenuCommand (U"Objects", U"Edit", U"Copy   ", nullptr, praat_UNHIDABLE | 'C' | praat_NO_API, DO_praat_copy);
			praat_addMenuCommand (U"Objects", U"Edit", U"Paste   ", nullptr, praat_UNHIDABLE | 'V' | praat_NO_API, DO_praat_paste);
			praat_addMenuCommand (U"Objects", U"Window", U"Minimize   ", nullptr, praat_UNHIDABLE | praat_NO_API, DO_praat_minimize);
			praat_addMenuCommand (U"Objects", U"Window", U"Zoom   ", nullptr, praat_UNHIDABLE | praat_NO_API, DO_praat_zoom);
			praat_addMenuCommand (U"Objects", U"Window", U"Close   ", nullptr, 'W' | praat_NO_API, DO_praat_close);
		#endif
	#endif
	#ifdef UNIX
		praat_addMenuCommand (U"Objects", U"Praat", itemTitle_about.string, nullptr, praat_UNHIDABLE, WINDOW_About);
	#endif
	praat_addMenuCommand (U"Objects", U"Praat", U"-- script --", nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Praat", U"New Praat script", nullptr, praat_NO_API, WINDOW_praat_newScript);
	praat_addMenuCommand (U"Objects", U"Praat", U"Open Praat script...", nullptr, praat_NO_API, WINDOW_praat_openScript);
	praat_addMenuCommand (U"Objects", U"Praat", U"-- buttons --", nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Praat", U"Add menu command...", nullptr, praat_HIDDEN | praat_NO_API, PRAAT_addMenuCommand);
	praat_addMenuCommand (U"Objects", U"Praat", U"Hide menu command...", nullptr, praat_HIDDEN | praat_NO_API, PRAAT_hideMenuCommand);
	praat_addMenuCommand (U"Objects", U"Praat", U"Show menu command...", nullptr, praat_HIDDEN | praat_NO_API, PRAAT_showMenuCommand);
	praat_addMenuCommand (U"Objects", U"Praat", U"Add action command...", nullptr, praat_HIDDEN | praat_NO_API, PRAAT_addAction);
	praat_addMenuCommand (U"Objects", U"Praat", U"Hide action command...", nullptr, praat_HIDDEN | praat_NO_API, PRAAT_hideAction);
	praat_addMenuCommand (U"Objects", U"Praat", U"Show action command...", nullptr, praat_HIDDEN | praat_NO_API, PRAAT_showAction);

	GuiMenuItem menuItem = praat_addMenuCommand (U"Objects", U"Praat", U"Goodies", nullptr, praat_UNHIDABLE, nullptr);
	goodiesMenu = menuItem ? menuItem -> d_menu : nullptr;
	praat_addMenuCommand (U"Objects", U"Goodies", U"Calculator...", nullptr, 'U', STRING_praat_calculator);
	praat_addMenuCommand (U"Objects", U"Goodies", U"Report difference of two proportions...", nullptr, 0, INFO_reportDifferenceOfTwoProportions);

	menuItem = praat_addMenuCommand (U"Objects", U"Praat", U"Preferences", nullptr, praat_UNHIDABLE, nullptr);
	preferencesMenu = menuItem ? menuItem -> d_menu : nullptr;
	praat_addMenuCommand (U"Objects", U"Preferences", U"Buttons...", nullptr, praat_UNHIDABLE, WINDOW_praat_editButtons);   // cannot be hidden
	praat_addMenuCommand (U"Objects", U"Preferences", U"-- encoding prefs --", nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Preferences", U"Text reading preferences...", nullptr, 0, PREFS_TextInputEncodingSettings);
	praat_addMenuCommand (U"Objects", U"Preferences", U"Text writing preferences...", nullptr, 0, PREFS_TextOutputEncodingSettings);
	praat_addMenuCommand (U"Objects", U"Preferences", U"CJK font style preferences...", nullptr, 0, PREFS_GraphicsCjkFontStyleSettings);

	menuItem = praat_addMenuCommand (U"Objects", U"Praat", U"Technical", nullptr, praat_UNHIDABLE, nullptr);
	technicalMenu = menuItem ? menuItem -> d_menu : nullptr;
	praat_addMenuCommand (U"Objects", U"Technical", U"Report memory use", nullptr, 0, INFO_reportMemoryUse);
	praat_addMenuCommand (U"Objects", U"Technical", U"Report integer properties", nullptr, 0, INFO_reportIntegerProperties);
	praat_addMenuCommand (U"Objects", U"Technical", U"Report text properties", nullptr, 0, INFO_reportTextProperties);
	praat_addMenuCommand (U"Objects", U"Technical", U"Report system properties", nullptr, 0, INFO_reportSystemProperties);
	praat_addMenuCommand (U"Objects", U"Technical", U"Report graphical properties", nullptr, 0, INFO_reportGraphicalProperties);
	praat_addMenuCommand (U"Objects", U"Technical", U"Debug...", nullptr, 0, PRAAT_debug);
	praat_addMenuCommand (U"Objects", U"Technical", U"-- api --", nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Technical", U"List readable types of objects", nullptr, 0, INFO_listReadableTypesOfObjects);
	praat_addMenuCommand (U"Objects", U"Technical", U"Create C header", nullptr, 0, INFO_praat_library_createCHeader);

	praat_addMenuCommand (U"Objects", U"Open", U"Read from file...", nullptr, praat_ATTRACTIVE | 'O', READMANY_Data_readFromFile);

	praat_addAction1 (classDaata, 0, U"Save as text file...", nullptr, 0, SAVE_Data_writeToTextFile);
	praat_addAction1 (classDaata, 0,   U"Write to text file...", nullptr, praat_DEPRECATED_2011, SAVE_Data_writeToTextFile);
	praat_addAction1 (classDaata, 0, U"Save as short text file...", nullptr, 0, SAVE_Data_writeToShortTextFile);
	praat_addAction1 (classDaata, 0,   U"Write to short text file...", nullptr, praat_DEPRECATED_2011, SAVE_Data_writeToShortTextFile);
	praat_addAction1 (classDaata, 0, U"Save as binary file...", nullptr, 0, SAVE_Data_writeToBinaryFile);
	praat_addAction1 (classDaata, 0,   U"Write to binary file...", nullptr, praat_DEPRECATED_2011, SAVE_Data_writeToBinaryFile);

	praat_addAction1 (classManPages, 1, U"Save to HTML directory...", nullptr, 0, PRAAT_ManPages_saveToHtmlDirectory);
	praat_addAction1 (classManPages, 1, U"View", nullptr, 0, WINDOW_ManPages_view);
}

void praat_addMenus2 () {
	praat_addMenuCommand (U"Objects", U"ApplicationHelp", U"-- manual --", nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"ApplicationHelp", U"Go to manual page...", nullptr, 0, HELP_GoToManualPage);
	praat_addMenuCommand (U"Objects", U"ApplicationHelp", U"Write manual to HTML directory...", nullptr, praat_HIDDEN, HELP_WriteManualToHtmlDirectory);
	praat_addMenuCommand (U"Objects", U"ApplicationHelp",
		Melder_cat (U"Search ", praatP.title, U" manual..."),
		nullptr, 'M' | praat_NO_API, HELP_SearchManual);
	#ifdef _WIN32
		praat_addMenuCommand (U"Objects", U"Help", U"-- about --", nullptr, 0, nullptr);
		praat_addMenuCommand (U"Objects", U"Help", itemTitle_about.string, nullptr, praat_UNHIDABLE, WINDOW_About);
	#endif

	#if defined (macintosh) || defined (_WIN32)
		Gui_setOpenDocumentCallback (cb_openDocument);
	#endif
}

/* End of file praat_objectMenus.cpp */
