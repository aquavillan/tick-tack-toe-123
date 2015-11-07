// 2D Tick Tack Toe game - developed by Yakovich Nikiforov
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author : Yakovich Nikiforov
// Keys : Use 'Up' - 'Down' - 'Left' - 'Right' keys to move the map

#include <stdio.h>
#include <math.h>
#include "include\hge.h"
#include "include\hgesprite.h"
#include "include\hgecolor.h"


HGE *hge = 0; 
HWND HWnd = 0; // Second window
bool bInitialize_secondWindow = false;

HEFFECT XO_Music = 0;
HEFFECT XO_Winning = 0;
HEFFECT XO_Click = 0;

// First window parameters
int nScreen_Width  = 800;
int nScreen_Height = 600;

// Second window parameters
int nScreen_Width2  = 800;
int nScreen_Height2 = 300;

#define ID_BUTTON_EXIT WM_USER + 60
///////////////////////////////////////////
// New functions
// InfoImport()
// PreInitialize()
// UpdateInfo() 
///////////////////////////////////////////

// Define all the parameters for game
///////////////////////////////////////////
#define BORDER_LEFT 50
#define BORDER_RIGHT 50
#define BORDER_TOP 120
#define BORDER_BOTTOM 50

#define Rect_Length(rt) (rt.x2 - rt.x1)
#define Rect_Height(rt) (rt.y2 - rt.y1)

int nBorder_Thickness = 20;

int nPosBorderLeft = 0;
int nPosBorderRight = 0;
int nPosBorderTop = 0;
int nPosBorderBottom = 0;

bool bGameDebug = false;
hgeRect rtPlayArea;

int nBoxSize = 50;
float fBoxesDistance = 10.0f;
hgeColorRGB colBolder;

bool bXTurn = false; // If it is false, then it's 'O' turn =D

#define BOXSTATUS_WIDTH 80
#define BOXSTATUS_HEIGHT 80
#define BOXSTATUS_BORDER 10

hgeRect rtBoxStatus;
hgeRect rtStatusArea;

#define NUMBER_BOXES_EACH 200


char XY_Map[NUMBER_BOXES_EACH][NUMBER_BOXES_EACH];
int nViewX = NUMBER_BOXES_EACH / 2.5, nViewY = NUMBER_BOXES_EACH / 2.5;

#define EFFECT_SELECTED 0x01
#define EFFECT_WINNING 0x02

// They will be initialized later, so they should be set zero
hgeSprite * sprBackground = 0;
hgeSprite * sprMiniBackground = 0;
hgeSprite * sprBorderLine = 0;
hgeSprite * sprMiniBorderLine = 0;

#define MAX_BOXTYPES 1000

int nComboMax = 5;
hgeSprite *sprBoxList[MAX_BOXTYPES];

#define XO_EMPTY_BOX 0x0
#define XO_X_BOX 0x1
#define XO_O_BOX 0x2
#define XO_OBSTACLE_BOX 0x3

int nRegistered_Boxes = XO_OBSTACLE_BOX + 1;

hgeSprite * sprBoxX = 0;
hgeSprite * sprBoxO = 0;
hgeSprite * sprBoxEmpty = 0;
hgeSprite * sprObstacleBox = 0;

hgeSprite * sprPlayArea = 0; // It will be rendered first before the boxes
hgeSprite * sprMiniPlayArea = 0; 

hgeSprite * sprCursor = 0;

hgeColor colBackground;
hgeColor colPlayArea;
hgeColor colBorder;

bool bXO_Win = false;

#define MINI_BACKGROUND 0
#define MINI_PLAYAREA 1
#define MINI_BOXES 2
#define MINI_BORDERS  3
#define MINI_STATUS_BORDERS  4

// Return values : 
// -1 : File ini not found
// 0 : Loaded unsuccessfully
// 1 : Sucessfully loaded
// To get the default "section_name", we get the file name but not its file path

int strlenA(char *str) // There might be some mistake that the built-in function strlen() is not loading. Define another one instead
{
	int len = 0;if(str)while(*str){++len;++str;} return len; 
}

int strcmpA(char *s1, char *s2) // Returns 0 if succeeded, and -1 otherwise
{
	int i, len = strlenA(s1);
	if(len != strlenA(s2))return -1;

	for(i = 0; i < len;++i)
	if(s1[i] != s2[i]) return -1;

	return 0;
}

