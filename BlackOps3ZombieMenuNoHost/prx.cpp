#include "stdafx.h"
#include "PS3.h"
#include "Lib.h"
#include "Var.h"
#include <cellstatus.h>
#include <sysutil\sysutil_msgdialog.h>
#include <sysutil\sysutil_oskdialog.h>
#include <sysutil\sysutil_oskdialog_ext.h>
//#include "Auth.h"

typedef long long __int64;
float BlueLight[4] = { 0.024, 0.251, 0.745, 1 };
float BlackLowAlpha[4] = { 0, 0, 0, 0.6};
float BlackMedAlpha[4] = { 0, 0, 0, 0.7 };
float je_sais_pas[4] = { 15, 214, 0, 0.6 };
float lol[3] = { 0, 250, 154 };
int maxOptions = 40;
bool OpenOption = false;
int submenuLevel;
int lastSubmenu[20];
int lastOption[20];
bool isOpen, isLoaded, print, storeText, isInGame;
int optionCount;
int submenu = 0;
bool uavac;
int CALL(int numOfArgs, int addr, ...)
{
  int i,val;
  va_list vl;
  va_start(vl,numOfArgs);
  
    va_arg(vl,int);//skip one

  for (i=1;i<(numOfArgs+1);i++)
  {
    val=va_arg(vl,int);
	*(int*)(0x2100000+i*4) = val;
  }
  va_end(vl);
  
  sleep(50);

  *(int*)0x2100000 = addr;
  return *(int*)(0x210004C);

}



void Calls_thread(std::uint64_t nothing)
{
	for(;;)
	{
		
	__asm(
		"lis 3, 0x210;"
	    "lwz 3, 0(3);"
	    "cmpwi 3, 0x00;"
	    "beq end;"


	    /*save the general registers !*/
		"mflr 0;"
		"std 0, -0x520-0x10(1);"
		"std 31, -0x520-0x18(1);"
		"std 30, -0x520-0x20(1);"
		"std 29, -0x520-0x28(1);"
		"std 28, -0x520-0x30(1);"
		"std 27, -0x520-0x38(1);"
		"std 26, -0x520-0x40(1);"
		"std 2, -0x520-0x48(1);"
		
		/*emulate the Ghosts POC registers*/
		"lis 2, 0xA7;"
		"ori 2, 2, 0xF3F8;"   
		"lis 26, 0;"
		"lis 27, 0;"
		"li 28, 0x0C800;"
		"lis 29, 0;"
		"lis 30, 0xD00D;"
		"addic 30,30, 0x9DF0;"
		"lis 31, 0x157;"
		"addic 31,31, 0x059C;"
		
	/*Loading args*/
	"lis 12, 0x210;"
	"lwz 3, 0x4(12);"
	"lwz 4, 0x8(12);"
	"lwz 5, 0xC(12);"
	"lwz 6, 0x10(12);"
	"lwz 7, 0x14(12);"
	"lwz 8, 0x18(12);"
	"lwz 9, 0x1C(12);"
	"lwz 10, 0x20(12);"
	"lwz 11, 0x24(12);"

	"lfs 1, 0x28(12);"
	"lfs 2, 0x2C(12);"
	"lfs 3, 0x30(12);"
	"lfs 4, 0x34(12);"
	"lfs 5, 0x38(12);"
	"lfs 6, 0x3C(12);"	
	"lfs 7, 0x40(12);"
	"lfs 8, 0x44(12);"
	"lfs 9, 0x48(12);"

	/*Calling*/
	"lwz 12, 0(12);"
	"mtctr 12;"
	"bctrl;"
	
	/*returns*/
	"lis 4, 0x210;"
	"stw 3, 0x4C(4);"
	"stfs 1, 0x50(4);"
	"li 3, 0;"
	"stw 3, 0(4);"
		  

	"ld 0, -0x520-0x10(1);"
	"mtlr 0;"
	"ld 26, -0x520-0x40(1);"
	"ld 27, -0x520-0x38(1);"
	"ld 28, -0x520-0x30(1);"
	"ld 29, -0x520-0x28(1);"
	"ld 30, -0x520-0x20(1);"
	"ld 31, -0x520-0x18(1);"
	"ld 2, -0x520-0x48(1);"
		  );
		 __asm("end:;");
		sleep(5);
	}
	
}
enum Submenus
{
	Closed,
	Main_Menu,
	PlayerOptions,
	OnlinePlayers,
	AllPlayers,
	Player_Options,
	MoreOptionsOnline,
	Weapons1,
	Weapons2,
	CamoSub,
	Teleports_Evac,
	TeleportsAquarium,
	TeleportsExodus,
	TeleportsHunted,
	TeleportsHavoc,
	TeleportsMetro,
	TeleportsRedwood,
	TeleportsCombine,
	TeleportsBreach,
	Round,
	NoHost,
	Stats,
	Settings,
	Color_Title,
	AllClients,
	Shadow,
	Der,
	Shadow2,
	Der2,
	Shadowclients,
	Der2clients,
	Clients,
	weapons2clients,
	Shadow2clients,
	Der2weapons,
	weaponsac,
	shadowsac,
	derac,
	Special1,
	Equipement1,
	specialists1,
	sub_machine_gun1,
	assault_rifle1,
	light_machine_gun1,
	snipers1,
	shotgun1,
	pistols,
	launcher1,
	killstreaks1,
	Specialclient,
	Equipementclient,
	specialistsclient,
	sub_machine_gunclient,
	assault_rifleclient,
	light_machine_gunclient,
	snipersclient,
	shotgunclient,
	pistolsclient,
	launcherclient,
	killstreaksclient,
	weaponsclient,
	camoself,
	visionself,
};
bool squarePress = false;
bool fastLeftPress = false;
bool leftPress = false;
bool fastRightPress = false;
bool rightPress = false;
int currentOption;
void OpenMenu()
{
	//Show Huds
	Purple[3] = 1;
	White[3] = 1;
	Black[3] = .7;
	//Slide Animation
	for(float i = -200; i <= 1 ;i+= 5)
    {
		Defx = i;
		sleep(5);
	}
	//Open Print
	Print("^6Menu ^2Open");
	//Enables Menu Controls
	Open = true;
	sleep(50);
}
void CloseMenu()
{
	//Slide Animation
	for(float i = 1; i >= -200;i-= 5)
    {
		Defx = i;
		sleep(5);
	}
	//Hide Huds
	Purple[3] = 0;
	White[3] = 0;
	Black[3] = 0;
	//Close Print
	Print("^5Menu ^1Closed");
	//Disable & Reset Menu Controls
	Open = false;
	ScrollIndex = 0;
	ScrollPos = 160;
	sleep(50);
}
void ScrollDown()
{
	if (ScrollIndex == 19)//5 = max
	{
	   ScrollIndex = 0;
	}
	else
	{
		ScrollIndex = ScrollIndex + 1;  
	}
	ScrollPos = (ScrollIndex * 30) + ScrollDefault;
	sleep(50);
}
void ScrollUp()
{
	if (ScrollIndex == 0)
	{
		ScrollIndex = 13;//5 = max
	}
	else
	{
		ScrollIndex = ScrollIndex - 1;  
	}
	ScrollPos = (ScrollIndex * 30) + ScrollDefault;
	sleep(50);
}
void SelectOption()
{
	
}

void my_dialog2(int button, void *userdata)
{
      switch(button)
      {
       case CELL_MSGDIALOG_BUTTON_OK:
       case CELL_MSGDIALOG_BUTTON_NONE:
       case CELL_MSGDIALOG_BUTTON_ESCAPE:
       break;
       default:
	   break;
	  }
}

void Menu(uint64_t)
{
	sleep(20000);
	for (;;)
	{
		sleep(60);
		if (isInGame)
			{
				if (submenu == Closed)
				{
					if (ButtonMonitor::Pressed(ButtonMonitor::R3))
					{
						submenu = Main_Menu;
						submenuLevel = 0;
						currentOption = 1;
						Open = true;
						OpenMenu();
						sleep(100);
					}
				}
				else {
					if (ButtonMonitor::Pressed(ButtonMonitor::R3))
					{
						if (submenu == Main_Menu)
						{
							submenu = Closed;
							Open = false;
							CloseMenu();
						}
						else {
							submenu = lastSubmenu[submenuLevel - 1];
							currentOption = lastOption[submenuLevel - 1];
							submenuLevel--;

						}
						sleep(100);
					}
					else if (ButtonMonitor::Pressed(ButtonMonitor::Cross))
					{
						OpenOption = true;
						sleep(100);
					}
					else if (ButtonMonitor::Pressed(ButtonMonitor::Up))
					{
						currentOption--;
						if (currentOption < 1)
						{
							currentOption = optionCount;
						}
						sleep(100);
					}
					else if (ButtonMonitor::Pressed(ButtonMonitor::Down))
					{
						currentOption++;
						if (currentOption > optionCount)
						{
							currentOption = 1;
						}
						ScrollDown();
						sleep(100);

					}
					
					sleep(30);
				}
			}
		//}
		else submenu = Closed;
	}
}


