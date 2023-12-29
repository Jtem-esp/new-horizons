#define MAPSPEED	1.0	// ccc mapships - Petros: reset to 1.0 to prevent trouble, was 0.6
//#define MAP_VOYAGELENGTH	8.0 // TIH moved to BS.h	// ccc maptweak, lets voyages take longer, original 4.0

/*
	Program interface:

	Init world map in start
		void wdmInitWorldMap();
	Create world map object
		void wdmCreateMap();
	Release world map object
		void wdmReleaseMap();


	SPI (Script program interface):
	Получение данных от карты
	(const) поля только для прочтения
	(var) поля только для записи и прочтения, запись в них приводит к обновления остальных полей,
	      принадлежащих этой группе

	Шторма
	worldMap.storm.num - текущие количество штормов в карте (const)
	worldMap.storm.cur - индекс текущего шторма 0..num-1, надо перезаписывать для обновления полей (var)
	worldMap.storm.x - позиция шторма по x (const)
	worldMap.storm.z - позиция шторма по z (const)
	worldMap.storm.time - оставшееся время жизни в секундах (const)

	Кораблики событий
	worldMap.encounter.num - текущие количество корабликов в карте (const)
	worldMap.encounter.cur - индекс текущего кораблика в карте (var)
	worldMap.encounter.x - позиция кораблика по x (const)
	worldMap.encounter.z - позиция кораблика по z (const)
	worldMap.encounter.time - оставшееся время жизни в секундах (const)
	worldMap.encounter.type - тип события, обозначаемый этим корабликом (define в таблице типов энкоунтеров)
	worldMap.encounter.attack - индекс кораблика, который атакуем (стоя на месте)

	Разная информация
	worldMap.info.playerInStorm - если 1 то плеер находиться в шторме
	worldMap.info.updateinfo - апдейт информации


	//Внутреняя информация - !!!Осторожно, ни каких проверок!!!
	worldMap.enemyshipViewDistMin;		//Растояние на котором корабль начинает исчезать
	worldMap.enemyshipViewDistMax;		//Растояние на котором корабль исчезает полностью
	worldMap.enemyshipDistKill;			//Расстояние на котором убиваем корабль
	worldMap.enemyshipBrnDistMin;		//Минимальное растояние на котором рожается корабль
	worldMap.enemyshipBrnDistMax;		//Максимальное растояние на котором рожается корабль

	worldMap.stormViewDistMin;			//Растояние на котором шторм начинает исчезать
	worldMap.stormViewDistMax;			//Растояние на котором шторм исчезает полностью
	worldMap.stormDistKill;				//Расстояние на котором убиваем шторм
	worldMap.stormBrnDistMin;			//Минимальное растояние на котором рожается шторм
	worldMap.stormBrnDistMax;			//Максимальное растояние на котором рожается шторм

*/

#include "worldmap\worldmap_globals.c"
#include "worldmap\worldmap_events.c"
#include "worldmap\worldmap_reload.c"
#include "worldmap\worldmap_encgen.c"
#include "worldmap\DailyCrewUpdate.c" // NK

//=========================================================================================

extern void wdmInitWorldMap();
extern void wdmReinit();

#event_handler("WorldMap_EncounterCreate", "wdmEvent_EncounterCreate");
#event_handler("WorldMap_PlayerInStorm", "wdmEvent_PlayerInStorm");
#event_handler("WorldMap_ShipEncounter", "wdmEvent_ShipEncounter");

//#event_handler("WorldMap_EventWindowChangeSelectoin", "wdmEvent_EventWindowChangeSelectoin");
//#event_handler("WorldMap_EventWindowSelect", "wdmEvent_EventWindowSelect");
//#event_handler("WorldMap_WaitMenuChangeSelectoin", "wdmEvent_WaitMenuChangeSelectoin");
//#event_handler("WorldMap_WaitMenuSelect", "wdmEvent_WaitMenuSelect");
//#event_handler("WorldMap_DeleteShipEncounter", "wdmEvent_DeleteShipEncounter");

#event_handler("WorldMap_UpdateDate", "wdmEvent_UpdateDate");
#event_handler("ExitFromWorldMap", "wdmReloadToSea");
#event_handler("EventLoadLocation", "wdmOpenLocation");

//=========================================================================================
// Programm interface
//=========================================================================================
bool wdmNoClearEncountersTable = false;