bool getShortFileNameA(char *full_path, char *result, char *leftover) // returns bool
{
	int len = strlenA(full_path);
	
	if(len == 0)return false;
	if((len == 1) && (full_path[0] == '\\'))return false;

	while((--len) >= 0)
	if(full_path[len] == '\\')
	{
		strcpy(result, &full_path[len + 1]); 
		if(strlenA(result) > 0){strcpy(leftover, full_path); leftover[len + 1] = 0; return true;}

		return false;
	}

	// Otherwise, no backslash is found - Can breath soundly while copying =D
	strcpy(result, full_path); leftover[0] = 0; return true;
}


int LoadBitmap(hgeSprite **sprTarget, char *file_name, char *str_section = 0) // "file_name" is also "section_name" by default
{
	HTEXTURE tex;
	char strTexFile[280];
	char strTemp[600];
	char strTemp2[300];
	char strSection[300];
	float x, y, w, h;

	char *section_name;
	if(str_section)
	{
		section_name = str_section;
		getShortFileNameA(file_name, strSection, strTemp2); // The 'leftover' part is pretty annoying...Still have to run this =D
	}
	else
	{
		if(getShortFileNameA(file_name, strSection, strTemp2) == false) // strTemp2 handles the 'leftover' =D
		{
			sprintf(strTemp, "The INI File \"%s\" is not invalid. Please check and try again", file_name);
			MessageBoxA(0, strTemp, "Msg", 0); return -1;
		}

		section_name = strSection;
	}

	//if(access(file_name, 0) != -1)
	//{
	//	sprintf(strTemp, "The INI file \"%s\" is not found. Please check and try again", file_name);
	//	MessageBoxA(0, strTemp, "Msg", 0); return -1;
	//}

	hge->System_SetState(HGE_INIFILE, file_name);

	strcpy(strTexFile, strTemp2);
	strcat(strTexFile, hge->Ini_GetString(section_name, "texture", "not_set"));

	printf("    Loading texture : %s\n", strTexFile);
	tex = hge->Texture_Load(strTexFile);

	if(tex == 0)
	{
		sprintf(strTemp, "An important file named \"%s\" defined in \"%s\" is not found or not set or not valid. Please check and try again", strTexFile, file_name);
		MessageBoxA(0, strTemp, "Msg", 0); return false;
	}

	x = hge->Ini_GetFloat(section_name, "x", -1.00f);
	y = hge->Ini_GetFloat(section_name, "y", -1.00f);
	w = hge->Ini_GetFloat(section_name, "w", -1.00f);
	h = hge->Ini_GetFloat(section_name, "h", -1.00f);

	if(w == 0)w = hge->Texture_GetWidth(tex) - x;
	if(h == 0)h = hge->Texture_GetHeight(tex) - y;

	if((x < 0) || (y < 0) || (w < 0) || (h < 0))
	{
		sprintf(strTemp, "Sprite Parameters for file \"%s\" defined in \"%s\" are not correct.\nReason : One of those values (x, y, w, h) is below zero or not defined", strTexFile, file_name);
		MessageBoxA(0, strTemp, "Msg", 0); return false;
	}

	if((w == 0) || (h == 0))
	{
		sprintf(strTemp, "Sprite Parameters for file \"%s\" defined in \"%s\" are not correct.\nReason : No width or no height", strTexFile, file_name);
		MessageBoxA(0, strTemp, "Msg", 0); return false;
	}

	if((w > hge->Texture_GetWidth(tex)) || (h > hge->Texture_GetHeight(tex)))
	{
		sprintf(strTemp, "Sprite Parameters for file \"%s\" defined in \"%s\" are not correct.\nReason : Invalid width or height", strTexFile, file_name);
		MessageBoxA(0, strTemp, "Msg", 0); return false;
	}

	(*sprTarget) = new hgeSprite(tex, x, y, w, h);
	
	return true;
}

		
int PreLoadBitmap(hgeSprite **sprTarget, char *section_name, char *key_name)
{
	int retCode;
	char strPath[280];
	char strSection[300];
	char strTemp[300];

	strcpy(strPath, hge->Ini_GetString(section_name, key_name, "not_set"));

	if(strcmpA(strPath, "not_set") == 0)return -1; // File not set (not found)

	if(5 - 4) // if(access(strPath, 0) != -1)
	{
		strcpy(strTemp, key_name); strcat(strTemp, "_Section");
		strcpy(strSection, hge->Ini_GetString(section_name, strTemp, "not_set"));

		if(strcmpA(strSection, "not_set") != 0)
			retCode = LoadBitmap(sprTarget, strPath, strSection);else 
			retCode = LoadBitmap(sprTarget, strPath, 0);

		// Restore the INI settings
		hge->System_SetState(HGE_INIFILE, strWorkingIniFile);

		if(retCode == 0) // Error when loading the file
		{
			sprintf(strTemp, "An error occured when trying to load \"%s\" defined in \"%s\"->\"%s\". \nPlease check and try again");
			FatalAppExitA(0, strTemp);
		}	
		if(retCode == -1)return false; // File not found - but please consider LoadBitmapDouble(); we will handle it later
		if(retCode == 1)return true; // Merely for debugging purpose
	}
	return false;
}