__int64 hookStub(int timeID){
	__asm("li %r3, 0x3");
}
void addTitle(char *title)
{
	//drawText("^0You Are Host", 1050, 50, "fonts/smallFont", 1.10, BlueLight);//if Host
	//DrawText("^3Controls : \n^1Open : R3\n^2Scroll Down / Up : Up / Down Arrow\n^5Validate : Square", 410, 980, "fonts/consoleFont", 1.15, BlueLight);//Credits Menu
	DrawText("^5Spectre Engine\n^2Version : 1.00\n\n", 410, 150, "fonts/boldFont",1.25f,White);//Title Menu
	DrawShader(400, 70, 450, 20, je_sais_pas,"white");//High
	DrawShader(400, 90, 450, 930, Black,"white");//Background
	DrawShader(405, 200, 440, 80, je_sais_pas,"white");//High
	DrawShader(405, 940, 440, 80, je_sais_pas,"white");//Downs
	DrawShader(400, 90, 5, 930, je_sais_pas,"white");//Left
	DrawShader(845, 90, 5, 930, je_sais_pas,"white");//Right
	//DrawShader(400, 0, 555, 200, lol, "white");//Title Shader
}
char *infoText;
void AddOption(char *option, char *info)
{
	optionCount++;
	if (currentOption == optionCount)
		infoText = info;
	if (currentOption <= maxOptions && optionCount <= maxOptions)
		DrawText(option, 410, 310 + (optionCount * 35), "fonts/consoleFont", 1.20, je_sais_pas);
	else if ((optionCount > (currentOption - maxOptions)) && optionCount <= currentOption)
		DrawText(option, 410, (((currentOption - maxOptions)), 310 + (optionCount * 35)), "fonts/consoleFont", 1.20, White);
}
void addOption(char * Option, char* null)
{

	AddOption(Option, null);
	if (optionCount == currentOption)
	{

	}
	else DrawText(Option, 410, 310 + (optionCount * 35), "fonts/consoleFont", 1.20, White);


}
int changeSubmenu(int newSubmenu)
{
	lastSubmenu[submenuLevel] = submenu;
	lastOption[submenuLevel] = currentOption;
	currentOption = 1;
	submenu = newSubmenu;
	submenuLevel++;
}
void addSubmenuOption(char *option, int newSubmenu, char *info)
{
		addOption(option, info);
		if (currentOption == optionCount && OpenOption)
			changeSubmenu(newSubmenu);
}
void Toggle(char* Stat)
{

	if (currentOption <= maxOptions && optionCount <= maxOptions)
		DrawText(Stat, 720, 350 + (optionCount * 35), "fonts/consoleFont", 1.20, White);
	else if ((optionCount > (currentOption - maxOptions)) && optionCount <= currentOption)
		DrawText(Stat, 720, (((currentOption - maxOptions)), 350 + (optionCount * 35)), "fonts/consoleFont", 1.20, White);
}
int getOption()
{
	if (OpenOption)
	{
		return currentOption;
	}
	else return 0;
}
void addBoolOption(char *option, bool b00l, char *info)
{
	char buf[50];
	if (b00l)
	{
		Toggle("^5Enabled");
		addOption(option, info);
	}
	else
	{
		Toggle("^1Disabled");
		addOption(option, info);
	}
}
bool GodMode;
bool Speedx2;
bool FakeLag;
bool Freeze;
bool hud;
bool spectator;
bool Third;
bool invisibleself;

bool forcehost;
bool GodModeAC;
bool Speedx2AC;
bool FakeLagAC;
bool FreezeAC;
bool jetpackAC;
bool ThirdAC;
bool AmmoAC;
bool perks;
bool killstreakAC;
bool specac;
bool InvisibleAC;
char* GetName(int ClientNum)
	 {
	  char* Name = ReadString(static_cast<unsigned int>(0x18CBFD0 + (0x6200 * ClientNum)));
	  char* Name2 = ReadString(static_cast<unsigned int>(0x18CBFD0 + (0x6200 * ClientNum)));
	  if (!strcmp(Name, "") && !strcmp(Name2, ""))
	  {
		  return "Not Connected";
	  }
	  else
	  {
		  return Name;
	  }
		 return;

	 }
  char* GetClientName(int Client)
  {
	  char* Names[4];
	  Names[Client] = ReadString((0x18CBFD0 + (0x6200 * Client)));
	  if (Names[Client] == "")
	  {
		  Names[Client] = "Not Connected";
	  }
	  return Names[Client];
  }
int G_Client(int client)
{
	return 0x18C6220 + (client * 0x6200);
}

/*
Prestige : 0x37DDE5D8
Level : 0x37DDE57A
*/
  int selectedPlayer;

  __int64 ForceHost(int TimeId)
{
  cBuf_AddText(-1, "partyMigrate_disabled 1");
                         cBuf_AddText(-1, "party_minplayers 1");
                         cBuf_AddText(-1, "allowAllNAT 1");
                         cBuf_AddText(-1, "party_connectToOthers 0");
						 cBuf_AddText(-1, "party_mergingEnabled 0");
						 cBuf_AddText(-1, "xpartygo");
						 cBuf_AddText(-1, "party_iAmHost 1");
  }