void wdmCreateMap(float dx, float dz, float ay)
{
	float fadeInTime = 0.5; // KK
	wdmDisableTornadoGen = true;
	worldMap.playerShipAY = ay;
	ClearAllLogStrings();
	wdmDisableRelease = true;
	//Create world map engine object
	ReloadProgressStart();
	wdmRemoveOldEncounters();
// KK -->
	// if (bNewInterface)
	// 	CreateEntity(&worldMap, "wrldmap2");
	// else
	if (!IsEntity(&worldMap)) {
		CreateEntity(&worldMap,"worldmap");
	}
// <-- KK
	worldMap.isLoaded = "true";
	//Set player ship position
	worldMap.playerShipDispX = dx / WDM_MAP_TO_SEA_SCALE;
	worldMap.playerShipDispZ = dz / WDM_MAP_TO_SEA_SCALE;
	//Update parameters
	worldMap.update = "";
	//Reset encounters data
	worldMap.playerInStorm = "0";
	wdmReset();
	worldMap.playerShipUpdateDisp = "";
	if (wdmNoClearEncountersTable == false) ReleaseMapEncounters();
	wdmNoClearEncountersTable = false; 
	//Fade in
// KK -->
	if (!IsEntity(&wdm_fader)) {
		CreateEntity(&wdm_fader, "fader");
		if (IsEntity(&wdm_fader)) {	
			SendMessage(&wdm_fader, "lfl", FADER_IN, fadeInTime, true);
			SetReloadImage(&wdm_fader, "sea");
		} else {
			trace("Fader not created!!!");
		}
	}
// <-- KK
	SetSchemeForMap();
	Event("EventWorldMapInit"); // KK
	ReloadProgressEnd();

	// ccc maptweak
	worldMap.date.hourPerSec = MAP_VOYAGELENGTH;	// ccc maptweak original 4.0
	SetTimeScale(MAPSPEED);			// slow motion
	SetWeatherScheme("seashore_weather");		
	// ccc maptweak end

	InitWmInterface()
}

void wdmCreateWorldMap()
{
	float dshipX = MakeFloat(worldMap.playerShipDispX)*WDM_MAP_TO_SEA_SCALE;
	float dshipZ = MakeFloat(worldMap.playerShipDispZ)*WDM_MAP_TO_SEA_SCALE;
	float shipAY = MakeFloat(worldMap.playerShipAY);
	wdmCreateMap(dshipX, dshipZ, shipAY);
}

void wdmLoadSavedMap()
{
	float dshipX = MakeFloat(worldMap.playerShipDispX)*WDM_MAP_TO_SEA_SCALE;
	float dshipZ = MakeFloat(worldMap.playerShipDispZ)*WDM_MAP_TO_SEA_SCALE;
	float shipAY = MakeFloat(worldMap.playerShipAY);
	wdmNoClearEncountersTable = true;
	wdmCreateMap(dshipX, dshipZ, shipAY);
}

void wdmOpenLocation()
{
	//Проскочим ненужные локации
	if(!TestRef(loadedLocation)) return;
	if(!IsEntity(loadedLocation)) return;
	if(!CheckAttribute(loadedLocation, "worldmap")) return;
	//Переберём все острова
	aref islands;
	makearef(islands, worldMap.islands);
	int num = GetAttributesNum(islands);
	for(int i = 0; i < num; i++)
	{
		aref isl = GetAttributeN(islands, i);
		if(!CheckAttribute(isl, "locations")) continue;
		aref locations;
		makearef(locations, isl.locations);
		wdmOpenIslandLocation(locations, loadedLocation.worldmap);
	}
}

void wdmOpenIslandLocation(aref locations, string name)
{
	//Переберём все города
	int num = GetAttributesNum(locations);
	for(int i = 0; i < num; i++)
	{
		aref loc = GetAttributeN(locations, i);
		if(!CheckAttribute(loc, "name")) continue;
		if(loc.name == name)
		{
			loc.visible = "true";
			if(CheckAttribute(loc, "label"))
			{
				loc.label.visible = "true";
			}
		}
	}
}

// WM NK wdmap/island interop 05-05-03
string wdmGetIslandNameFromID(string islandid)
{
	if(islandid == "") return islandid;
	int idx = FindIsland(islandid);
	if(idx >= 0 && idx < ISLANDS_QUANTITY)
	{
		ref island = GetIslandByIndex(idx);
		if(CheckAttribute(island,"worldmap")) return island.worldmap;
		return islandid;
	}
	return "";
}
// WM <--

// KK -->
void wdmSetIcon(string town, string new_name, int iNation)
{
	int island_idx, i, num;
	string island, wdmisland;
	aref wdmislandlocs, loc;

	if (!TownExist(town)) return;
	if (iNation >= NATIONS_QUANTITY) return;
	if (new_name == "" && iNation < -1) return;

	island = GetIslandIDFromTown(town);
	island_idx = FindIsland(island);
	if (island_idx < 0) return;
	wdmisland = wdmGetIslandNameFromID(island);
	if (!CheckAttribute(worldMap, "islands." + wdmisland + ".locations")) return;
	makearef(wdmislandlocs, worldMap.islands.(wdmisland).locations);
	num = GetAttributesNum(wdmislandlocs);
	for (i = 0; i < num; i++) {
		loc = GetAttributeN(wdmislandlocs, i);
		if (!CheckAttribute(loc, "name")) continue;
		if (loc.name == town) {
			if (IsIslandDisabled(island)) {
				trace("wdmSetIcon: island "+island+" is disabled.");
				loc.modelName = "";
				loc.label.text = "";
				loc.label.icon = -1;
				DeleteAttribute(loc, "real");
			} else {
				if (new_name != "") loc.label.text = new_name;
				loc.label.icon = wdmTownFlag(iNation);
			}
			return;
		}
	}
}