void LoadBitmapDouble(hgeSprite **sprBigVer, hgeSprite **sprSmallVer, char *section_name, char *key_name1, char *key_name2)
{
	char strTemp[400];
	if(PreLoadBitmap(sprBigVer, section_name, key_name1) == -1)
	{
		if(PreLoadBitmap(sprSmallVer, section_name, key_name2) == -1)
		{
			sprintf(strTemp, "Error : Either key \"%s\" or key \"%s\" cannot be loaded; the file is not found or not set", key_name1, key_name2);
			FatalAppExitA(0, strTemp);
		}
	}
}




void PreRenderBorder(int nMode)
{
	hgeSprite *sprTemp = 0;
	int i, j, id, x1, y1, x2, y2;
	float fX = 0, fY = 0;
	long int nTmp;
	
	float fWidth, fHeight;
	int numCol, numRow;

	id = -1;
	sprTemp = sprMiniBorderLine;
	fWidth = sprTemp->GetWidth(); fHeight = sprTemp->GetHeight();

	hgeRect border_rects[4];

	if(nMode == MINI_BORDERS)
	{
		border_rects[0].Set(nPosBorderLeft, nPosBorderTop, nPosBorderRight, rtPlayArea.y1);
		border_rects[1].Set(nPosBorderLeft, nPosBorderTop, rtPlayArea.x1, rtPlayArea.y2);
		border_rects[2].Set(rtPlayArea.x2, nPosBorderTop, nPosBorderRight, rtPlayArea.y2);
		border_rects[3].Set(nPosBorderLeft, rtPlayArea.y2, nPosBorderRight, nPosBorderBottom);
	}
	else
	{
		border_rects[0].Set(rtStatusArea.x1, rtStatusArea.y1, rtStatusArea.x2, rtBoxStatus.y1);
		border_rects[1].Set(rtStatusArea.x1, rtStatusArea.y1, rtBoxStatus.x1, rtBoxStatus.y2);
		border_rects[2].Set(rtBoxStatus.x2, rtStatusArea.y1, rtStatusArea.x2, rtBoxStatus.y2);
		border_rects[3].Set(rtStatusArea.x1, rtBoxStatus.y2, rtStatusArea.x2, rtStatusArea.y2);
	}

	while(++id < 4)
	{
		
		x1 = border_rects[id].x1; 
		y1 = border_rects[id].y1; 
		x2 = border_rects[id].x2; 
		y2 = border_rects[id].y2;

		numCol = (x2 - x1) / fWidth;
		if(numCol < 1)numCol = 1; fWidth = (x2 - x1) / numCol;

		numRow = (y2 - y1) / fHeight; 
		if(numRow < 1)numRow = 1; fHeight = (y2 - y1) / numRow;

		// Square patterns look much nicer
		if(fWidth != fHeight)
		{
			if(fWidth < fHeight)
			{
				fHeight = fWidth;
				numRow = (y2 - y1) / fHeight; if(numRow < 1)numRow = 1; fHeight = (y2 - y1) / numRow;
				
			} 
			else 
			{
				fWidth = fHeight;
				numCol = (x2 - x1) / fWidth; if(numCol < 1)numCol = 1;  fWidth = (x2 - x1) / numCol;
			}
		}


		fY = y1;
		for(i = 0;i < numRow;++i)
		{
			fX = x1;
			for(j = 0;j < numCol;++j)
			{
				sprTemp->RenderStretch(fX, fY, fX + fWidth, fY + fHeight); fX += fWidth;
			} 
			fY += fHeight;
		}
	}




}