__int64 Hook(int TimeId)
{
		*(int*)0x61A154 = 0x60000000;
	    *(int*)0x622DA4 = 0x60000000;
	    *(int*)0x622DCC = 0x38600000;
		*(int*)0x7C4148 = 0x60000000;
	    *(int*)0x7C4050 = 0x60000000;
		*(int*)0x7C4758 = 0x60000000;
	    *(int*)0x7C4660 = 0x60000000;
		//DrawText("^5Spectre Engine Menu\n^3Indie Developer and ^4MrNiato", 900, 100, "fonts/smallFont", 1.14, BlueLight);//Credits Menu
	if (InGame() == true)
	{
			isInGame = true;
			optionCount = 0;
			switch (submenu)
			{
#pragma region Main Menu
			case Main_Menu:
				addTitle("^1Spectre Engine");
				addSubmenuOption("Self Menu", PlayerOptions, NULL);
				addSubmenuOption("Weapons Menu", Weapons1, NULL);
				addSubmenuOption("Lobby Editor", CamoSub, NULL);
				addSubmenuOption("Stats Menu", Stats, NULL);
				addSubmenuOption("Clients Menu", Clients, NULL);
				addSubmenuOption("All Clients Menu", AllClients, NULL);
				break;
#pragma endregion
				//All Perks : 018C4954
#pragma region Player Options
			case PlayerOptions:
				addTitle("^2Self Menu");
				addBoolOption("God Mode", GodMode, NULL);
				addBoolOption("Unlimited Ammo", Ammo, NULL);
				addBoolOption("Invisible", invisibleself, NULL);
				addBoolOption("Speed x2", Speedx2, NULL);
				addBoolOption("Fake Lag", FakeLag, NULL);
				addBoolOption("Freeze", Freeze, NULL);
				addBoolOption("Third Person", Third, NULL);
				addBoolOption("Uav", notarget, NULL);
				addBoolOption("Remove HUD", hud, NULL);
				addBoolOption("All Perks", perks, NULL);
				addBoolOption("Spectator Mode", spectator, NULL);
				addBoolOption("Force Host", forcehost, NULL);
				addOption("Camos Menu", NULL);
				switch (getOption())
				{
				case 1:
					GodMode = !GodMode;
					if (GodMode == true)
					{
						*(int*)(G_Client(0) + 0x23) = 0x05000000;	
						//cBuf_AddText(0, "player_sustainAmmo 1");
					}
					else if (GodMode == false)
					{
						*(int*)(G_Client(0) + 0x23) = 0x04000000;
					}
					break;
				case 2:
					Ammo = !Ammo;
					if (Ammo == true)
					{
						*(int*)(G_Client(0) + 0x548) = 0x7fffffff;
						*(int*)(G_Client(0) + 0x54C) = 0x7fffffff;
						*(int*)(G_Client(0) + 0x584) = 0x7fffffff;
						*(int*)(G_Client(0) + 0x588) = 0x7fffffff;
						*(int*)(G_Client(0) + 0x593) = 0x7fffffff;			
						*(int*)(G_Client(0) + 0x58F) = 0x7fffffff;	
					}
					else if (Ammo == false)
					{
						*(int*)(G_Client(0) + 0x548) = 0x00000000;
						*(int*)(G_Client(0) + 0x54C) = 0x00000000;
						*(int*)(G_Client(0) + 0x584) = 0x00000000;
						*(int*)(G_Client(0) + 0x588) = 0x00000000;
						*(int*)(G_Client(0) + 0x593) = 0x00000000;			
						*(int*)(G_Client(0) + 0x58F) = 0x00000000;	
					}
					break;
					case 3:
					invisibleself = !invisibleself;
					if (invisibleself == true)
					{
						*(int*)(G_Client(0) + 0x65a) = 0x01000000;
					}
					else if (invisibleself == false)
					{
						*(int*)(G_Client(0) + 0x65a) = 0x00000000;	
					}
					break;
					case 4:
				    Speedx2 = !Speedx2;
					if (Speedx2 == true)
					{
						*(int*)(G_Client(0) + 0x5DF8) = 0x3fff0000;				
					}
					else if (Speedx2 == false)
					{
						*(int*)(G_Client(0) + 0x5DF8) = 0x3f800000;		
					}
					break;
					case 5:
					FakeLag = !FakeLag;
					if (FakeLag == true)
					{
						*(int*)(G_Client(0) + 0x5D27) = 0x01000000;				
					}
					else if (FakeLag == false)
					{
						*(int*)(G_Client(0) + 0x5D27) = 0x02000000;
					}
					break;
					case 6:
					Freeze = !Freeze;
					if (Freeze == true)
					{
						*(int*)(G_Client(0) + 0x4) = 0x09000000;				
					}
					else if (Freeze == false)
					{
						*(int*)(G_Client(0) + 0x4) = 0x00000000;
					}
					break;
					case 7:
					Third = !Third;
					if (Third == true)
					{
						*(int*)(G_Client(0) + 0x12d) = 0x01000000;				
					}
					else if (Third == false)
					{
						*(int*)(G_Client(0) + 0x12d) = 0x00000000;		
					}
					break;
					case 8:
					notarget = !notarget;
					if (notarget == true)
					{
						*(int*)(G_Client(0) + 0x5eff) = 0x07000000;				
					}
					else if (notarget == false)
					{
						*(int*)(G_Client(0) + 0x5eff) = 0x03000000;
					}
					break;
					case 9:
					hud = !hud;
					if (hud == true)
					{
						*(int*)(G_Client(0) + 0x5EFF) = 0x00000000;				
					}
					else if (hud == false)
					{
						*(int*)(G_Client(0) + 0x5EFF) = 0x03000000;
					}
					break;
					case 10:
					perks = !perks;
					if (perks == true)
					{
						*(int*)(G_Client(0) + 0x53E) = 0xff;
						*(int*)(G_Client(0) + 0x542) = 0xff;
						*(int*)(G_Client(0) + 0x546) = 0xff;
					}
					else if (perks == false)
					{
						*(int*)(G_Client(0) + 0x53E) = 0x00;
						*(int*)(G_Client(0) + 0x542) = 0x00;
						*(int*)(G_Client(0) + 0x546) = 0x00;	
					}
					break;
					case 11:
					spectator = !spectator;
					if (spectator == true)
					{
						*(int*)(G_Client(0) + 0x23) = 0x07000000;
					}
					else if (spectator == false)
					{
					if (GodMode == true)
					{
						*(int*)(G_Client(0) + 0x23) = 0x05000000;				
					}
					else if (GodMode == false)
					{
						*(int*)(G_Client(0) + 0x23) = 0x04000000;
					}
					}
					break;
					case 12:
					forcehost = !forcehost;
					if (forcehost == true)
					{
						 cBuf_AddText(-1, "partyMigrate_disabled 1");
                         cBuf_AddText(-1, "party_minplayers 1");
                         cBuf_AddText(-1, "allowAllNAT 1");
                         cBuf_AddText(-1, "party_connectToOthers 0");
						 cBuf_AddText(-1, "party_mergingEnabled 0");
						 cBuf_AddText(-1, "party_iAmHost 1");
						 cBuf_AddText(-1, "xpartygo");
						ForceHost(250);
					}
					else if (forcehost == false)
					{
						ForceHost(500000);
						cBuf_AddText(0, "party_minplayers 1");	
						cBuf_AddText(0, "allowAllNAT 1");	
						cBuf_AddText(0, "party_connectToOthers 0");	
						cBuf_AddText(0, "party_maxplayers 10");	
						}
					break;
				case 13:
					changeSubmenu(camoself);
					break;
				}

				break;
				case camoself:
				addTitle("Camo Menu");
				addOption("No Camo", NULL);
				addOption("Jungle", NULL);
				addOption("Flectarn", NULL);
				addOption("Dante", NULL);
				addOption("Policia", NULL);
				addOption("Dark Metter", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x389) = 0x00000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x389) = 0x01000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x389) = 0x03000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x389) = 0x06000000;
					break;
				case 5:
					*(int*)(G_Client(0) + 0x389) = 0x09000000;
					break;
				case 6:
					*(int*)(G_Client(0) + 0x389) = 0x11000000;
					break;
				case 7:
					*(int*)(G_Client(0) + 0x389) = 0x0B000000;
					break;
				}
				break;
#pragma endregion
#pragma region LOL
              case Weapons1:
				addTitle("Weapons Menu");
				addOption("Special", NULL);
				addOption("Equipment", NULL);
				addOption("Specialist", NULL);
				addOption("Sub Machine Guns", NULL);
				addOption("Assault Rifles", NULL);
				addOption("Light Machine Guns", NULL);
				addOption("Snipers", NULL);
				addOption("Shotguns", NULL);
				addOption("Pistols", NULL);
				addOption("Launcher", NULL);
				addOption("Killstreaks", NULL);
				switch (getOption())
				{
				case 1:
					changeSubmenu(Special1);
					break;
				case 2:
					changeSubmenu(Equipement1);
					break;
				case 3:
					changeSubmenu(specialists1);
					break;
				case 4:
					changeSubmenu(sub_machine_gun1);
					break;
			    case 5:
					changeSubmenu(assault_rifle1);
					break;
                case 6:
					changeSubmenu(light_machine_gun1);
					break;
				case 7:
					changeSubmenu(snipers1);
					break;
				case 8:
					changeSubmenu(shotgun1);
					break;
				case 9:
					changeSubmenu(pistols);
					break;
				case 10:
					changeSubmenu(launcher1);
					break;
				case 11:
					changeSubmenu(killstreaks1);
					break;
				}
				break;
				case Special1:
				addTitle("Special Weapons Menu");
				addOption("Default Weapons", NULL);
				addOption("Combat Knife", NULL);
				addOption("Pink Weapons", NULL);
				addOption("Satellite Ball", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x01000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x3B000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x66000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x3F000000;
					break;
				}
				break;
				case Equipement1:
				addTitle("Equipment Weapons Menu");
				addOption("Flash Bangs", NULL);
				addOption("Grenades", NULL);
				addOption("1 Grenade Infinite Concussions", NULL);
				addOption("Weird Stick Thing", NULL);
				addOption("Sticky Grenades", NULL);
				addOption("Smoke Grenades ", NULL);
				addOption("TripMines", NULL);
				addOption("EMP Grenades", NULL);
				addOption("Thermites", NULL);
				addOption("Trophy System ", NULL);
				addOption("Combat Axe", NULL);
				addOption("Shock Charge", NULL);
				addOption("C4", NULL);
				addOption("Blackhat", NULL);
				addOption("Nova Gas", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x41000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x42000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x43000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x44000000;
					break;
				case 5:
					*(int*)(G_Client(0) + 0x383) = 0x45000000;
					break;
                case 6:
					*(int*)(G_Client(0) + 0x383) = 0x46000000;
					break;
				case 7:
					*(int*)(G_Client(0) + 0x383) = 0x48000000;
					break;
				case 8:
					*(int*)(G_Client(0) + 0x383) = 0x49000000;
					break;
				case 9:
					*(int*)(G_Client(0) + 0x383) = 0x4B000000;
					break;
				case 10:
					*(int*)(G_Client(0) + 0x383) = 0x4C000000;
					break;
				case 11:
					*(int*)(G_Client(0) + 0x383) = 0x4E000000;
					break;
                case 12:
					*(int*)(G_Client(0) + 0x383) = 0x4F000000;
					break;
				case 13://
					*(int*)(G_Client(0) + 0x383) = 0x50000000;
					break;
				case 14:
					*(int*)(G_Client(0) + 0x383) = 0x53000000;
					break;
				case 15:
					*(int*)(G_Client(0) + 0x383) = 0x58000000;
					break;
				}
				break;
