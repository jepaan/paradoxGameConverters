#include <fstream>
#include <sys/stat.h>
#include <io.h>
#include "Parsers\Parser.h"
#include "Log.h"
#include "EU3World/EU3World.h"
#include "V2World/V2World.h"
#include "V2World/V2Factory.h"
#include "V2World/V2TechSchools.h"
#include "V2World/V2LeaderTraits.h"
#include "Configuration.h"



int main(int argc, char * argv[]) //changed from TCHAR, no use when everything else in program is in ASCII...
{
	initLog();

	Object*	obj;				// generic object
	ifstream	read;				// ifstream for reading files


	//Get V2 install location
	string V2Loc = Configuration::getV2Path();
	struct stat st;
	if (V2Loc.empty() || (stat(V2Loc.c_str(), &st) != 0))
	{
		log("No Victoria 2 path was specified in configuration.txt, or the path was invalid.  A valid path must be specified.\n");
		printf("No Victoria 2 path was specified in configuration.txt, or the path was invalid.  A valid path must be specified.\n");
		return (-2);
	}

	//Get EU3 install location
	string EU3Loc = Configuration::getEU3Path();
	if (EU3Loc.empty() || (stat(EU3Loc.c_str(), &st) != 0))
	{
		log("No Europa Universalis 3 path was specified in configuration.txt, or the path was invalid.  A valid path must be specified.\n");
		printf("No Europa Universalis 3 path was specified in configuration.txt, or the path was invalid.  A valid path must be specified.\n");
		return (-2);
	}


	//Get Input EU3 save 
	string inputFilename("input.eu3");
	if (argc >= 2)
	{
		inputFilename = argv[1];
		log("Using input file %s.\n", inputFilename.c_str());
		printf("Using input file %s.\n", inputFilename.c_str());
	}
	else
	{
		log("No input file given, defaulting to input.eu3\n");
		printf("No input file given, defaulting to input.eu3\n");
	}

	//get output name
	int slash = inputFilename.find_last_of("\\");
	int length = inputFilename.find_first_of(".") - slash - 1;
	string outputName = inputFilename.substr(slash + 1, length);
	int dash = outputName.find_first_of('-');
	while (dash != string::npos)
	{
		outputName.replace(dash, 1, "_");
		dash = outputName.find_first_of('-');
	}
	Configuration::setOutputName(outputName);
	log("Using output name %s\n", outputName.c_str());


	// Parse EU3 Save
	log("Importing EU3 save.\n");
	printf("Importing EU3 save.\n");
	log("\tParsing save.\n");
	printf("\tParsing save.\n");
	obj = doParseFile(inputFilename.c_str());
	if (obj == NULL)
	{
		log("Could not parse file %s\n", inputFilename.c_str());
		exit(-1);
	}
	log("\tExtracting data.\n");
	printf("\tExtracting data.\n");
	EU3World sourceWorld(obj);


	// Figure out what EU3 gametype we're using
	WorldType game = sourceWorld.getWorldType();
	switch (game)
	{
		case VeryOld:
			printf("	Error: EU3 game appears to be from an old version; only IN, HttT, and DW are supported.\n");
			log("	Error: EU3 game appears to be from an old version; only IN, HttT, and DW are supported.\n");
			exit(1);
		case InNomine:
			printf("	Game type is: EU3 In Nomine.  EXPERIMENTAL.\n");
			log("	Game type is: EU3 In Nomine.  EXPERIMENTAL.\n");
			break;
		case HeirToTheThrone:
			printf("	Game type is: EU3 Heir to the Throne.\n");
			log("	Game type is: EU3 Heir to the Throne.\n");
			break;
		case DivineWind:
			printf("	Game type is: EU3 Divine Wind.\n");
			log("	Game type is: EU3 Divine Wind.\n");
			break;
		default:
			printf("	Error: Could not determine savegame type.\n");
			log("	Error: Could not determine savegame type.\n");
			exit(1);
	}


	// Resolve unit types
	log("Resolving unit types.\n");
	printf("Resolving unit types.\n");
	RegimentTypeMap rtm;
	read.open("unit_strength.txt");
	if (read.is_open())
	{
		read.close();
		read.clear();
		log("\tReading unit strengths from unit_strength.txt\n");
		obj = doParseFile("unit_strength.txt");
		if (obj == NULL)
		{
			log("Could not parse file unit_strength.txt\n");
			exit(-1);
		}
		for (int i = 0; i < num_reg_categories; ++i)
		{
			AddCategoryToRegimentTypeMap(obj, (RegimentCategory)i, RegimentCategoryNames[i], rtm);
		}
	}
	else
	{
		log("	Reading unit strengths from EU3 installation folder\n");
		struct _finddata_t unitFileData;
		intptr_t fileListing;
		if ( (fileListing = _findfirst( (EU3Loc + "\\common\\units\\*.txt").c_str(), &unitFileData)) == -1L)
		{
			log("	Could not open units directory.\n");
			return -1;
		}
		do
		{
			if (strcmp(unitFileData.name, ".") == 0 || strcmp(unitFileData.name, "..") == 0 )
			{
				continue;
			}
			string unitFilename = unitFileData.name;
			string unitName = unitFilename.substr(0, unitFilename.find_first_of('.'));
			AddUnitFileToRegimentTypeMap((EU3Loc + "\\common\\units"), unitName, rtm);
		} while(_findnext(fileListing, &unitFileData) == 0);
		_findclose(fileListing);
	}
	read.close();
	read.clear();
	sourceWorld.resolveRegimentTypes(rtm);


	// Merge nations
	log("Merging nations.\n");
	printf("Merging nations.\n");
	obj = doParseFile("merge_nations.txt");
	if (obj == NULL)
	{
		log("Could not parse file merge_nations.txt\n");
		exit(-1);
	}
	mergeNations(sourceWorld, obj);


	// Parse V2 input file
	log("Parsing Vicky2 data.\n");
	printf("Parsing Vicky2 data.\n");
	V2World destWorld(V2Loc);


	// Construct factory factory
	log("Determining factory allocation rules.\n");
	printf("Determining factory allocation rules.\n");
	V2FactoryFactory factoryBuilder(V2Loc);


	// Parse province mappings
	log("Parsing province mappings.\n");
	printf("Parsing province mappings.\n");
	const char* mappingFile = "province_mappings.txt";
	// if province_mappings.txt exists, always use it (e.g. as an override for mods);
	// otherwise use the appropriate mapping for the game type
	FILE* testfp = NULL;
	if (fopen_s(&testfp, mappingFile, "rb") != 0)
	{
		if (game == HeirToTheThrone)
			mappingFile = "HttT_province_mappings.txt";
		else if (game == DivineWind)
			mappingFile = "DW_province_mappings.txt";
		else if (game == InNomine)
			mappingFile = "IN_province_mappings.txt";
	}
	else
	{
		fclose(testfp);
	}
	obj = doParseFile(mappingFile);
	if (obj == NULL)
	{
		log("Could not parse file %s\n", mappingFile);
		exit(-1);
	}
	provinceMapping provinceMap;
	inverseProvinceMapping inverseProvinceMap;
	initProvinceMap(obj, provinceMap, inverseProvinceMap);
	sourceWorld.checkAllProvincesMapped(inverseProvinceMap);


	// Get list of blocked nations
	log("Getting blocked V2 nations.\n");
	printf("Getting blocked V2 nations.\n");
	obj = doParseFile("blocked_nations.txt");
	if (obj == NULL)
	{
		log("Could not parse file blocked_nations.txt\n");
		exit(-1);
	}
	vector<string> blockedNations = processBlockedNations(obj);

	// Get country mappings
	log("Parsing country mappings.\n");
	printf("Parsing country mappings.\n");
	initParser();
	obj = doParseFile("country_mappings.txt");	
	if (obj == NULL)
	{
		log("Could not parse file country_mappings.txt\n");
		exit(-1);
	}

	// Map EU3 nations to V2 nations
	log("Mapping EU3 nations to V2 nations.\n");
	printf("Mapping EU3 nations to V2 nations.\n");
	removeEmptyNations(sourceWorld);
	if (Configuration::getRemovetype() == "dead")
	{
		removeDeadLandlessNations(sourceWorld);
	}
	else if (Configuration::getRemovetype() == "all")
	{
		removeLandlessNations(sourceWorld);
	}
	countryMapping countryMap;
	int leftoverNations = initCountryMap(countryMap, sourceWorld, destWorld, blockedNations, obj);
	if (leftoverNations == -1)
	{
		return 1;
	}
	else if (leftoverNations > 0)
	{
		log("\tToo many EU3 nations (%d). Removing dead landless nations.\n", leftoverNations);
		printf("\tToo many EU3 nations (%d). Removing dead landless nations.\n", leftoverNations);
		removeDeadLandlessNations(sourceWorld);
		leftoverNations = initCountryMap(countryMap, sourceWorld, destWorld, blockedNations, obj);
	}
	if (leftoverNations == -1)
	{
		return 1;
	}
	else if (leftoverNations > 0)
	{
		log("\tToo many EU3 nations (%d). Removing older landless nations.\n", leftoverNations);
		printf("\tToo many EU3 nations (%d). Removing older landless nations.\n", leftoverNations);
		removeOlderLandlessNations(sourceWorld, leftoverNations + blockedNations.size());
		leftoverNations = initCountryMap(countryMap, sourceWorld, destWorld, blockedNations, obj);
	}
	if (leftoverNations == -1)
	{
		return 1;
	}
	else if (leftoverNations > 0)
	{
		log("\tToo many EU3 nations (%d). Removing all landless nations.\n", leftoverNations);
		printf("\tToo many EU3 nations (%d). Removing all landless nations.\n", leftoverNations);
		removeLandlessNations(sourceWorld);
		leftoverNations = initCountryMap(countryMap, sourceWorld, destWorld, blockedNations, obj);
	}

	// Get adjacencies
	log("Importing adjacencies\n");
	printf("Importing adjacencies\n");
	adjacencyMapping adjacencyMap = initAdjacencyMap();
	
	// Generate region mapping
	log("Parsing region structure.\n");
	printf("Parsing region structure.\n");
	obj = doParseFile( (V2Loc + "\\map\\region.txt").c_str() );
	if (obj == NULL)
	{
		log("Could not parse file %s\n", (V2Loc + "\\map\\region.txt").c_str());
		exit(-1);
	}
	if (obj->getLeaves().size() < 1)
	{
		log("Error: Could not parse region.txt.\n");
		printf("Error: Could not parse region.txt.\n");
		return 1;
	}
	stateMapping		stateMap;
	stateIndexMapping stateIndexMap;
	initStateMap(obj, stateMap, stateIndexMap);


	// Parse Culture Mappings
	log("Parsing culture mappings.\n");
	printf("Parsing culture mappings.\n");
	obj = doParseFile("cultureMap.txt");
	if (obj == NULL)
	{
		log("Could not parse file cultureMap.txt\n");
		exit(-1);
	}
	if (obj->getLeaves().size() < 1)
	{
		log("Error: Failed to parse cultureMap.txt.\n");
		printf("Error: Failed to parse cultureMap.txt.\n");
		return 1;
	}
	cultureMapping cultureMap;
	cultureMap = initCultureMap(obj->getLeaves()[0]);

	string EU3Mod = Configuration::getEU3Mod();
	unionCulturesMap unionCultures;
	if (EU3Mod != "")
	{
		string modCultureFile = Configuration::getEU3Path() + "\\mod\\" + EU3Mod + "\\common\\cultures.txt";
		if ((stat(modCultureFile.c_str(), &st) != 0))
		{
			obj = doParseFile(modCultureFile.c_str());
			if (obj == NULL)
			{
				log("Could not parse file %s\n", modCultureFile.c_str());
				exit(-1);
			}
			if (obj->getLeaves().size() < 1)
			{
				log("Error: Failed to parse cultures.txt.\n");
				printf("Error: Failed to parse cultures.txt.\n");
				return 1;
			}
			unionCultures = initUnionCultures(obj);
		}
	}
	if (unionCultures.size() == 0)
	{
		obj = doParseFile((EU3Loc + "\\common\\cultures.txt").c_str());
		if (obj == NULL)
		{
			log("Could not parse file %s\n", (EU3Loc + "\\common\\cultures.txt").c_str());
			exit(-1);
		}
		if (obj->getLeaves().size() < 1)
		{
			log("Error: Failed to parse cultures.txt.\n");
			printf("Error: Failed to parse cultures.txt.\n");
			return 1;
		}
		unionCultures = initUnionCultures(obj);
	}

	// Parse Religion Mappings
	log("Parsing religion mappings.\n");
	printf("Parsing religion mappings.\n");
	obj = doParseFile("religionMap.txt");
	if (obj == NULL)
	{
		log("Could not parse file religionMap.txt\n");
		exit(-1);
	}
	if (obj->getLeaves().size() < 1)
	{
		log("Error: Failed to parse religionMap.txt.\n");
		printf("Error: Failed to parse religionMap.txt.\n");
		return 1;
	}
	religionMapping religionMap;
	religionMap = initReligionMap(obj->getLeaves()[0]);


	//Parse unions mapping
	log("Parsing union mappings.\n");
	printf("Parsing union mappings.\n");
	obj = doParseFile("unions.txt");
	if (obj == NULL)
	{
		log("Could not parse file unions.txt\n");
		exit(-1);
	}
	if (obj->getLeaves().size() < 1)
	{
		log("Error: Failed to parse unions.txt.\n");
		printf("Error: Failed to parse unions.txt.\n");
		return 1;
	}
	unionMapping unionMap;
	unionMap = initUnionMap(obj->getLeaves()[0]);


	//Parse government mapping
	log("Parsing governments mappings.\n");
	printf("Parsing governments mappings.\n");
	initParser();
	obj = doParseFile("governmentMapping.txt");
	if (obj == NULL)
	{
		log("Could not parse file governmentMapping.txt\n");
		exit(-1);
	}
	governmentMapping governmentMap;
	governmentMap = initGovernmentMap(obj->getLeaves()[0]);


	//Parse tech schools
	log("Parsing tech schools.\n");
	printf("Parsing tech schools.\n");
	initParser();
	obj = doParseFile("blocked_tech_schools.txt");
	if (obj == NULL)
	{
		log("Could not parse file blocked_tech_schools.txt\n");
		exit(-1);
	}
	vector<string> blockedTechSchools;
	blockedTechSchools = initBlockedTechSchools(obj);
	initParser();
	obj = doParseFile( (V2Loc + "\\common\\technology.txt").c_str() );
	if (obj == NULL)
	{
		log("Could not parse file %s\n", (V2Loc + "\\common\\technology.txt").c_str());
		exit(-1);
	}
	vector<techSchool> techSchools;
	techSchools = initTechSchools(obj, blockedTechSchools);


	// Get Leader traits
	log("Getting leader traits.\n");
	printf("Getting leader traits.\n");
	V2LeaderTraits lt;
	map<int, int> leaderIDMap; // <EU3, V2>


	// Convert
	printf("Converting countries.\n");
	log("Converting countries.\n");
	destWorld.convertCountries(sourceWorld, countryMap, cultureMap, unionCultures, religionMap, governmentMap, inverseProvinceMap, techSchools, leaderIDMap, lt);
	printf("Converting diplomacy.\n");
	log("Converting diplomacy.\n");
	destWorld.convertDiplomacy(sourceWorld, countryMap);
	printf("Converting provinces.\n");
	log("Converting provinces.\n");
	destWorld.convertProvinces(sourceWorld, provinceMap, countryMap, cultureMap, religionMap, stateIndexMap);
	printf("Setting colonies\n");
	log("Setting colonies\n");
	destWorld.setupColonies(adjacencyMap);
	printf("Creating states.\n");
	log("Creating states.\n");
	destWorld.setupStates(stateMap);
	printf("Setting unciv reforms.\n");
	log("Setting unciv reforms.\n");
	destWorld.convertUncivReforms();
	printf("Creating pops.\n");
	log("Creating pops.\n");
	//destWorld.setupPops(sourceWorld);
	printf("Adding unions.\n");
	log("Adding unions.\n");
	destWorld.addUnions(unionMap);
	printf("Converting armies and navies.\n");
	log("Converting armies and navies.\n");
	destWorld.convertArmies(sourceWorld, inverseProvinceMap, leaderIDMap);
	printf("Converting techs.\n");
	log("Converting techs.\n");
	destWorld.convertTechs(sourceWorld);
	printf("Allocating starting factories.\n");
	log("Allocating starting factories.\n");
	destWorld.allocateFactories(sourceWorld, factoryBuilder);

	// Generate Mod Directory Structure
	printf("Outputting mod.\n");
	log("Outputting mod.\n");
	printf("\tGenerating mod directory structure.\n");
	log("\tGenerating mod directory structure.\n");
	/*CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName).c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\common").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\common\\countries").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\gfx").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\gfx\\flags").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\history").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\history\\countries").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\history\\diplomacy").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\history\\pops").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\history\\pops\\1836.1.1").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\history\\provinces").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\africa").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\asia").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\australia").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\austria").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\balkan").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\canada").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\carribean").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\central asia").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\china").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\france").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\germany").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\india").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\indonesia").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\italy").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\japan").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\low countries").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\mexico").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\pacific island").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\portugal").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\scandinavia").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\south america").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\soviet").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\spain").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\united kingdom").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\" + OutputFolderName + "\\history\\provinces\\usa").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\history\\units").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\history\\wars").c_str(), NULL);
	CreateDirectory((V2Loc + "\\mods\\"+ OutputFolderName + "\\localisation").c_str(), NULL);*/

	// Output results
	printf("Outputting mod\n");
	log("Outputting mod\n");
	system("%systemroot%\\System32\\xcopy blankMod output /E /Q /Y /I");
	FILE* modFile;
	if (fopen_s(&modFile, ("Output\\" + Configuration::getOutputName() + ".mod").c_str(), "w") != 0)
	{
		log("\tError: Could not create .mod file\n");
		exit(-1);
	}
	fprintf(modFile, "name = \"Converted - %s\"\n", Configuration::getOutputName().c_str());
	fprintf(modFile, "path = \"mod/%s\"\n", Configuration::getOutputName().c_str());
	fprintf(modFile, "replace = \"history/provinces\"\n");
	fclose(modFile);
	string renameCommand = "move /Y output\\output output\\" + Configuration::getOutputName();
	system(renameCommand.c_str());
	destWorld.output();

	closeLog();
	return 0;
}