int RenderMiniBackground(int nMode)
{
	hgeSprite *sprTemp = 0;
	int i, j, id, x1, y1, x2, y2;
	float fX = 0, fY = 0;
	hgeSprite *nTmp;
	
	float fWidth, fHeight;
	int numCol, numRow;

	switch(nMode)
	{
		case MINI_BACKGROUND : 
			sprTemp = sprMiniBackground; x1 = 0; y1 = 0; x2 = nScreen_Width; y2 = nScreen_Height;
			sprTemp->SetColor(colBackground.GetHWColor());
		break;
		case MINI_PLAYAREA : 
			sprTemp = sprPlayArea; x1 = nPosBorderLeft; y1 = nPosBorderRight; x2 = nPosBorderTop; y2 = nPosBorderBottom;
			sprTemp->SetColor(colPlayArea.GetHWColor());
		break;
		case MINI_BOXES : 
			sprTemp = sprBoxX; x1 = rtPlayArea.x1; y1 = rtPlayArea.y1; x2 = rtPlayArea.x2; y2 = rtPlayArea.y2;
		break;
		case MINI_BORDERS :
			PreRenderBorder(nMode);
		return true; break;
		case MINI_STATUS_BORDERS : 
			PreRenderBorder(nMode);
		return true; break;
		default : 
			FatalAppExitA(0, "Error calling RenderMiniBackground() function - Mode should be from (0) to (3)");
		break;
	}

	nTmp = sprTemp;

	// There may be some error when comparing a pointer with a regular value. Please use a basic type variable to compare
	if(nTmp == 0)
	FatalAppExitA(0, "Fatal Error : sprTemp required for Rendering is NULL");
	fWidth = sprTemp->GetWidth(); fHeight = sprTemp->GetHeight();

	if(nMode == MINI_BOXES)
	{
		fWidth = nBoxSize;
		fHeight = nBoxSize;
	}
	
	// Round values - so that they can fit the play area
	numCol = (x2 - x1) / fWidth; fWidth = (x2 - x1) / numCol;
	numRow = (y2 - y1) / fHeight; fHeight = (y2 - y1) / numRow;

	if(nMode != MINI_BOXES)
	{
		fY = y1;
		for(i = 0;i < numRow;++i)
		{
			fX = x1;
			for(j = 0;j < numCol;++j)
			{
				sprTemp->RenderStretch(fX, fY, fX + fWidth, fY + fHeight); fX += fWidth;
			} 
			fY += fHeight;
		}
	}
	else // if(nMode == MINI_BOXES)
	{
		float fBoxWidth = fWidth - fBoxesDistance;
		float fBoxHeight = fHeight - fBoxesDistance; 

		fY = y1 + fBoxesDistance / 2.0f;
		for(i = 0;i < numRow;++i)
		{
			fX = x1 + fBoxesDistance / 2.0f;
			for(j = 0;j < numCol;++j)
			{
				// This decides which type of box it should draw
				// Debug version
				/*
				switch(XY_Map[nViewX + i][nViewY + j])
				{
					case 0 : sprTemp = sprBoxEmpty; break;
					case 1 : sprTemp = sprBoxX; break;
					case 2 : sprTemp = sprBoxO; break;
					default : 
					FatalAppExit(0, "I don't get your point. Invalid value, or wanna a Z box?");
				}
				*/
	
				id = XY_Map[nViewX + i][nViewY + j];
				if((id >= 0) && (id < nRegistered_Boxes))
					sprTemp = sprBoxList[id];
				else FatalAppExitA(0, "A unknown box due to either unregistered or miscaculation. Please check and try again");
				
				sprTemp->RenderStretch(fX, fY, fX + fBoxWidth, fY + fBoxHeight); fX += fWidth;
			}
			fY += fHeight;
		}

	}
}


bool GetSelectedBox(float x, float y, int *nSelectedBoxX, int *nSelectedBoxY) // returns bool
{
	hgeRect rtTmp;

	int nX, nY;
	int x1, y1, x2, y2;
	float fX = 0, fY = 0;
	
	float fWidth, fHeight;
	int numCol, numRow;

	x1 = rtPlayArea.x1; y1 = rtPlayArea.y1; x2 = rtPlayArea.x2; y2 = rtPlayArea.y2;

	fWidth = nBoxSize;
	fHeight = nBoxSize;

	float fBoxWidth = fWidth - fBoxesDistance;
	float fBoxHeight = fHeight - fBoxesDistance; 

	numCol = (x2 - x1) / fWidth; fWidth = (x2 - x1) / numCol;
	numRow = (y2 - y1) / fHeight; fHeight = (y2 - y1) / numRow;

	nX = (x - x1) / fWidth; 
	nY = (y - y1) / fHeight; 

	// Check if the cursor is between the space between boxes or not
	fX = x1 + fBoxesDistance / 2.0f + fWidth * nX;
	fY = y1 + fBoxesDistance / 2.0f + fHeight * nY;

	rtTmp.Set(fX, fY, fX + fWidth - fBoxesDistance / 2.0f, fY + fHeight - fBoxesDistance / 2.0f);

	if(rtTmp.TestPoint(x, y))
	{
		*nSelectedBoxX = nX;
		*nSelectedBoxY = nY;

		return true;
	}
	return false;
}