#pragma endregion
#pragma region Map Changer
				case specialists1:
				//0x18C4643
				addTitle("Specialists Weapons Menu");
				addOption("Tempest", NULL);
				addOption("Gravity Spikes", NULL);
				addOption("Ripper", NULL);
				addOption("Anihilator", NULL);
				addOption("War Machine ", NULL);
				addOption("Bow ", NULL);
				addOption("Hive", NULL);
				addOption("FlameThrower", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x5A000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x5B000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x5C000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x5D000000;
					break;
			    case 5:
					*(int*)(G_Client(0) + 0x383) = 0x5E000000;
					break;
                case 6:
					*(int*)(G_Client(0) + 0x383) = 0x60000000;
					break;
				case 7:
					*(int*)(G_Client(0) + 0x383) = 0x64000000;
					break;
				case 8:
					*(int*)(G_Client(0) + 0x383) = 0x65000000;
					break;
				}
				break;
				case sub_machine_gun1:
				addTitle("Machine Gun Weapons Menu");
				addOption("Kuda", NULL);
				addOption("VMP", NULL);
				addOption("Weevil", NULL);
				addOption("Vesper", NULL);
				addOption("Pharo", NULL);
				addOption("Razorback", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x02000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x04000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x06000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x08000000;
					break;
				case 5:
					*(int*)(G_Client(0) + 0x383) = 0x0A000000;
					break;
				case 6:
					*(int*)(G_Client(0) + 0x383) = 0x0C000000;
					break;
				}
				break;
				case assault_rifle1:
				addTitle("Assault Weapons Menu");
				addOption("KN44", NULL);
				addOption("XR2", NULL);
				addOption("HVK30", NULL);
				addOption("ICR1", NULL);
				addOption("Man-O-War", NULL);
				addOption("Shieva", NULL);
			    addOption("M8A7", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x0E000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x10000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x12000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x14000000;
					break;
				case 5:
					*(int*)(G_Client(0) + 0x383) = 0x16000000;
					break;
				case 6:
					*(int*)(G_Client(0) + 0x383) = 0x18000000;
					break;
				case 7:
					*(int*)(G_Client(0) + 0x383) = 0x1A000000;
					break;
				}
				break;
				case light_machine_gun1:
				addTitle("Light Weapons Menu");
				addOption("BRM", NULL);
				addOption("Dingo", NULL);
				addOption("Gorgon", NULL);
				addOption("Dredge", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x20000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x22000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x24000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x26000000;
					break;
				}
				break;
				case snipers1:
				addTitle("Snipers Weapons Menu");
				addOption("Drakon", NULL);
				addOption("Locus", NULL);
				addOption("P06", NULL);
				addOption("SVG100", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x28000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x2A000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x2C000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x2E000000;
					break;
				}
				break;
				case shotgun1:
				addTitle("Shotgun Weapons Menu");
				addOption("KRM", NULL);
				addOption("Brecci", NULL);
				addOption("Haymaker", NULL);
				addOption("Argus", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x1C000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x1D000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x1E000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x1F000000;
					break;
				}
				break;
				case pistols:
				addTitle("Weapons Menu");
				addOption("MR6", NULL);
				addOption("RK5", NULL);
				addOption("LCar9", NULL);		
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x30000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x35000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x38000000;
					break;
				
				}
				break;
				case launcher1:
				addTitle("Launcher Weapons Menu");
				addOption("Blackcell", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x3A000000;
					break;
				
				}
				break;
				case killstreaks1:
				addTitle("Killstreaks Weapons Menu");
				addOption("HCXD", NULL);
				addOption("Bullet Turret", NULL);
				addOption("Missile Turret", NULL);
				addOption("Missile Turret Slow", NULL);
				addOption("Mothership", NULL);
				addOption("Bullet Turret Fast", NULL);
				addOption("Spawns_Talon", NULL);
				addOption("Hellstorm", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x383) = 0x6B000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x383) = 0x6C000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x383) = 0x6D000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x383) = 0x6F000000;
					break;
				case 5:
					*(int*)(G_Client(0) + 0x383) = 0x70000000;
					break;
				case 6:
					*(int*)(G_Client(0) + 0x383) = 0x75000000;
					break;
				case 7:
					*(int*)(G_Client(0) + 0x383) = 0x76000000;
					break;
				case 8:
					*(int*)(G_Client(0) + 0x383) = 0x78000000;
					break;
				}
				break;
#pragma endregion
#pragma region Round
			case CamoSub:
				addTitle("Lobby Editor Menu");
				addOption("Restart Match", NULL);	
				switch (getOption())
				{
				case 1:
					CALL(0, 0x5B1A7C, "map_restart");
					break;
				}

				break;