int wdmTownFlag(int iNation)
{
	if (iNation == PERSONAL_NATION)	return 8;
	if (iNation >= 0 && iNation < NATIONS_QUANTITY) return GetFlagIndex(iNation);
	return -1;
}

bool wdmGetShorePos(string locid, ref x, ref z)
{
	x = 0.0;
	z = 0.0;
	int locidx = FindLocation(locid);
	if (locidx < 0) return false;
	string wrlmapcity = locid;
	if (CheckAttribute(&Locations[locidx], "worldmap")) wrlmapcity = Locations[locidx].worldmap;
	string island = Locations[locidx].island;
	aref labels_attr;
	makearef(labels_attr, worldmap.labels);
	for (int i = 0; i < GetAttributesNum(labels_attr); i++) {
		aref label_attr = GetAttributeN(labels_attr, i);
		string label_name = GetAttributeName(label_attr);
		if (label_attr.id == wrlmapcity) {
			if (CheckAttribute(label_attr, "position.x") && CheckAttribute(label_attr, "position.z") ) {
				x = stf(label_attr.position.x);
				z = stf(label_attr.position.z);
				return true;
			}
			break;
		}
	}
	return false;
}

float wdmGetDays(int year, int month, int day, int hour)
{
	//РЎС‡РёС‚Р°РµРј РґРЅРё РїРѕ РіРѕРґР°Рј
	if(year < 0) year = 0;
	if(year > 3000) year = 3000;
	year = year*365;
	//РЎС‡РёС‚Р°РµРј С†РµР»С‹Рµ РґРЅРё
	for(int i = 1; i < month; i++)
	{
		day = day + GetMonthDays(i, year);
	}
	//РЎС‡РёС‚Р°РµРј РїРѕР»РЅС‹Рµ РґРЅРё
	float days = year + day + (hour/24.0);
	return days;
}

void wdmMarkDeleteEncounters()
{
	// Getting the date
	int year = sti(worldMap.date.year);
	int month = sti(worldMap.date.month);
	int day = sti(worldMap.date.day);
	int hour = sti(worldMap.date.hour);
	float days = wdmGetDays(year, month, day, hour);
	int encYear, encMonth, encDay, encHour;
	// We go through all the encounters, marking for deletion
	aref encs;
	makearef(encs, worldMap.encounters);
	int num = GetAttributesNum(encs);
	for(int i = 0; i < num; i++)
	{
		aref enc = GetAttributeN(encs, i);
		if(CheckAttribute(enc, "Quest") != 0)
		{
			continue;
		}

		if (wdmNoClearEncountersTable == false) {
			enc.needDelete = "Worldmap reload";
			continue;
		}

		bool deleteMe = false;
		if(CheckAttribute(enc, "year") != 0)
		{
			encYear = sti(enc.year);
		}else{
			deleteMe = true;
		}
		if(CheckAttribute(enc, "month") != 0)
		{
			encMonth = sti(enc.month);
		}else{
			deleteMe = true;
		}
		if(CheckAttribute(enc, "day") != 0)
		{
			encDay = sti(enc.day);
		}else{
			deleteMe = true;
		}
		if(CheckAttribute(enc, "hour") != 0)
		{
			encHour = sti(enc.hour);
		}else{
			deleteMe = true;
		}

		float deltaDays = 0;
		float encounter_days = wdmGetDays(encYear, encMonth, encDay, encHour);

		if(deleteMe != true)
		{
			deltaDays = encounter_days - days;
			if(deltaDays < 0)
			{
				deltaDays = -deltaDays;
			}
			if(deltaDays > 1.0)
			{
				deleteMe = true;
			}
		}
		if(deleteMe != false)
		{
			if (encounter_days > 0) {
				Trace("Marked (" + enc.type + ") encounter for deletion after " + deltaDays + " days.");
			}
			else {
				Trace("Marked (" + enc.type + ") encounter for deletion.");
			}
			enc.needDelete = "Time delete";
		}
	}
}

void wdmInit()
{
	if(LoadSegment("worldmap\worldmap_init.c"))
	{
		wdmInitWorldMap();
		UnloadSegment("worldmap\worldmap_init.c");
	}

//	if (!IsEntity(&worldMap)) {
//		CreateEntity(&worldMap,"worldmap");
//		DeleteClass(&worldMap);
//	}
}

void DestroyWorldmap() {
	DeleteClass(&worldMap);
	worldMap.isLoaded = "false";
}