void RenderBackground() // Render background and render play area
{
	if(sprMiniBackground)RenderMiniBackground(MINI_BACKGROUND);
	else if(sprBackground)
	{
		sprBackground->SetColor(colBackground.GetHWColor());
		sprBackground->RenderStretch(0, 0, nScreen_Width, nScreen_Height);
	}

	if(sprMiniPlayArea)RenderMiniBackground(MINI_PLAYAREA);
	else if(sprPlayArea)
	{
		sprPlayArea->SetColor(colPlayArea.GetHWColor());
		sprPlayArea->RenderStretch(nPosBorderLeft, nPosBorderTop, nPosBorderRight, nPosBorderBottom);
	}
}

bool RenderFunc()
{
	// To do : Add your own code here
	hge->Gfx_BeginScene();

	if(bGameDebug)
	MessageBoxA(0, "Start rendering!!!", "Msg", 0);
	
	// Render background
	RenderBackground();

	// Render boxes
	RenderMiniBackground(MINI_BOXES);

	// Render border - The border lines are rendered after rendering all the boxes
	if(sprBorderLine)
	{
		sprBorderLine->RenderStretch(nPosBorderLeft, nPosBorderTop, nPosBorderRight, rtPlayArea.y1);
		sprBorderLine->RenderStretch(nPosBorderLeft, nPosBorderTop, rtPlayArea.x1, rtPlayArea.y2);

		sprBorderLine->RenderStretch(rtPlayArea.x2, nPosBorderTop, nPosBorderRight, rtPlayArea.y2);
		sprBorderLine->RenderStretch(nPosBorderLeft, rtPlayArea.y2, nPosBorderRight, nPosBorderBottom);
	
	} 
	else if(sprMiniBorderLine)
	{
		RenderMiniBackground(MINI_BORDERS);
	}

	// Render box status
	if(bXTurn)
	{
		sprBoxX->SetColor(0xFFFFFFFF);
		sprBoxX->RenderStretch(rtBoxStatus.x1, rtBoxStatus.y1, rtBoxStatus.x2, rtBoxStatus.y2);
	}
	else
	{
		sprBoxO->SetColor(0xFFFFFFFF);
		sprBoxO->RenderStretch(rtBoxStatus.x1, rtBoxStatus.y1, rtBoxStatus.x2, rtBoxStatus.y2);
	}
	RenderMiniBackground(MINI_STATUS_BORDERS);

	// Render cursor
	if(sprCursor)
	{
		float x, y;
		hge->Input_GetMousePos(&x, &y);
		sprCursor->RenderStretch(x, y, x + 32, y + 32);
	}

	hge->Gfx_EndScene();

	if(bGameDebug)
	MessageBoxA(0, "Rendering completes!!!", "Msg", 0);

	if(bXO_Win == true)
	{
		Sleep(100);
		hge->Effect_Play(XO_Winning);

		Sleep(300);
		if(bXTurn == true)FatalAppExitA(0, "X Player Win!!!");
		if(bXTurn == false)FatalAppExitA(0, "O Player Win!!!");
	}

	return false;
}

void InfoImport()
{
	// ToDo : Add your code here
}

void UpdateInfo() // Writes all necessary settings to one INI file
{
	// ToDo : Add your code here
}