#pragma endregion
#pragma region Settings
			case Stats:
				addTitle("Stats Menu");
				addOption("Prestige 1", NULL);
				addOption("Prestige 2", NULL);
				addOption("Prestige 3", NULL);
				addOption("Prestige 4", NULL);
				addOption("Prestige 5", NULL);
				addOption("Prestige 6", NULL);
				addOption("Prestige 7", NULL);
				addOption("Prestige 8", NULL);
				addOption("Prestige 9", NULL);
				addOption("Prestige 10", NULL);
				addOption("Prestige 11", NULL);
				addOption("Level 55", NULL);
				addOption("Unlock 10 Class", NULL);
				addOption("Modded Stats", NULL);
				addOption("Unlock All Weap + Calling Card", NULL);
				switch (getOption())
				{
				case 1:
					char Prestige1[] = { 0x01 };
					WriteBytes(0x37EF8E45, Prestige1, sizeof(Prestige1));
					break;
				case 2:
					char Prestige2[] = { 0x02 };
					WriteBytes(0x37EF8E45, Prestige2, sizeof(Prestige2));
					break;
				case 3:
					char Prestige3[] = { 0x03 };
					WriteBytes(0x37EF8E45, Prestige3, sizeof(Prestige3));
					break;
				case 4:
					char Prestige4[] = { 0x04 };
					WriteBytes(0x37EF8E45, Prestige4, sizeof(Prestige4));
					break;
				case 5:
					char Prestige5[] = { 0x05 };
					WriteBytes(0x37EF8E45, Prestige5, sizeof(Prestige5));
					break;
				case 6:
					char Prestige6[] = { 0x06 };
					WriteBytes(0x37EF8E45, Prestige6, sizeof(Prestige6));
					break;
				case 7:
					char Prestige7[] = { 0x07 };
					WriteBytes(0x37EF8E45, Prestige7, sizeof(Prestige7));
					break;
				case 8:
					char Prestige8[] = { 0x08 };
					WriteBytes(0x37EF8E45, Prestige8, sizeof(Prestige8));
					break;
				case 9:
					char Prestige9[] = { 0x09 };
					WriteBytes(0x37EF8E45, Prestige9, sizeof(Prestige9));
					break;
				case 10:
					char Prestige10[] = { 0x0A };
					WriteBytes(0x37EF8E45, Prestige10, sizeof(Prestige10));
					break;
				case 11:
					char Prestige11[] = { 0x0B };
					WriteBytes(0x37EF8E45, Prestige11, sizeof(Prestige11));
					break;
				case 12:
					char Level55[] = { 0xff };
					WriteBytes(0x37EF8E65, Level55, sizeof(Level55));
					break;
				case 13:
					char UnlockClass[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
					WriteBytes(0x37EEC9F8, UnlockClass, sizeof(UnlockClass));
					break;
				case 14:
					char Score[] = { 0xFF, 0xFF };
					char Kills[] = { 0x22, 0x08 };
					char Tokens[] = { 0xFF };
					char Wins[] = { 0x26, 0x25 };
					char Deaths[] = { 0x36, 0x08 };
					char Loses[] = { 0x3C, 0x0B };
					WriteBytes(0x37EF8E87, Score, sizeof(Score));
					WriteBytes(0x37EF86D1, Kills, sizeof(Kills));
					WriteBytes(0x37EF80F7, Tokens, sizeof(Tokens));
					WriteBytes(0x37F3E8DD, Wins, sizeof(Wins));
					WriteBytes(0x37EF828D, Deaths, sizeof(Deaths));
					WriteBytes(0x37EF87AF, Loses, sizeof(Loses));
					break;
				case 15:
					/* Unlock All Weapon */
					char Level[] = { 0xff };
					WriteBytes(0x37EF2984, Level, sizeof(Level)); //Bowie
					/* Assault Weapon */
					WriteBytes(0x37EF0340, Level, sizeof(Level)); //KN-44					
					WriteBytes(0x37EF03BC, Level, sizeof(Level)); //Second Assault
					WriteBytes(0x37EF0438, Level, sizeof(Level)); //Third Assault
					WriteBytes(0x37EF04B4, Level, sizeof(Level)); //4 Assault
					WriteBytes(0x37EF0530, Level, sizeof(Level)); //5 Assault
					WriteBytes(0x37EF05AC, Level, sizeof(Level)); //6 Assault
					WriteBytes(0x37EF0628, Level, sizeof(Level)); //7 Assault
					/* Machine Weapon */
					WriteBytes(0x37EEFE68, Level, sizeof(Level)); //1 Machine Weapon					
					WriteBytes(0x37EEFEE4, Level, sizeof(Level)); //2 Machine Weapon
					WriteBytes(0x37EEFF60, Level, sizeof(Level)); //3 Machine Weapon
					WriteBytes(0x37EEFFDC, Level, sizeof(Level)); //4 Machine Weapon
					WriteBytes(0x37EF0058, Level, sizeof(Level)); //5 Machine Weapon
					WriteBytes(0x37EF00D4, Level, sizeof(Level)); //6 Machine Weapon
					/* Other Weapon */
					WriteBytes(0x37EF0818, Level, sizeof(Level)); //1 					
					WriteBytes(0x37EF0894, Level, sizeof(Level)); //2 
					WriteBytes(0x37EF0910, Level, sizeof(Level)); //3 
					WriteBytes(0x37EF098C, Level, sizeof(Level)); //4 
					/* Snipers*/
					WriteBytes(0x37EF0CF0, Level, sizeof(Level)); //1 					
					WriteBytes(0x37EF0D6C, Level, sizeof(Level)); //2 
					WriteBytes(0x37EF91C8, Level, sizeof(Level)); //3 
					WriteBytes(0x37EF0DE8, Level, sizeof(Level)); //4 
					/* Shutgun Weapon */
					WriteBytes(0x37EF11C8, Level, sizeof(Level)); //1 					
					WriteBytes(0x37EF1244, Level, sizeof(Level)); //2 
					WriteBytes(0x37EF12C0, Level, sizeof(Level)); //3 
					WriteBytes(0x37EF133C, Level, sizeof(Level)); //4 
					/* Gun Weapons */
					WriteBytes(0x37EEFA0C, Level, sizeof(Level)); //1 					
					WriteBytes(0x37EEFA88, Level, sizeof(Level)); //2 
					WriteBytes(0x37EEFB04, Level, sizeof(Level)); //3 
					/* Missile Launcher */
					WriteBytes(0x37EF15A8, Level, sizeof(Level)); //1 
					WriteBytes(0x37EF1624, Level, sizeof(Level)); //2
					/* Unlock 72% Title */
					char Title[] = { 0xff };
					WriteBytes(Entry_Stats + 0xC539, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC53E, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC544, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC54A, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC550, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC556, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC55C, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC562, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC568, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC568, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC56E, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC58C, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC5E1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC74F, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC76E, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC791, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC797, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xC7C6, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc80e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc815, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc856, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc862, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc868, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc8aa, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc8e1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc8e6, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc8fe, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc90a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc916, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc91d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc958, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc965, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc96a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc971, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc977, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc97d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc995, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc9a6, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc9c5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca18, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca1f, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca2a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca31, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca36, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca6d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca8a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca91, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca96, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xca9d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcaa3, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcaa8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcaa9, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcaaf, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcab5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcacd, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcaf1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcb08, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcb15, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcb38, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcb3e, Title, sizeof(Title));
                    WriteBytes(Entry_Stats + 0xcb45, Title, sizeof(Title));
                    WriteBytes(Entry_Stats + 0xcb5d, Title, sizeof(Title));
                    WriteBytes(Entry_Stats + 0xcb62, Title, sizeof(Title));
                    WriteBytes(Entry_Stats + 0xcb7b, Title, sizeof(Title));
                    WriteBytes(Entry_Stats + 0xcb98, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcb9e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcba5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcbaa, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcbb1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcbc2, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcbd5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcbda, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcbe1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcbed, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcbf8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcc95, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xccc3, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcd01, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcd20, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcd5a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcdae, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcdcd, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcdd8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcdf1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xce93, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xcfe9, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xd0cd, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xd13f, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc011, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc03a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc041, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc047, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc04d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc053, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc088, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc0c5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc0d1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc0e2, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc0e8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc0ee, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc101, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc107, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc10d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc113, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc118, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc137, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc13d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc143, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc14e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc155, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc167, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc178, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc17e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc185, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc18a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc1c1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc1c7, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc1cd, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc1d2, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc1d8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc1e5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc20e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc227, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc232, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc251, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc257, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2a5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2aa, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2b1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2b7, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2bd, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2c8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2ce, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2d5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2db, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2e1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2e7, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2ed, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2f2, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2f8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc2fe, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc305, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc30b, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc317, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc31d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc322, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc328, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc34d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc352, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc358, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc35e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc365, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc371, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc382, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc38e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3a7, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3b2, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3b8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3bf, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3c5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3ca, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3d1, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3d7, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3dd, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3e3, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3e8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc3fa, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc401, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc407, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc40d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc412, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc418, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc41e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc425, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc42a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc431, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc436, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc44e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc455, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc461, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc467, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc472, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc478, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc47e, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc49d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc4a2, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc4a8, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc4b5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc4cd, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc4e5, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc4ea, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc50f, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc515, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc51a, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc521, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc527, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc52d, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc532, Title, sizeof(Title));
					WriteBytes(Entry_Stats + 0xc538, Title, sizeof(Title));
		{
 

 };
					break;
				}
				break;
			case Clients:
				addTitle("Clients Menu");
				addOption(GetName(0), NULL);
				addOption(GetName(1), NULL);
				addOption(GetName(2), NULL);
				addOption(GetName(3), NULL);
				addOption(GetName(4), NULL);
				addOption(GetName(5), NULL);
				addOption(GetName(6), NULL);
				addOption(GetName(7), NULL);
				addOption(GetName(8), NULL);
				addOption(GetName(9), NULL);
				addOption(GetName(10), NULL);
				addOption(GetName(11), NULL);
				if (OpenOption)
				{
					selectedPlayer = currentOption - 1;
					changeSubmenu(Player_Options);
				}
				break;
				#pragma region Player Options
			case Player_Options:
				addTitle("Clients Menu");
				addOption("God Mode", NULL);
				addOption("Unlimited Ammo", NULL);
				addOption("Uav", NULL);
				addOption("Give Weapons", NULL);
				switch (getOption())
				{
				case 1:
					*(char*)(G_Client(selectedPlayer) + 0x23 + (selectedPlayer * 0x6200)) = 0x05000000;
					break;
				case 2:
					*(char*)(G_Client(selectedPlayer) + 0x548 + (selectedPlayer * 0x6200)) = 0x7fffffff;
					*(char*)(G_Client(selectedPlayer) + 0x54C + (selectedPlayer * 0x6200)) = 0x7fffffff;
					*(char*)(G_Client(selectedPlayer) + 0x584 + (selectedPlayer * 0x6200)) = 0x7fffffff;
					*(char*)(G_Client(selectedPlayer) + 0x588 + (selectedPlayer * 0x6200)) = 0x7fffffff;
					*(char*)(G_Client(selectedPlayer) + 0x593 + (selectedPlayer * 0x6200)) = 0x7fffffff;
					*(char*)(G_Client(selectedPlayer) + 0x58F + (selectedPlayer * 0x6200)) = 0x7fffffff;
					break;
				case 3:
					*(char*)(G_Client(selectedPlayer) + 0x5EFF + (selectedPlayer * 0x6200)) = 0x07000000;
					break;
				case 4:
					changeSubmenu(weaponsclient);
					break;
				}
				break;
				case weapons2clients:
				//0x18C4643
				addTitle("Primary Weapons Menu");
				addOption("Default Weapons", NULL);
				addOption("Vesper", NULL);
				addOption("VMP", NULL);
				addOption("Weevil", NULL);
				addOption("Pharo", NULL);
				addOption("Razorback", NULL);
				addOption("Other Weapons", NULL);
				addOption("Bullet Weapons", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x0100000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x0800000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x0400000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x0600000000;
					break;
			    case 5:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x0A00000000;
					break;
                case 6:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x0C00000000;
					break;
				case 7:
					changeSubmenu(Shadow2clients);
					break;
				case 8:
					changeSubmenu(Der2weapons);
					break;
				}
				break;
				case Shadow2clients:
				addTitle("Other Weapons");
				addOption("KN-44", NULL);
				addOption("HVK-30", NULL);
				addOption("Shieva", NULL);
				addOption("M8A7", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x0E00000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x1200000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x1800000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x1A00000000;
					break;
				}
				break;
				case Der2weapons:
				addTitle("Bullet Weapons");
				addOption("Hellstorm", NULL);
				addOption("Turrett", NULL);
				addOption("Missile Turrett", NULL);
				addOption("HC-XD", NULL);
				addOption("Talon", NULL);
				addOption("Mothership", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x7800000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x7300000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x6D00000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x6B00000000;
					break;
				case 5:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x7600000000;
					break;
                case 6:
					*(int*)(G_Client(selectedPlayer) + 0x30B) = 0x7000000000;
					break;
				}
					break;
#pragma endregion
				case AllClients:
				addTitle("All Clients Menu");
				addBoolOption("God Mode", GodModeAC, NULL);
				addBoolOption("Unlimited Ammo", AmmoAC, NULL);
				addBoolOption("UAV", uavac, NULL);
				addBoolOption("Speed x2", Speedx2AC, NULL);
				addBoolOption("Fake Lag", FakeLagAC, NULL);
				addBoolOption("Freeze", FreezeAC, NULL);
				addBoolOption("Third Person", ThirdAC, NULL);
				addBoolOption("Invisible", InvisibleAC, NULL);
				addBoolOption("JetPack", jetpackAC, NULL);
				addBoolOption("Killstreak", killstreakAC, NULL);
				addBoolOption("Become Spectator", specac, NULL);
				addOption("Give Weapons", NULL);
				switch (getOption())
				{
				case 1:
					GodModeAC = !GodModeAC;
					if (GodModeAC == true)
					{
						*(int*)(0x18C6243) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x05000000;
					    *(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x05000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x05000000;
					}

					else if (GodModeAC == false)
					{
						*(int*)(0x18C6243) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
					    *(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x04000000;
					}
					break;
				case 2:
					AmmoAC = !AmmoAC;
					if (AmmoAC == true)
					{
						*(int*)(0x18C6768) = 0x7fffffff;
						*(int*)(0x18C676C) = 0x7fffffff;
						*(int*)(0x18C67A4) = 0x7fffffff;
						*(int*)(0x18C67A8) = 0x7fffffff;
						*(int*)(0x18C67B3) = 0x7fffffff;	
						*(int*)(0x18C67AF) = 0x7fffffff;	

						*(int*)(0x18C6768 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
					}
					else if (AmmoAC == false)
					{
						*(int*)(0x18C6768) = 0x0000000;
						*(int*)(0x18C676C) = 0x0000000;
						*(int*)(0x18C67A4) = 0x0000000;
						*(int*)(0x18C67A8) = 0x0000000;
						*(int*)(0x18C67B3) = 0x0000000;	
						*(int*)(0x18C67AF) = 0x0000000;	

						*(int*)(0x18C6768 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;

						*(int*)(0x18C6768 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C676C + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x7fffffff;
						*(int*)(0x18C67A4 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67A8 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
						*(int*)(0x18C67B3 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;	
						*(int*)(0x18C67AF + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0000000;
					}
					break;
				case 3:
					uavac = !uavac;
					if (uavac == true)
					{
						*(int*)(0x18CC11F) = 0x01000000;		
						*(int*)(0x18CC11F + 0x6200) = 0x01000000;		
						*(int*)(0x18CC11F + 0x6200 + 0x6200) = 0x01000000;		
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200) = 0x01000000;		
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
					}
					else if (uavac == false)
					{
				     *(int*)(0x18CC11F) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					 *(int*)(0x18CC11F + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					}
					break;
				case 4:
					Speedx2AC = !Speedx2AC;
					if (Speedx2AC == true)
					{
						*(int*)(0x18CC018) = 0x42040000;
						*(int*)(0x18CC018 + 0x6200) = 0x42040000;		
						*(int*)(0x18CC018 + 0x6200 + 0x6200) = 0x42040000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;		
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x42040000;	
					}
					else if (Speedx2AC == false)
					{
						*(int*)(0x18CC018) = 0x3f800000;
						*(int*)(0x18CC018 + 0x6200) = 0x3f800000;		
						*(int*)(0x18CC018 + 0x6200 + 0x6200) = 0x3f800000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200) = 0x3f800000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f800000;		
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f800000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f800000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f800000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f800000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f800000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f800000;	
						*(int*)(0x18CC018 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f80000;
					}
					break;
				case 5:
					FakeLagAC = !FakeLagAC;
					if (FakeLagAC == true)
					{
						*(int*)(0x18CBF47) = 0x01000000;		
						*(int*)(0x18CBF47 + 0x6200) = 0x01000000;		
						*(int*)(0x18CBF47 + 0x6200 + 0x6200) = 0x01000000;		
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;		
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
					}
					else if (FakeLagAC == false)
					{
						*(int*)(0x18CBF47) = 0x02000000;		
						*(int*)(0x18CBF47 + 0x6200) = 0x02000000;		
						*(int*)(0x18CBF47 + 0x6200 + 0x6200) = 0x02000000;		
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;		
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;
						*(int*)(0x18CBF47 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x02000000;
					}
					break;
				case 6:
					FreezeAC = !FreezeAC;
					if (FreezeAC == true)
					{
						*(int*)(0x18C6224) = 0x09000000;	
						*(int*)(0x18C6224 + 0x6200) = 0x09000000;	
						*(int*)(0x18C6224 + 0x6200 + 0x6200) = 0x09000000;	
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200) = 0x09000000;	
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x09000000;	
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x09000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x09000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x09000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x09000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x09000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x0900000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x09;
					}
					else if (FreezeAC == false)
					{
						*(int*)(0x18C6224) = 0x00000000;	
						*(int*)(0x18C6224 + 0x6200) = 0x06000000;	
						*(int*)(0x18C6224 + 0x6200 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x06000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x06000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x06000000;
						*(int*)(0x18C6224 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x06000000;
					}
					break;
				case 7:
					ThirdAC = !ThirdAC;
					if (ThirdAC == true)
					{
						*(int*)(0x18C634D) = 0x01000000;		
						*(int*)(0x18C634D + 0x6200) = 0x01000000;		
						*(int*)(0x18C634D + 0x6200 + 0x6200) = 0x01000000;		
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x01000000;
					}
					else if (ThirdAC == false)
					{
						*(int*)(0x18C634D) = 0x00;		
						*(int*)(0x18C634D + 0x6200) = 0x00000000;		
						*(int*)(0x18C634D + 0x6200 + 0x6200) = 0x00000000;		
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C634D + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					}
					break;
				case 8:
					InvisibleAC = !InvisibleAC;
					if (InvisibleAC == true)
					{
						*(int*)(G_Client(-1)) = 0x01000000;		
						
					}
					else if (InvisibleAC == false)
					{
						*(int*)(G_Client(-1)) = 0x00000000;		
					}
					break;
				case 9:
					jetpackAC = !jetpackAC;
					if (jetpackAC == true)
					{
						*(int*)(0x18C6950) = 0x3f000000;	
						*(int*)(0x18C6950 + 0x6200) = 0x3f000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200) = 0x3f000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x3f000000;
					}
					else if (jetpackAC == false)
					{
						*(int*)(0x18C6950) = 0x00000000;	
						*(int*)(0x18C6950 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;	
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
						*(int*)(0x18C6950 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x00000000;
					}
					break;
				case 10:
					killstreakAC = !killstreakAC;
					if (killstreakAC == true)
					{
						*(int*)(0x18C675E) = 0xff000000;
						*(int*)(0x18C675E + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
						*(int*)(0x18C675E + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0xff000000;
					}
					else if (killstreakAC == false)
					{
                        // Nothing
					}
					break;
				case 11:
					specac = !specac;
					if (specac == true)
					{
						*(int*)(0x18C6243) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x07000000;
					    *(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x07000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x07000000;
					}
					else if (specac == false)
					{
						*(int*)(0x18C6243) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
					    *(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 ) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x04000000;
						*(int*)(0x18C6243 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200 + 0x6200) = 0x04000000;
					}
					break;
				case 12:
					changeSubmenu(weaponsac);
					break;
				
				}
				break;
				case weaponsac:
				//0x18C4643
				addTitle("Secondary Weapons Menu");
				addOption("Default Weapons", NULL);
				addOption("Vesper", NULL);
				addOption("VMP", NULL);
				addOption("Kuda", NULL);
				addOption("Weevil", NULL);
				addOption("Pharo", NULL);
				addOption("Other Weapons", NULL);
				addOption("Bullett Weapons", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x30B) = 0x01000000;
					*(int*)(G_Client(1) + 0x30B) = 0x01000000;
					*(int*)(G_Client(2) + 0x30B) = 0x01000000;
					*(int*)(G_Client(3) + 0x30B) = 0x01000000;
					*(int*)(G_Client(4) + 0x30B) = 0x01000000;
					*(int*)(G_Client(5) + 0x30B) = 0x01000000;
					*(int*)(G_Client(6) + 0x30B) = 0x01000000;
					*(int*)(G_Client(7) + 0x30B) = 0x01000000;
					*(int*)(G_Client(8) + 0x30B) = 0x01000000;
					*(int*)(G_Client(9) + 0x30B) = 0x01000000;
					*(int*)(G_Client(10) + 0x30B) = 0x01000000;
					*(int*)(G_Client(11) + 0x30B) = 0x01000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x30B) = 0x08000000;
					*(int*)(G_Client(1) + 0x30B) = 0x08000000;
					*(int*)(G_Client(2) + 0x30B) = 0x08000000;
					*(int*)(G_Client(3) + 0x30B) = 0x08000000;
					*(int*)(G_Client(4) + 0x30B) = 0x08000000;
					*(int*)(G_Client(5) + 0x30B) = 0x08000000;
					*(int*)(G_Client(6) + 0x30B) = 0x08000000;
					*(int*)(G_Client(7) + 0x30B) = 0x08000000;
					*(int*)(G_Client(8) + 0x30B) = 0x08000000;
					*(int*)(G_Client(9) + 0x30B) = 0x08000000;
					*(int*)(G_Client(10) + 0x30B) = 0x08000000;
					*(int*)(G_Client(11) + 0x30B) = 0x08000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x30B) = 0x04000000;
					*(int*)(G_Client(1) + 0x30B) = 0x04000000;
					*(int*)(G_Client(2) + 0x30B) = 0x04000000;
					*(int*)(G_Client(3) + 0x30B) = 0x04000000;
					*(int*)(G_Client(4) + 0x30B) = 0x04000000;
					*(int*)(G_Client(5) + 0x30B) = 0x04000000;
					*(int*)(G_Client(6) + 0x30B) = 0x04000000;
					*(int*)(G_Client(7) + 0x30B) = 0x04000000;
					*(int*)(G_Client(8) + 0x30B) = 0x04000000;
					*(int*)(G_Client(9) + 0x30B) = 0x04000000;
					*(int*)(G_Client(10) + 0x30B) = 0x04000000;
					*(int*)(G_Client(11) + 0x30B) = 0x04000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x30B) = 0x02000000;
					*(int*)(G_Client(1) + 0x30B) = 0x02000000;
					*(int*)(G_Client(2) + 0x30B) = 0x02000000;
					*(int*)(G_Client(3) + 0x30B) = 0x02000000;
					*(int*)(G_Client(4) + 0x30B) = 0x02000000;
					*(int*)(G_Client(5) + 0x30B) = 0x02000000;
					*(int*)(G_Client(6) + 0x30B) = 0x02000000;
					*(int*)(G_Client(7) + 0x30B) = 0x02000000;
					*(int*)(G_Client(8) + 0x30B) = 0x02000000;
					*(int*)(G_Client(9) + 0x30B) = 0x02000000;
					*(int*)(G_Client(10) + 0x30B) = 0x02000000;
					*(int*)(G_Client(11) + 0x30B) = 0x02000000;
					break;
			    case 5:
					*(int*)(G_Client(0) + 0x30B) = 0x06000000;
					*(int*)(G_Client(1) + 0x30B) = 0x06000000;
					*(int*)(G_Client(2) + 0x30B) = 0x06000000;
					*(int*)(G_Client(3) + 0x30B) = 0x06000000;
					*(int*)(G_Client(4) + 0x30B) = 0x06000000;
					*(int*)(G_Client(5) + 0x30B) = 0x06000000;
					*(int*)(G_Client(6) + 0x30B) = 0x06000000;
					*(int*)(G_Client(7) + 0x30B) = 0x06000000;
					*(int*)(G_Client(8) + 0x30B) = 0x06000000;
					*(int*)(G_Client(9) + 0x30B) = 0x06000000;
					*(int*)(G_Client(10) + 0x30B) = 0x06000000;
					*(int*)(G_Client(11) + 0x30B) = 0x06000000;
					break;
                case 6:
					*(int*)(G_Client(0) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(1) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(2) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(3) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(4) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(5) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(6) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(7) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(8) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(9) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(10) + 0x30B) = 0x0A000000;
					*(int*)(G_Client(11) + 0x30B) = 0x0A000000;
					break;
				case 7:
					changeSubmenu(shadowsac);
					break;
				case 8:
					changeSubmenu(derac);
					break;
				}
				break;
				case shadowsac:
				addTitle("Other Weapons");
				addOption("KN-44", NULL);
				addOption("HVK-30", NULL);
				addOption("Shieva", NULL);
				addOption("M8A7", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(1) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(2) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(3) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(4) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(5) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(6) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(7) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(8) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(9) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(10) + 0x30B) = 0x0E000000;
					*(int*)(G_Client(11) + 0x30B) = 0x0E000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x30B) = 0x12000000;
					*(int*)(G_Client(1) + 0x30B) = 0x12000000;
					*(int*)(G_Client(2) + 0x30B) = 0x12000000;
					*(int*)(G_Client(3) + 0x30B) = 0x12000000;
					*(int*)(G_Client(4) + 0x30B) = 0x12000000;
					*(int*)(G_Client(5) + 0x30B) = 0x12000000;
					*(int*)(G_Client(6) + 0x30B) = 0x12000000;
					*(int*)(G_Client(7) + 0x30B) = 0x12000000;
					*(int*)(G_Client(8) + 0x30B) = 0x12000000;
					*(int*)(G_Client(9) + 0x30B) = 0x12000000;
					*(int*)(G_Client(10) + 0x30B) = 0x12000000;
					*(int*)(G_Client(11) + 0x30B) = 0x12000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x30B) = 0x18000000;
					*(int*)(G_Client(1) + 0x30B) = 0x18000000;
					*(int*)(G_Client(2) + 0x30B) = 0x18000000;
					*(int*)(G_Client(3) + 0x30B) = 0x18000000;
					*(int*)(G_Client(4) + 0x30B) = 0x18000000;
					*(int*)(G_Client(5) + 0x30B) = 0x18000000;
					*(int*)(G_Client(6) + 0x30B) = 0x18000000;
					*(int*)(G_Client(7) + 0x30B) = 0x18000000;
					*(int*)(G_Client(8) + 0x30B) = 0x18000000;
					*(int*)(G_Client(9) + 0x30B) = 0x18000000;
					*(int*)(G_Client(10) + 0x30B) = 0x18000000;
					*(int*)(G_Client(11) + 0x30B) = 0x18000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(1) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(2) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(3) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(4) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(5) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(6) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(7) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(8) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(9) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(10) + 0x30B) = 0x1A000000;
					*(int*)(G_Client(11) + 0x30B) = 0x1A000000;
					break;
				}
				break;
				case derac:
				addTitle("Bullet Weapons");
				addOption("Hellstorm", NULL);
				addOption("Talon", NULL);
				addOption("Turret", NULL);
				addOption("Missile Turret", NULL);
				addOption("HC-XD", NULL);
				addOption("Mothership", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(0) + 0x30B) = 0x78000000;
					*(int*)(G_Client(1) + 0x30B) = 0x78000000;
					*(int*)(G_Client(2) + 0x30B) = 0x78000000;
					*(int*)(G_Client(3) + 0x30B) = 0x78000000;
					*(int*)(G_Client(4) + 0x30B) = 0x78000000;
					*(int*)(G_Client(5) + 0x30B) = 0x78000000;
					*(int*)(G_Client(6) + 0x30B) = 0x78000000;
					*(int*)(G_Client(7) + 0x30B) = 0x78000000;
					*(int*)(G_Client(8) + 0x30B) = 0x78000000;
					*(int*)(G_Client(9) + 0x30B) = 0x78000000;
					*(int*)(G_Client(10) + 0x30B) = 0x78000000;
					*(int*)(G_Client(11) + 0x30B) = 0x78000000;
					break;
				case 2:
					*(int*)(G_Client(0) + 0x30B) = 0x76000000;
					*(int*)(G_Client(1) + 0x30B) = 0x76000000;
					*(int*)(G_Client(2) + 0x30B) = 0x76000000;
					*(int*)(G_Client(3) + 0x30B) = 0x76000000;
					*(int*)(G_Client(4) + 0x30B) = 0x76000000;
					*(int*)(G_Client(5) + 0x30B) = 0x76000000;
					*(int*)(G_Client(6) + 0x30B) = 0x76000000;
					*(int*)(G_Client(7) + 0x30B) = 0x76000000;
					*(int*)(G_Client(8) + 0x30B) = 0x76000000;
					*(int*)(G_Client(9) + 0x30B) = 0x76000000;
					*(int*)(G_Client(10) + 0x30B) = 0x76000000;
					*(int*)(G_Client(11) + 0x30B) = 0x76000000;
					break;
				case 3:
					*(int*)(G_Client(0) + 0x30B) = 0x73000000;
					*(int*)(G_Client(1) + 0x30B) = 0x73000000;
					*(int*)(G_Client(2) + 0x30B) = 0x73000000;
					*(int*)(G_Client(3) + 0x30B) = 0x73000000;
					*(int*)(G_Client(4) + 0x30B) = 0x73000000;
					*(int*)(G_Client(5) + 0x30B) = 0x73000000;
					*(int*)(G_Client(6) + 0x30B) = 0x73000000;
					*(int*)(G_Client(7) + 0x30B) = 0x73000000;
					*(int*)(G_Client(8) + 0x30B) = 0x73000000;
					*(int*)(G_Client(9) + 0x30B) = 0x73000000;
					*(int*)(G_Client(10) + 0x30B) = 0x73000000;
					*(int*)(G_Client(11) + 0x30B) = 0x73000000;
					break;
				case 4:
					*(int*)(G_Client(0) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(1) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(2) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(3) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(4) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(5) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(6) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(7) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(8) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(9) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(10) + 0x30B) = 0x6D000000;
					*(int*)(G_Client(11) + 0x30B) = 0x6D000000;
					break;
				case 5:
					*(int*)(G_Client(0) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(1) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(2) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(3) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(4) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(5) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(6) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(7) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(8) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(9) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(10) + 0x30B) = 0x6B000000;
					*(int*)(G_Client(11) + 0x30B) = 0x6B000000;
					break;
				case 6:
					*(int*)(G_Client(0) + 0x30B) = 0x70000000;
					*(int*)(G_Client(1) + 0x30B) = 0x70000000;
					*(int*)(G_Client(2) + 0x30B) = 0x70000000;
					*(int*)(G_Client(3) + 0x30B) = 0x70000000;
					*(int*)(G_Client(4) + 0x30B) = 0x70000000;
					*(int*)(G_Client(5) + 0x30B) = 0x70000000;
					*(int*)(G_Client(6) + 0x30B) = 0x70000000;
					*(int*)(G_Client(7) + 0x30B) = 0x70000000;
					*(int*)(G_Client(8) + 0x30B) = 0x70000000;
					*(int*)(G_Client(9) + 0x30B) = 0x70000000;
					*(int*)(G_Client(10) + 0x30B) = 0x70000000;
					*(int*)(G_Client(11) + 0x30B) = 0x70000000;
					break;
				}
				break;
#pragma endregion
				#pragma region LOL
              case weaponsclient:
				addTitle("Weapons Menu");
				addOption("Special", NULL);
				addOption("Equipment", NULL);
				addOption("Specialist", NULL);
				addOption("Sub Machine Guns", NULL);
				addOption("Assault Rifles", NULL);
				addOption("Light Machine Guns", NULL);
				addOption("Snipers", NULL);
				addOption("Shotguns", NULL);
				addOption("Pistols", NULL);
				addOption("Launcher", NULL);
				addOption("Killstreaks", NULL);
				switch (getOption())
				{
				case 1:
					changeSubmenu(Specialclient);
					break;
				case 2:
					changeSubmenu(Equipementclient);
					break;
				case 3:
					changeSubmenu(specialistsclient);
					break;
				case 4:
					changeSubmenu(sub_machine_gunclient);
					break;
			    case 5:
					changeSubmenu(assault_rifleclient);
					break;
                case 6:
					changeSubmenu(light_machine_gunclient);
					break;
				case 7:
					changeSubmenu(snipersclient);
					break;
				case 8:
					changeSubmenu(shotgunclient);
					break;
				case 9:
					changeSubmenu(pistolsclient);
					break;
				case 10:
					changeSubmenu(launcherclient);
					break;
				case 11:
					changeSubmenu(killstreaksclient);
					break;
				}
				break;
				case Specialclient:
				addTitle("Special Weapons Menu");
				addOption("Default Weapons", NULL);
				addOption("Combat Knife", NULL);
				addOption("Pink Weapons", NULL);
				addOption("Satellite Ball", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x01000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x3B000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x66000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x3F000000;
					break;
				}
				break;
				case Equipementclient:
				addTitle("Equipment Weapons Menu");
				addOption("Flash Bangs", NULL);
				addOption("Grenades", NULL);
				addOption("1 Grenade Infinite Concussions", NULL);
				addOption("Weird Stick Thing", NULL);
				addOption("Sticky Grenades", NULL);
				addOption("Smoke Grenades ", NULL);
				addOption("TripMines", NULL);
				addOption("EMP Grenades", NULL);
				addOption("Thermites", NULL);
				addOption("Trophy System ", NULL);
				addOption("Combat Axe", NULL);
				addOption("Shock Charge", NULL);
				addOption("C4", NULL);
				addOption("Blackhat", NULL);
				addOption("Nova Gas", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x41000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x42000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x43000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x44000000;
					break;
				case 5:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x45000000;
					break;
                case 6:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x46000000;
					break;
				case 7:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x48000000;
					break;
				case 8:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x49000000;
					break;
				case 9:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x4B000000;
					break;
				case 10:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x4C000000;
					break;
				case 11:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x4E000000;
					break;
                case 12:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x4F000000;
					break;
				case 13://
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x50000000;
					break;
				case 14:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x53000000;
					break;
				case 15:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x58000000;
					break;
				}
				break;
#pragma endregion
#pragma region Map Changer
				case specialistsclient:
				//0x18C4643
				addTitle("Specialists Weapons Menu");
				addOption("Tempest", NULL);
				addOption("Gravity Spikes", NULL);
				addOption("Ripper", NULL);
				addOption("Anihilator", NULL);
				addOption("War Machine ", NULL);
				addOption("Bow ", NULL);
				addOption("Hive", NULL);
				addOption("FlameThrower", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x5A000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x5B000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x5C000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x5D000000;
					break;
			    case 5:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x5E000000;
					break;
                case 6:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x60000000;
					break;
				case 7:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x64000000;
					break;
				case 8:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x65000000;
					break;
				}
				break;
				case sub_machine_gunclient:
				addTitle("Machine Gun Weapons Menu");
				addOption("Kuda", NULL);
				addOption("VMP", NULL);
				addOption("Weevil", NULL);
				addOption("Vesper", NULL);
				addOption("Pharo", NULL);
				addOption("Razorback", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x02000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x04000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x06000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x08000000;
					break;
				case 5:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x0A000000;
					break;
				case 6:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x0C000000;
					break;
				}
				break;
				case assault_rifleclient:
				addTitle("Assault Weapons Menu");
				addOption("KN44", NULL);
				addOption("XR2", NULL);
				addOption("HVK30", NULL);
				addOption("ICR1", NULL);
				addOption("Man-O-War", NULL);
				addOption("Shieva", NULL);
			    addOption("M8A7", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x0E000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x10000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x12000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x14000000;
					break;
				case 5:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x16000000;
					break;
				case 6:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x18000000;
					break;
				case 7:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x1A000000;
					break;
				}
				break;
				case light_machine_gunclient:
				addTitle("Light Weapons Menu");
				addOption("BRM", NULL);
				addOption("Dingo", NULL);
				addOption("Gorgon", NULL);
				addOption("Dredge", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x20000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x22000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x24000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x26000000;
					break;
				}
				break;
				case snipersclient:
				addTitle("Snipers Weapons Menu");
				addOption("Drakon", NULL);
				addOption("Locus", NULL);
				addOption("P06", NULL);
				addOption("SVG100", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x28000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x2A000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x2C000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x2E000000;
					break;
				}
				break;
				case shotgunclient:
				addTitle("Shotgun Weapons Menu");
				addOption("KRM", NULL);
				addOption("Brecci", NULL);
				addOption("Haymaker", NULL);
				addOption("Argus", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x1C000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x1D000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x1E000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x1F000000;
					break;
				}
				break;
				case pistolsclient:
				addTitle("Weapons Menu");
				addOption("MR6", NULL);
				addOption("RK5", NULL);
				addOption("LCar9", NULL);		
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x30000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x35000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x38000000;
					break;
				
				}
				break;
				case launcherclient:
				addTitle("Launcher Weapons Menu");
				addOption("Blackcell", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x3A000000;
					break;
				
				}
				break;
				case killstreaksclient:
				addTitle("Killstreaks Weapons Menu");
				addOption("HCXD", NULL);
				addOption("Bullet Turret", NULL);
				addOption("Missile Turret", NULL);
				addOption("Missile Turret Slow", NULL);
				addOption("Mothership", NULL);
				addOption("Bullet Turret Fast", NULL);
				addOption("Spawns Talon", NULL);
				addOption("Hellstorm", NULL);
				switch (getOption())
				{
				case 1:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x6B000000;
					break;
				case 2:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x6C000000;
					break;
				case 3:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x6D000000;
					break;
				case 4:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x6F000000;
					break;
				case 5:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x70000000;
					break;
				case 6:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x75000000;
					break;
				case 7:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x76000000;
					break;
				case 8:
					*(int*)(G_Client(selectedPlayer) + 0x383) = 0x78000000;
					break;
				}
				break;
#pragma endregion
			}
			OpenOption = false;
			rightPress = false;
			leftPress = false;
			fastRightPress = false;
			fastLeftPress = false;
			squarePress = false;
		}
		else isInGame = false;
		return hookStub(TimeId);
}

extern "C" int _BlackOps3ZombieMenuNoHost_prx_entry(void)
{
	cellMsgDialogOpen(1,"Spectre Engine Loaded\nCreated By Indie Developer & Guillaume MrNiato\n\nVersion : 1.00",my_dialog2,(void*) 0x0000aaab, NULL);
	PatchInJump(R_SetFrameFog, (int)Hook, false);
	sys_ppu_thread_t id;
	sys_ppu_thread_create(&id,Menu,0,0x4AA,0x1000,0,"Menu");
	
	
    return SYS_PRX_RESIDENT;
}
SYS_MODULE_INFO( BlackOps3ZombieMrNiatoSPRX, 0, 1, 1);
SYS_MODULE_START( _BlackOps3ZombieMenuNoHost_prx_entry );

SYS_LIB_DECLARE_WITH_STUB( LIBNAME, SYS_LIB_AUTO_EXPORT, STUBNAME );
SYS_LIB_EXPORT( _BlackOps3ZombieMenuNoHost_export_function, LIBNAME );

sys_ppu_thread_t create_thread(void (*entry)(uint64_t), int priority, size_t stacksize, const char* threadname,sys_ppu_thread_t id)
{      
       if(sys_ppu_thread_create(&id, entry, 0, priority , stacksize, 0, threadname) == CELL_OK)
	{
		console_write("\n\nINJECTED !\n\n");	
	}
		
    return id;
}

// An exported function is needed to generate the project's PRX stub export library
extern "C" int _BlackOps3ZombieMenuNoHost_export_function(void)
{
	console_write("\n\nSpectre Project Injected !\n\n");
    return CELL_OK;
}
void writeString(const char* str, int len, unsigned int addr)
{
   for(int i = 0; i < len; i++)
   {
	   if(*str == 0x00) {break;}
   *(char*)(addr+i) = *(str+i);
   }
}