void AfterInitialize()
{
	HTEXTURE tex;

	if(hge->Random_Int(0, 100) > 50)bXTurn = true;

	XO_Music = hge->Effect_Load("XO_Game\\XO_Music.mp3", 0);
	if(XO_Music == 0)FatalAppExitA(0, "The music cannot be loaded....");

	XO_Winning = hge->Effect_Load("XO_Game\\XO_Winning.mp3", 0);
	if(XO_Winning == 0)FatalAppExitA(0, "The winning sound cannot be loaded....");

	XO_Click = hge->Effect_Load("XO_Game\\XO_Click.mp3", 0);
	if(XO_Click == 0)FatalAppExitA(0, "The click sound cannot be loaded....");

	hge->Effect_PlayEx(XO_Music, 100, 0, 1.0, true);

	hge->System_SetState(HGE_INIFILE, "XO_Game\\XO_settings.ini");
	rtBoxStatus.Set(nScreen_Width/2 - BOXSTATUS_WIDTH/2, 0, nScreen_Width/2 + BOXSTATUS_WIDTH/2, 0 + BOXSTATUS_HEIGHT);
	rtStatusArea.Set(rtBoxStatus.x1 - BOXSTATUS_BORDER, rtBoxStatus.y1 - BOXSTATUS_BORDER, rtBoxStatus.x2 + BOXSTATUS_BORDER, rtBoxStatus.y2 + BOXSTATUS_BORDER);

	colBolder.SetHWColor(0xFFFFFFFF);
	colBackground.SetHWColor(0xFFFFFFFF);
	colPlayArea.SetHWColor(0xFFFFFFFF);
	colBorder.SetHWColor(0xFFFFFFFF);

	// Loading X-O-Empty boxes, etc
	PreLoadBitmap(&sprBoxO, "Data_Play", "O_BoxData");
	PreLoadBitmap(&sprBoxX, "Data_Play", "X_BoxData");
	PreLoadBitmap(&sprBoxEmpty, "Data_Play", "Empty_BoxData");	
	PreLoadBitmap(&sprObstacleBox, "Data_Play", "Obstacle_BoxData");	


	// Registering all loaded boxes
	sprBoxList[XO_X_BOX] = sprBoxX;
	sprBoxList[XO_O_BOX] = sprBoxO;	
	sprBoxList[XO_EMPTY_BOX] = sprBoxEmpty;	
	sprBoxList[XO_OBSTACLE_BOX] = sprObstacleBox;	


	if(sprBoxO->GetWidth() != sprBoxO->GetHeight())
		FatalAppExitA(0, "The data for O Box must be a perfect square... Please check and try again");
	

	if(sprBoxX->GetWidth() != sprBoxX->GetHeight())
		FatalAppExitA(0, "The data for X Box must be a perfect square... Please check and try again");
	

	if(sprBoxEmpty->GetWidth() != sprBoxEmpty->GetHeight())
		FatalAppExitA(0, "The data for Empty Box must be a perfect square... Please check and try again");
	

	// Loading background
	LoadBitmapDouble(&sprBackground, &sprMiniBackground, "Data_Play", "MainBackground", "MiniBackground");

	// Loading border
	LoadBitmapDouble(&sprBorderLine, &sprMiniBorderLine, "Data_Play", "MainBorderLine", "MiniBorderLine");

	// Loading play area (texture)
	LoadBitmapDouble(&sprPlayArea, &sprMiniPlayArea, "Data_Play", "MainPlayArea", "MiniPlayArea");

	// Loading cursor
	tex = hge->Texture_Load("XO_Game\\cursor_01.png");
	if(tex == 0)
	{
		hge->System_SetState(HGE_HIDEMOUSE, false);
	}
	else
	{
		hge->System_SetState(HGE_HIDEMOUSE, true);
		sprCursor = new hgeSprite(tex, 0, 0, 32, 32);
	}

	if(bGameDebug)
	MessageBoxA(0, "AfterInitialize() has been completed", "Msg", 0);
}

void PreInitializeSettings() // load HGE Ini and stuff
{
	strcpy(strWorkingIniFile, "XO_Game\\XO_Settings.ini");
	hge->System_SetState(HGE_INIFILE, strWorkingIniFile);

	nBorder_Thickness = hge->Ini_GetFloat("Game_Settings", "Border_Thickness", -1);
	nScreen_Width = hge->Ini_GetFloat("Game_Settings", "Screen_Width", -1);
	nScreen_Height = hge->Ini_GetFloat("Game_Settings", "Screen_Height", -1);
	nBoxSize = hge->Ini_GetFloat("Game_Settings", "Box_Size", -1);
	fBoxesDistance = hge->Ini_GetFloat("Game_Settings", "Boxes_Distance", -1);
	nComboMax = hge->Ini_GetFloat("Game_Settings", "nComboBox", 5);
if(nComboMax < 3)nComboMax = 3;
if(nComboMax > 9)nComboMax = 9;

}




void PreInitialize()
{
	bool bHasChanges = false;
	hge->Random_Seed();

	// Clean XY_Table
	ZeroMemory(XY_Map, sizeof(XY_Map));

	// Initialize the Box sprite list
	memset(sprBoxList, 0, sizeof(sprBoxList));


	// Random obstacles
	int i, j;
	for(i = 0; i < NUMBER_BOXES_EACH; ++i)
	for(j = 0; j < NUMBER_BOXES_EACH; ++j)
	if((hge->Random_Int(0, 100)) >= 95)XY_Map[i][j] = XO_OBSTACLE_BOX;


	PreInitializeSettings();

	while (1)
	{
		if((nBorder_Thickness < 4) || (nBorder_Thickness > 40))
		{
			MessageBoxA(0, "nBorder_Thickness is out of range!", "Msg", 0);
			nBorder_Thickness = 20; bHasChanges = true; continue;
		}

		if((fBoxesDistance < 0.0f) || (fBoxesDistance > 10.0f))
		{
			MessageBoxA(0, "nBorder_Thickness is out of range!", "Msg", 0);
			fBoxesDistance = 2.0f; bHasChanges = true; continue;
		}

		if((int(floor(fBoxesDistance)) % 2) != 0)
		{
			MessageBoxA(0, "nBorder_Thickness must be divideable by 2", "Msg", 0);
			fBoxesDistance = 2.0f; bHasChanges = true; continue;
		}

		nPosBorderLeft = 0 + BORDER_LEFT;
		nPosBorderRight = nScreen_Width - BORDER_RIGHT;
		nPosBorderTop = 0 + BORDER_TOP;
		nPosBorderBottom = nScreen_Height - BORDER_BOTTOM;
		
		rtPlayArea.Set(
				nPosBorderLeft + nBorder_Thickness, nPosBorderTop + nBorder_Thickness, 
				nPosBorderRight - nBorder_Thickness, nPosBorderBottom - nBorder_Thickness
				);
		
		if((Rect_Length(rtPlayArea) < 120) ||(Rect_Height(rtPlayArea) < 120))
		{
			MessageBoxA(0, "Screen Width or Screen Height is too low!", "Msg", 0);
			nScreen_Width = 600; nScreen_Height = 600; bHasChanges = true; continue;
		}

		if((Rect_Length(rtPlayArea) > 1400) ||(Rect_Height(rtPlayArea) > 700))
		{
			MessageBoxA(0, "Screen Width or Screen Height is too high!", "Msg", 0);
			nScreen_Width = 600; nScreen_Height = 600; bHasChanges = true; continue;
		}

		if((nBoxSize > 100.0f) || (nBoxSize < 20.0f))
		{
			MessageBoxA(0, "Box size is out of range!", "Msg", 0);
			nBoxSize = 30.0f; bHasChanges = true; continue;
		}
		break;
	}

	if(bHasChanges == true)
	{
		MessageBoxA(0, "The game will be re-updating all information. All pieces of information that don't work will be reset", "Msg", 0);
		UpdateInfo();
	}

	if(bGameDebug)
	MessageBoxA(0, "PreInitialize() has been completed", "Msg", 0);
}





// Not yet used, but it will be very useful in the future
struct  boxEffectStruct
{
	int x, y;
	long int Ex[32]; double fEx[32];
	int nEffectNum;
	
	void initialize(int _y, int _x, int _nEffectNum)
	{
		x = _x; y = _y; nEffectNum = _nEffectNum;

		switch(nEffectNum)
		{
			case EFFECT_SELECTED : /*The box will have a yellow color when selected*/ break;
			case EFFECT_WINNING : FatalAppExitA(0, "A box derises to have a winning effect!!! Hooray!!!"); break;
		}
	}

	bool update(float dt) // return true if it is no longer needed and needs to be removed, and false otherwise
	{
		// TODO : add your code here
		return false;
	}
};

bool Bingo_Checkmate(int nY, int nX) // returns bool
{
	bool bFound;
	int nCombo;
	int i, x, y, dx, dy, nBoxType = XY_Map[nY][nX];

	// Check all possible directions (8 possible directions)
	int boxDirection[] = {-1, -1,   -1, 0,   -1, 1,   0, 1,   1, 1,   1, 0,   1, -1,    0, -1,   0, 0};
	int DirectionId = 0;
	
	for(i = 0;i < 8;++i)
	{
		bFound = false;
		dy = boxDirection[DirectionId];
		dx = boxDirection[DirectionId + 1];

		DirectionId += 2;

		y = nY + dy;
		x = nX + dx;

		while(XY_Map[y][x] == nBoxType)
		{
			bFound = true;
			y += dy;
			x += dx;
		}

		if(bFound == true)
		{
			nCombo = 0; y -= dy; x -= dx;
			while(XY_Map[y][x] == nBoxType)
			{
				++nCombo; y -= dy; x -= dx;
			}
				
			if(nCombo >= nComboMax)
			{
				y += dy; x += dx; 

				// TODO : Add effects to the boxes that are marked as "Win"
				printf("A Player won. You should add effects to the boxes that are marked as \"Win\"\n\n");

				return true;
			}
		}
		
	}	
	return false;

}


bool FrameFunc()
{
	float x, y;
	int nSelectedBoxX = 0;
	int nSelectedBoxY = 0;

	hge->Input_GetMousePos(&x, &y);

	// Handle up, down, left, right
	if (hge->Input_KeyDown(HGEK_LEFT))
	{
		if(--nViewY < 0)nViewY = 0;
	}
	if (hge->Input_KeyDown(HGEK_RIGHT))
	{
		int numCol = (rtPlayArea.x2 - rtPlayArea.x1) / nBoxSize;
		if((nViewY + numCol) < NUMBER_BOXES_EACH)++nViewY;
	}
	if (hge->Input_KeyDown(HGEK_UP))
	{
		if(--nViewX < 0)nViewX = 0;
	}
	if (hge->Input_KeyDown(HGEK_DOWN))
	{
		int numRow = (rtPlayArea.y2 - rtPlayArea.y1) / nBoxSize;
		if((nViewX + numRow) < NUMBER_BOXES_EACH)++nViewX;
	}

	// Handle left mouse button
	if (hge->Input_KeyDown(HGEK_LBUTTON))
	{
		if(rtPlayArea.TestPoint(x, y))
		{
			if(GetSelectedBox(x, y, &nSelectedBoxX, &nSelectedBoxY))
			{
				if(XY_Map[nViewX + nSelectedBoxY][nViewY + nSelectedBoxX] == XO_EMPTY_BOX)
				{
					hge->Effect_Play(XO_Click);

					if(bXTurn == false) // It is 'O' turn
					{
						XY_Map[nViewX + nSelectedBoxY][nViewY + nSelectedBoxX] = XO_O_BOX;
					}
					else
					{
						XY_Map[nViewX + nSelectedBoxY][nViewY + nSelectedBoxX] = XO_X_BOX;
					}

					if(Bingo_Checkmate(nViewX + nSelectedBoxY, nViewY + nSelectedBoxX))
					{
						// TODO : What should you do if a player wins???
						bXO_Win = true;
					}
					else
					bXTurn = (bXTurn == false);
				}
			}
		}
	}

	if (hge->Input_GetKeyState(HGEK_ESCAPE)) return true;

	return false;
}

void CleanUp()
{
	// TODO : Add your code here
}


bool ExitFunc()
{
	// To do : Add your own code here
	MessageBoxA(0, "Thanks you for visiting and playing the game!", "Exiting...", 0);

	return true;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	hge = hgeCreate(HGE_VERSION); 

	hge->System_SetState(HGE_FRAMEFUNC, FrameFunc);
	hge->System_SetState(HGE_RENDERFUNC, RenderFunc);
	hge->System_SetState(HGE_EXITFUNC, ExitFunc);

	hge->System_SetState(HGE_SCREENWIDTH, nScreen_Width);
	hge->System_SetState(HGE_SCREENHEIGHT, nScreen_Height);

	hge->System_SetState(HGE_TITLE, "HGE Game - XO Game (Yakovich Nikiforov)");
	hge->System_SetState(HGE_WINDOWED, true);
	// hge->System_SetState(HGE_USESOUND, true);

	// Initialize the program before running the window
	PreInitialize();
	if(hge->System_Initiate())
	{

		// Initialize the program after the main window is loaded
		AfterInitialize();
		hge->System_Start();
	}
	else
	{	
		MessageBoxA(NULL, hge->System_GetErrorMessage(), "Error", MB_OK | MB_ICONERROR | MB_APPLMODAL); 
	}

	hge->System_Shutdown();
	hge->Release();
	return 0;
}

