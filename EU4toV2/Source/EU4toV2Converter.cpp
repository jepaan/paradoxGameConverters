/*Copyright (c) 2014 The Paradox Game Converters Project

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/

#include "OSCompatabilityLayer.h"

#include <fstream>
#include <sys/io.h>
#include <stdexcept>
#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <boost/filesystem.hpp>

#include "Configuration.h"
#include "Log.h"
#include "Parsers/Parser.h"
#include "EU4World/EU4World.h"
#include "EU4World/EU4Religion.h"
#include "EU4World/EU4Localisation.h"
#include "V2World/V2World.h"
#include "V2World/V2Factory.h"
#include "V2World/V2TechSchools.h"
#include "V2World/V2LeaderTraits.h"

// Converts the given EU4 save into a V2 mod.
// Returns 0 on success or a non-zero failure code on error.
int ConvertEU4ToV2(const std::string& EU4SaveFileName)
{
	char curDir[MAX_PATH];
	Utils::GetCurrentDirectory(MAX_PATH, curDir);
	LOG(LogLevel::Debug) << "Current directory is " << curDir;

	Configuration::getInstance();

	FILE* saveFile = fopen(EU4SaveFileName.c_str(), "r");
	if (saveFile == NULL)
	{
		LOG(LogLevel::Error) << "Could not open save! Exiting!";
		exit(-1);
	}
	else
	{
		char buffer[7];
		fread(buffer, 1, 7, saveFile);
		if ((buffer[0] == 'P') && (buffer[1] == 'K'))
		{
			LOG(LogLevel::Error) << "Saves must be uncompressed to be converted.";
			exit(-1);
		}
		else if ((buffer[0] = 'E') && (buffer[1] == 'U') && (buffer[2] == '4') && (buffer[3] = 'b') && (buffer[4] == 'i') && (buffer[5] == 'n') && (buffer[6] == 'M'))
		{
			LOG(LogLevel::Error) << "Ironman saves cannot be converted.";
			exit(-1);
		}
	}

	// Get V2 install location
	LOG(LogLevel::Info) << "Get V2 Install Path";
	string V2Loc = Configuration::getV2Path();	// the V2 install location as stated in the configuration file
	if (!boost::filesystem::exists(V2Loc) || !boost::filesystem::is_directory(V2Loc)) 
	{
		LOG(LogLevel::Error) << "No Victoria 2 path was specified in configuration.txt, or the path was invalid ";
		return (-1);
	}
	else
	{
		LOG(LogLevel::Debug) << "Victoria 2 install path is " << V2Loc;
	}

	// Get V2 Documents Directory
	LOG(LogLevel::Debug) << "Get V2 Documents directory";
	string V2DocLoc = Configuration::getV2DocumentsPath();	// the V2 My Documents location as stated in the configuration file
	if (!boost::filesystem::exists(V2DocLoc) || !boost::filesystem::is_directory(V2DocLoc)) 
	{
		LOG(LogLevel::Error) << "No Victoria 2 documents directory was specified in configuration.txt, or the path was invalid";
		return (-1);
	}
	else
	{
		LOG(LogLevel::Debug) << "Victoria 2 documents directory is " << V2DocLoc;
	}

	// Get EU4 install location
	LOG(LogLevel::Debug) << "Get EU4 Install Path";
	string EU4Loc = Configuration::getEU4Path();	// the EU4 install location as stated in the configuration file
	if (!boost::filesystem::exists(EU4Loc) || !boost::filesystem::is_directory(EU4Loc)) 
	{
		LOG(LogLevel::Error) << "No Europa Universalis 4 path was specified in configuration.txt, or the path was invalid";
		return (-1);
	}
	else
	{
		LOG(LogLevel::Debug) << "EU4 install path is " << EU4Loc;
	}

	// Get EU4 Mod directory
	map<string, string> possibleMods; // name, path
	LOG(LogLevel::Debug) << "Get EU4 Mod Directory";
	string EU4DocumentsLoc = Configuration::getEU4DocumentsPath();	// the EU4 My Documents location as stated in the configuration file
	if (!boost::filesystem::exists(EU4DocumentsLoc) || !boost::filesystem::is_directory(EU4DocumentsLoc))
	{
		LOG(LogLevel::Error) << "No Europa Universalis 4 documents directory was specified in configuration.txt, or the path was invalid";
		return (-1);
	}
	else
	{
		LOG(LogLevel::Debug) << "EU4 Documents directory is " << EU4DocumentsLoc;
		set<string> fileNames;
		Utils::GetAllFilesInFolder(EU4DocumentsLoc + "/mod", fileNames);
		for (set<string>::iterator itr = fileNames.begin(); itr != fileNames.end(); itr++)
		{
			const int pos = itr->find_last_of('.');	// the position of the last period in the filename
			if (itr->substr(pos, itr->length()) == ".mod")
			{
				Object* modObj = doParseFile((*itr).c_str());	// the parsed mod file

				string name;	// the name of the mod
				vector<Object*> nameObj = modObj->getValue("name");
				if (nameObj.size() > 0)
				{
					name = nameObj[0]->getLeaf();
				}
				else
				{
					name = "";
				}

				string path;	// the path of the mod
				vector<Object*> dirObjs = modObj->getValue("path");	// the possible paths of the mod
				if (dirObjs.size() > 0)
				{
					path = dirObjs[0]->getLeaf();
				}
				else
				{
					vector<Object*> dirObjs = modObj->getValue("archive");	// the other possible paths of the mod (if its zipped)
					if (dirObjs.size() > 0)
					{
						path = dirObjs[0]->getLeaf();
					}
				}

				if ((name != "") && (path != ""))
				{
					possibleMods.insert(make_pair(name, EU4DocumentsLoc + "/" + path));
					Log(LogLevel::Debug) << "\t\tFound a mod named " << name << " claiming to be at " << EU4DocumentsLoc << "/" << path;
				}
			}
		}
	}

	// Get CK2 Export directory
	LOG(LogLevel::Debug) << "Get CK2 Export Directory";
	string CK2ExportLoc = Configuration::getCK2ExportPath();		// the CK2 converted mods location as stated in the configuration file
	if (!boost::filesystem::exists(CK2ExportLoc) || !boost::filesystem::is_directory(CK2ExportLoc))
	{
		LOG(LogLevel::Warning) << "No Crusader Kings 2 mod directory was specified in configuration.txt, or the path was invalid - this will cause problems with CK2 converted saves";
	}
	else
	{
		LOG(LogLevel::Debug) << "CK2 export directory is " << CK2ExportLoc;
		set<string> fileNames;
		Utils::GetAllFilesInFolder(CK2ExportLoc, fileNames);
		for (set<string>::iterator itr = fileNames.begin(); itr != fileNames.end(); itr++)
		{
			const int pos = itr->find_last_of('.');	// the last period in the filename
			if ((pos != string::npos) && (itr->substr(pos, itr->length()) == ".mod"))
			{
				Object* modObj = doParseFile((CK2ExportLoc + "/" + *itr).c_str());	// the parsed mod file
				vector<Object*> nameObj = modObj->getValue("name");
				string name;
				if (nameObj.size() > 0)
				{
					name = nameObj[0]->getLeaf();
				}

				string path;	// the path of the mod
				vector<Object*> dirObjs = modObj->getValue("user_dir");	// the possible paths for the mod
				if (dirObjs.size() > 0)
				{
					path = dirObjs[0]->getLeaf();
				}
				else
				{
					vector<Object*> dirObjs = modObj->getValue("archive");	// the other possible paths for the mod (if it's zipped)
					if (dirObjs.size() > 0)
					{
						path = dirObjs[0]->getLeaf();
					}
				}

				if (path != "")
				{
					possibleMods.insert(make_pair(name, CK2ExportLoc + "/" + path));
				}
			}
		}
	}

	//get output name
	const int slash	= EU4SaveFileName.find_last_of("/");				// the last slash in the save's filename
	string outputName	= EU4SaveFileName.substr(slash + 1, EU4SaveFileName.length());
	const int length	= outputName.find_first_of(".");						// the first period after the slash
	outputName			= outputName.substr(0, length);						// the name to use to output the mod
	int dash = outputName.find_first_of('-');									// the first (if any) dask in the output name
	while (dash != string::npos)
	{
		outputName.replace(dash, 1, "_");
		dash = outputName.find_first_of('-');
	}
	int space = outputName.find_first_of(' ');	// the first space (if any) in the output name
	while (space != string::npos)
	{
		outputName.replace(space, 1, "_");
		space = outputName.find_first_of(' ');
	}
	Configuration::setOutputName(outputName);
	LOG(LogLevel::Info) << "Using output name " << outputName;

	string outputFolder = string(curDir) + "/output/" + Configuration::getOutputName();
	if (Utils::doesFolderExist(outputFolder.c_str()))
	{
		LOG(LogLevel::Error) << "Output folder " << Configuration::getOutputName() << " already exists! Clear the output folder before running again!";
		exit(0);
	}


	LOG(LogLevel::Info) << "* Importing EU4 save *";

	//	Parse EU4 Save
	LOG(LogLevel::Info) << "Parsing save";
	Object* saveObj = doParseFile(EU4SaveFileName.c_str());
	if (saveObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file " << EU4SaveFileName;
		exit(-1);
	}

	// Get EU4 Mod
	LOG(LogLevel::Debug) << "Get EU4 Mod";
	vector<string> fullModPaths;	// the full pathnames for used mods
	vector<Object*> modObj = saveObj->getValue("mod_enabled");	// the used mods
	if (modObj.size() > 0)
	{
		string modString = modObj[0]->getLeaf();	// the names of all the mods
		while (modString != "")
		{
			string newMod;	// the corrected name of the mod
			const int firstQuote = modString.find("\"");	// the location of the first quote, defining the start of a mod name
			if (firstQuote == std::string::npos)
			{
				newMod.clear();
				modString.clear();
			}
			else
			{
				const int secondQuote = modString.find("\"", firstQuote + 1);	// the location of the second quote, defining the end of a mod name
				if (secondQuote == std::string::npos)
				{
					newMod.clear();
					modString.clear();
				}
				else
				{
					newMod = modString.substr(firstQuote + 1, secondQuote - firstQuote - 1);
					modString = modString.substr(secondQuote + 1, modString.size());
				}
			}

			if (newMod != "")
			{
				map<string, string>::iterator modItr = possibleMods.find(newMod);
				if (modItr != possibleMods.end())
				{
					string newModPath = modItr->second;	// the path for this mod
					if (!boost::filesystem::exists(newModPath) || boost::filesystem::is_directory(newModPath))
					{
						LOG(LogLevel::Error) << newMod << " could not be found in the specified mod directory - a valid mod directory must be specified. Tried " << newModPath;
						exit(-1);
					}
					else
					{
						LOG(LogLevel::Debug) << "EU4 Mod is at " << newModPath;
						fullModPaths.push_back(newModPath);
					}
				}
				else
				{
					LOG(LogLevel::Error) << "No path could be found for " << newMod;
					exit(-1);
				}
			}
		}
	}

	// Read all localisations.
	LOG(LogLevel::Info) << "Reading localisation";
	EU4Localisation localisation;
	localisation.ReadFromAllFilesInFolder(Configuration::getEU4Path() + "/localisation");
	for (vector<string>::iterator itr = fullModPaths.begin(); itr != fullModPaths.end(); itr++)
	{
		LOG(LogLevel::Debug) << "Reading mod localisation";
		localisation.ReadFromAllFilesInFolder(*itr + "/localisation");
	}

	// Read Idea Effects
	LOG(LogLevel::Info) << "Getting idea effects";
	Object* ideaObj = doParseFile("idea_effects.txt");
	if (ideaObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file idea_effects.txt";
		exit(-1);
	}
	map<string, int>					armyInvIdeas;
	map<string, int>					commerceInvIdeas;
	map<string, int>					cultureInvIdeas;
	map<string, int>					industryInvIdeas;
	map<string, int>					navyInvIdeas;
	map<string, double>				armyTechIdeas;
	map<string, double>				commerceTechIdeas;
	map<string, double>				cultureTechIdeas;
	map<string, double>				industryTechIdeas;
	map<string, double>				navyTechIdeas;
	map<string, double>				UHLiberalIdeas;
	map<string, double>				UHReactionaryIdeas;
	vector< pair<string, int> >	literacyIdeas;
	map<string, int>					orderIdeas;
	map<string, int>					libertyIdeas;
	map<string, int>					equalityIdeas;
	initIdeaEffects(ideaObj, armyInvIdeas, commerceInvIdeas, cultureInvIdeas, industryInvIdeas, navyInvIdeas, armyTechIdeas, commerceTechIdeas, cultureTechIdeas, industryTechIdeas, navyTechIdeas, UHLiberalIdeas, UHReactionaryIdeas, literacyIdeas, orderIdeas, libertyIdeas, equalityIdeas);

	// Parse Culture Mappings
	LOG(LogLevel::Info) << "Parsing culture mappings";
	Object* cultureObj = doParseFile("cultureMap.txt");
	if (cultureObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file cultureMap.txt";
		exit(-1);
	}
	if (cultureObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse cultureMap.txt";
		return 1;
	}
	cultureMapping cultureMap;
	cultureMap = initCultureMap(cultureObj->getLeaves()[0]);
	Object* slaveCultureObj = doParseFile("slaveCultureMap.txt");
	if (slaveCultureObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file slaveCultureMap.txt";
		exit(-1);
	}
	if (slaveCultureObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse slaveCultureMap.txt";
		return 1;
	}
	cultureMapping slaveCultureMap;
	slaveCultureMap = initCultureMap(slaveCultureObj->getLeaves()[0]);

	// find culture groups
	unionCulturesMap			unionCultures;
	inverseUnionCulturesMap	inverseUnionCultures;
	Object* culturesObj = doParseFile( (EU4Loc + "/common/cultures/00_cultures.txt").c_str() );
	if (culturesObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file " << EU4Loc << "/common/cultures/00_cultures.txt";
		exit(-1);
	}
	if (culturesObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse 00_cultures.txt";
		return 1;
	}
	initUnionCultures(culturesObj, unionCultures, inverseUnionCultures);
	for (vector<string>::iterator itr = fullModPaths.begin(); itr != fullModPaths.end(); itr++)
	{
		if(boost::filesystem::exists(string(*itr + "/common/cultures/")) && boost::filesystem::is_directory(string(*itr + "/common/cultures/")))
		{
                        for(boost::filesystem::directory_entry& file : boost::filesystem::directory_iterator(string(*itr + "/common/cultures/")))
                        {
                          string modCultureFile(*itr + "/common/cultures/" + file.path().native());    // the path and name of the culture file in this mod
                          culturesObj = doParseFile(modCultureFile.c_str());
                          if (culturesObj == NULL)
                          {
                                  LOG(LogLevel::Error) << "Could not parse file " << modCultureFile;
                                  exit(-1);
                          }
                          initUnionCultures(culturesObj, unionCultures, inverseUnionCultures);
                        }
		}
	}

	// Construct world from EU4 save.
	LOG(LogLevel::Info) << "Building world";
	EU4World sourceWorld(saveObj, armyInvIdeas, commerceInvIdeas, cultureInvIdeas, industryInvIdeas, navyInvIdeas, inverseUnionCultures);
	sourceWorld.checkAllEU4CulturesMapped(cultureMap, inverseUnionCultures);

	// Read EU4 common\countries
	LOG(LogLevel::Info) << "Reading EU4 common/countries";
	{
		ifstream commonCountries(Configuration::getEU4Path() + "/common/country_tags/00_countries.txt");	// the data in the countries file
		sourceWorld.readCommonCountries(commonCountries, Configuration::getEU4Path());
		for (vector<string>::iterator itr = fullModPaths.begin(); itr != fullModPaths.end(); itr++)
		{
			set<string> fileNames;
			Utils::GetAllFilesInFolder(*itr + "/common/country_tags/", fileNames);
			for (set<string>::iterator fileItr = fileNames.begin(); fileItr != fileNames.end(); fileItr++)
			{
				ifstream convertedCommonCountries(*itr + "/common/country_tags/" + *fileItr);	// a stream of the data in the converted countries file
				sourceWorld.readCommonCountries(convertedCommonCountries, *itr);
			}
		}
	}

	sourceWorld.setLocalisations(localisation);

	// Resolve unit types
	LOG(LogLevel::Info) << "Resolving unit types.";
	RegimentTypeMap rtm;
	fstream read;
	read.open("unit_strength.txt");
	if (read.is_open())
	{
		read.close();
		read.clear();
		LOG(LogLevel::Info) << "\tReading unit strengths from unit_strength.txt";
		Object* unitsObj = doParseFile("unit_strength.txt");
		if (unitsObj == NULL)
		{
			LOG(LogLevel::Error) << "Could not parse file unit_strength.txt";
			exit(-1);
		}
		for (int i = 0; i < num_reg_categories; ++i)
		{
			AddCategoryToRegimentTypeMap(unitsObj, (RegimentCategory)i, RegimentCategoryNames[i], rtm);
		}
	}
	else
	{
		LOG(LogLevel::Info) << "\tReading unit strengths from EU4 installation folder";
                if(boost::filesystem::exists(string(EU4Loc + "/common/units/")) && boost::filesystem::is_directory(string(EU4Loc + "/common/units/")))
                {
                        for(boost::filesystem::directory_entry& file : boost::filesystem::directory_iterator(string(EU4Loc + "/common/units/")))
                        {
                          string unitFilename = file.path().native();
                          string unitName = unitFilename.substr(0, unitFilename.find_first_of('.'));
                          AddUnitFileToRegimentTypeMap((EU4Loc + "/common/units"), unitName, rtm);
                        }
                }
	}
	read.close();
	read.clear();
	sourceWorld.resolveRegimentTypes(rtm);


	// Merge nations
	LOG(LogLevel::Info) << "Merging nations";
	Object* mergeObj = doParseFile("merge_nations.txt");
	if (mergeObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file merge_nations.txt";
		exit(-1);
	}
	mergeNations(sourceWorld, mergeObj);

	// Parse minoruty pops map
	LOG(LogLevel::Info) << "Parsing minority pops mappings";
	Object* minoritiesObj = doParseFile("minorityPops.txt");
	if (minoritiesObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file minorityPops.txt";
		exit(-1);
	}
	if (minoritiesObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse minorityPops.txt";
		return 1;
	}
	minorityPopMapping minorityPops = initMinorityPopMap(minoritiesObj->getLeaves()[0]);

	// Parse V2 input file
	LOG(LogLevel::Info) << "Parsing Vicky2 data";
	V2World destWorld(minorityPops);
	
	// Construct factory factory
	LOG(LogLevel::Info) << "Determining factory allocation rules.";
	V2FactoryFactory factoryBuilder;

	// Parse province mappings
	LOG(LogLevel::Info) << "Parsing province mappings";
	Object* provinceMappingObj = doParseFile("province_mappings.txt");
	if (provinceMappingObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file province_mappings.txt";
		exit(-1);
	}
	provinceMapping			provinceMap;
	inverseProvinceMapping	inverseProvinceMap;
	resettableMap				resettableProvinces;
	initProvinceMap(provinceMappingObj, sourceWorld.getVersion(), provinceMap, inverseProvinceMap, resettableProvinces);
	sourceWorld.checkAllProvincesMapped(inverseProvinceMap);
	sourceWorld.setEU4WorldProvinceMappings(inverseProvinceMap);

	// Get country mappings
	LOG(LogLevel::Info) << "Getting country mappings";
	CountryMapping countryMap;
	countryMap.ReadRules("country_mappings.txt");

	// Get adjacencies
	LOG(LogLevel::Info) << "Importing adjacencies";
	adjacencyMapping adjacencyMap = initAdjacencyMap();

	// Generate continent mapping. Only the one from the last listed mod will be used
	LOG(LogLevel::Info) << "Finding Continents";
	continentMapping continentMap;
	for (vector<string>::iterator itr = fullModPaths.begin(); itr != fullModPaths.end(); itr++)
	{
		string continentFile = *itr + "/map/continent.txt";	// the path and name of the continent file
		if (boost::filesystem::exists(continentFile) && boost::filesystem::is_regular_file(continentFile))
		{
			Object* continentObject = doParseFile(continentFile.c_str());
			if ((continentObject != NULL) && (continentObject->getLeaves().size() > 0))
			{
				initContinentMap(continentObject, continentMap);
			}
		}
	}
	if (continentMap.size() == 0)
	{
		Object* continentObject = doParseFile((EU4Loc + "/map/continent.txt").c_str());
		if (continentObject == NULL)
		{
			LOG(LogLevel::Error) << "Could not parse file " << EU4Loc << "/map/continent.txt";
			exit(-1);
		}
		if (continentObject->getLeaves().size() < 1)
		{
			LOG(LogLevel::Error) << "Failed to parse continent.txt";
			return 1;
		}
		initContinentMap(continentObject, continentMap);
	}
	if (continentMap.size() == 0)
	{
		LOG(LogLevel::Warning) << "No continent mappings found - may lead to problems later";
	}
	
	// Generate region mapping
	LOG(LogLevel::Info) << "Parsing region structure";
	Object* Vic2RegionsObj;
        std::string regionPath = "./blankMod/output/map/region.txt";
	if (boost::filesystem::exists(regionPath) && boost::filesystem::is_regular_file(regionPath))
	{
		Vic2RegionsObj = doParseFile(regionPath.c_str());
		if (Vic2RegionsObj == NULL)
		{
			LOG(LogLevel::Error) << "Could not parse file ." << regionPath;
			exit(-1);
		}
	}
	else
	{
		Vic2RegionsObj = doParseFile( (V2Loc + "/map/region.txt").c_str() );
		if (Vic2RegionsObj == NULL)
		{
			LOG(LogLevel::Error) << "Could not parse file " << V2Loc << "/map/region.txt";
			exit(-1);
		}
	}
	if (Vic2RegionsObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Could not parse region.txt";
		return 1;
	}
	stateMapping		stateMap;
	stateIndexMapping	stateIndexMap;
	initStateMap(Vic2RegionsObj, stateMap, stateIndexMap);
	countryMap.readV2Regions(Vic2RegionsObj);

	// Parse EU4 Religions
	LOG(LogLevel::Info) << "Parsing EU4 religions";
	Object* religionsObj = doParseFile((EU4Loc + "/common/religions/00_religion.txt").c_str());
	if (religionsObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file " << EU4Loc << "/common/religions/00_religion.txt";
		exit(-1);
	}
	if (religionsObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse 00_religion.txt";
		return 1;
	}
	EU4Religion::parseReligions(religionsObj);
	for (vector<string>::iterator itr = fullModPaths.begin(); itr != fullModPaths.end(); itr++)
	{
		if(boost::filesystem::exists(string(*itr + "/common/religions/")) && boost::filesystem::is_directory(string(*itr + "/common/religions/")))
                {
                        for(boost::filesystem::directory_entry& file : boost::filesystem::directory_iterator(string(*itr + "/common/religions/")))
                        {
                          string modReligionFile(*itr + "/common/religions/" + file.path().native());  // the path and name of the religions file in this mod
                          if(boost::filesystem::exists(modReligionFile) && boost::filesystem::is_regular_file(modReligionFile))
                          {
                                  religionsObj = doParseFile(modReligionFile.c_str());
                                  if (religionsObj == NULL)
                                  {
                                          LOG(LogLevel::Error) << "Could not parse file " << modReligionFile;
                                          exit(-1);
                                  }
                                  EU4Religion::parseReligions(religionsObj);
                          }
                        }
                }
	}

	// Parse Religion Mappings
	LOG(LogLevel::Info) << "Parsing religion mappings";
	Object* religionMapObj = doParseFile("religionMap.txt");
	if (religionMapObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file religionMap.txt";
		exit(-1);
	}
	if (religionMapObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse religionMap.txt";
		return 1;
	}
	religionMapping religionMap;
	religionMap = initReligionMap(religionMapObj->getLeaves()[0]);
	sourceWorld.checkAllEU4ReligionsMapped(religionMap);


	// Parse unions mapping
	LOG(LogLevel::Info) << "Parsing union mappings";
	Object* unionsMapObj = doParseFile("unions.txt");
	if (unionsMapObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file unions.txt";
		exit(-1);
	}
	if (unionsMapObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse unions.txt";
		return 1;
	}
	unionMapping unionMap;
	unionMap = initUnionMap(unionsMapObj->getLeaves()[0]);


	// Parse government mapping
	LOG(LogLevel::Info) << "Parsing governments mappings";
	initParser();
	Object* governmentMapObj = doParseFile("governmentMapping.txt");
	if (governmentMapObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file governmentMapping.txt";
		exit(-1);
	}
	governmentMapping governmentMap;
	governmentMap = initGovernmentMap(governmentMapObj->getLeaves()[0]);


	// Parse tech schools
	LOG(LogLevel::Info) << "Parsing tech schools.";
	initParser();
	Object* techSchoolObj = doParseFile("blocked_tech_schools.txt");
	if (techSchoolObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file blocked_tech_schools.txt";
		exit(-1);
	}
	vector<string> blockedTechSchools;	// the list of disallowed tech schools
	blockedTechSchools = initBlockedTechSchools(techSchoolObj);
	initParser();
	Object* technologyObj = doParseFile( (V2Loc + "/common/technology.txt").c_str() );
	if (technologyObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file " << V2Loc << "/common/technology.txt";
		exit(-1);
	}
	vector<techSchool> techSchools;
	techSchools = initTechSchools(technologyObj, blockedTechSchools);


	// Get Leader traits
	LOG(LogLevel::Info) << "Getting leader traits";
	const V2LeaderTraits lt;	// the V2 leader traits
	map<int, int> leaderIDMap; // <EU4, V2>

	// Get CK2 title names
	LOG(LogLevel::Info) << "Getting CK2 titles";
	Object* CK2TitleObj = doParseFile("ck2titlemap.txt");
	CK2TitleMapping ck2Titles = initCK2TitleMap(CK2TitleObj);

	// Parse colony rules
	LOG(LogLevel::Info) << "Parsing colony naming rules.";
	initParser();
	Object* colonialObj = doParseFile("colonial.txt");
	if (colonialObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse colonial.txt";
		exit(-1);
	}
	colonyMapping colonyMap = initColonyMap(colonialObj);
	colonyFlagset colonyFlags = initColonyFlagset(colonialObj);

	// Get EU4 custom flag colours
	FlagColourMapping flagColourMapping;
	LOG(LogLevel::Info) << "Parsing EU4 flag colours";
	Object* colorsObj = doParseFile((EU4Loc + "/common/custom_country_colors/00_custom_country_colors.txt").c_str());
	if (colorsObj == NULL)
	{
		LOG(LogLevel::Warning) << "Could not parse file " << EU4Loc << "/common/custom_country_colors/00_custom_country_colors.txt";
	}
	else if (colorsObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Warning) << "Failed to parse 00_custom_country_colors.txt";
	}
	else
	{
		flagColourMapping = initFlagColours(colorsObj);
	}

	// Get EU4 colonial regions
	LOG(LogLevel::Info) << "Parsing EU4 colonial regions";
	Object* colonialRegionsObj = doParseFile((EU4Loc + "/common/colonial_regions/00_colonial_regions.txt").c_str());
	if (colonialRegionsObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file " << EU4Loc << "/common/colonial_regions/00_colonial_regions.txt";
		exit(-1);
	}
	if (colonialRegionsObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse 00_colonial_regions.txt";
		return 1;
	}
	countryMap.readEU4Regions(colonialRegionsObj);
	for (vector<string>::iterator itr = fullModPaths.begin(); itr != fullModPaths.end(); itr++)
	{
		if(boost::filesystem::exists(string(*itr + "/common/colonial_regions/")) && boost::filesystem::is_directory(string(*itr + "/common/colonial_regions/")))
                {
                        for(boost::filesystem::directory_entry& file : boost::filesystem::directory_iterator(string(*itr + "/common/colonial_regions/")))
                        {
                          string modRegionsFile(*itr + "/common/colonial_regions/" + file.path().native());    // the path and name of the colonial regions file in this mod
                          if(boost::filesystem::exists(modRegionsFile) && boost::filesystem::is_regular_file(modRegionsFile))
                          {
                                  colonialRegionsObj = doParseFile(modRegionsFile.c_str());
                                  if (colonialRegionsObj == NULL)
                                  {
                                          LOG(LogLevel::Error) << "Could not parse file " << modRegionsFile;
                                          exit(-1);
                                  }
                                  countryMap.readEU4Regions(colonialRegionsObj);
                          }
                        }
                }
	}

	// Parse EU4 Regions
	LOG(LogLevel::Info) << "Parsing EU4 regions";
	Object* regionsObj = doParseFile((EU4Loc + "/map/region.txt").c_str());
	if (regionsObj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file " << EU4Loc << "/map/region.txt";
		exit(-1);
	}
	if (regionsObj->getLeaves().size() < 1)
	{
		LOG(LogLevel::Error) << "Failed to parse region.txt";
		return 1;
	}
	EU4RegionsMapping EU4RegionsMap;
	initEU4RegionMapOldVersion(regionsObj, EU4RegionsMap);
	for (vector<string>::iterator itr = fullModPaths.begin(); itr != fullModPaths.end(); itr++)
	{
		string modRegionFile(*itr + "/map/region.txt");
		if(boost::filesystem::exists(modRegionFile) && boost::filesystem::is_regular_file(modRegionFile))
		{
			regionsObj = doParseFile(modRegionFile.c_str());
			if (regionsObj == NULL)
			{
				LOG(LogLevel::Error) << "Could not parse file " << modRegionFile;
				exit(-1);
			}
			initEU4RegionMapOldVersion(regionsObj, EU4RegionsMap);
		}
	}
	if (EU4RegionsMap.size() == 0) // if it failed, we're using the new regions format
	{
		Object* areaObj = doParseFile((EU4Loc + "/map/area.txt").c_str());
		if (areaObj == NULL)
		{
			LOG(LogLevel::Error) << "Could not parse file " << EU4Loc << "/map/area.txt";
			exit(-1);
		}
		if (areaObj->getLeaves().size() < 1)
		{
			LOG(LogLevel::Error) << "Failed to parse area.txt";
			return 1;
		}
		initEU4RegionMap(regionsObj, areaObj, EU4RegionsMap);
		for (vector<string>::iterator itr = fullModPaths.begin(); itr != fullModPaths.end(); itr++)
		{
			string modAreaFile(*itr + "/map/area.txt");

			if(boost::filesystem::exists(modAreaFile) && boost::filesystem::is_regular_file(modAreaFile))
			{
				areaObj = doParseFile(modAreaFile.c_str());
				if (areaObj == NULL)
				{
					LOG(LogLevel::Error) << "Could not parse file " << modAreaFile;
					exit(-1);
				}
				initEU4RegionMap(regionsObj, areaObj, EU4RegionsMap);
			}
		}
	}

	// Create country mapping
	LOG(LogLevel::Info) << "Creating country mapping";
	removeEmptyNations(sourceWorld);
	if (Configuration::getRemovetype() == "dead")
	{
		removeDeadLandlessNations(sourceWorld);
	}
	else if (Configuration::getRemovetype() == "all")
	{
		removeLandlessNations(sourceWorld);
	}
	countryMap.CreateMapping(sourceWorld, destWorld, colonyMap, inverseProvinceMap, provinceMap, inverseUnionCultures, ck2Titles);


	// Convert
	LOG(LogLevel::Info) << "Converting countries";
	destWorld.convertCountries(sourceWorld, countryMap, cultureMap, unionCultures, religionMap, governmentMap, inverseProvinceMap, techSchools, leaderIDMap, lt, ck2Titles, colonyFlags, UHLiberalIdeas, UHReactionaryIdeas, literacyIdeas, orderIdeas, libertyIdeas, equalityIdeas, EU4RegionsMap);
	LOG(LogLevel::Info) << "Converting provinces";
	destWorld.convertProvinces(sourceWorld, provinceMap, resettableProvinces, countryMap, cultureMap, slaveCultureMap, religionMap, stateIndexMap, EU4RegionsMap);
	LOG(LogLevel::Info) << "Converting diplomacy";
	destWorld.convertDiplomacy(sourceWorld, countryMap);
	LOG(LogLevel::Info) << "Setting colonies";
	destWorld.setupColonies(adjacencyMap, continentMap);
	LOG(LogLevel::Info) << "Creating states";
	destWorld.setupStates(stateMap);
	LOG(LogLevel::Info) << "Setting unciv reforms";
	destWorld.convertUncivReforms();
	LOG(LogLevel::Info) << "Converting techs";
	destWorld.convertTechs(sourceWorld, armyTechIdeas, commerceTechIdeas, cultureTechIdeas, industryTechIdeas, navyTechIdeas);
	LOG(LogLevel::Info) << "Allocating starting factories";
	destWorld.allocateFactories(sourceWorld, factoryBuilder);
	LOG(LogLevel::Info) << "Creating pops";
	destWorld.setupPops(sourceWorld);
	LOG(LogLevel::Info) << "Adding unions";
	destWorld.addUnions(unionMap);
	LOG(LogLevel::Info) << "Converting armies and navies";
	destWorld.convertArmies(sourceWorld, inverseProvinceMap, leaderIDMap, adjacencyMap);

	// Output results
	LOG(LogLevel::Info) << "Outputting mod";
	system("%systemroot%\\System32\\xcopy blankMod output /E /Q /Y /I");
	FILE* modFile;	// the .mod file for this mod
	if (fopen_s(&modFile, ("Output\\" + Configuration::getOutputName() + ".mod").c_str(), "w") != 0)
	{
		LOG(LogLevel::Error) << "Could not create .mod file";
		exit(-1);
	}
	fprintf(modFile, "name = \"Converted - %s\"\n", Configuration::getOutputName().c_str());
	fprintf(modFile, "path = \"mod/%s\"\n", Configuration::getOutputName().c_str());
	fprintf(modFile, "user_dir = \"%s\"\n", Configuration::getOutputName().c_str());
	fprintf(modFile, "replace = \"history/provinces\"\n");
	fprintf(modFile, "replace = \"history/countries\"\n");
	fprintf(modFile, "replace = \"history/diplomacy\"\n");
	fprintf(modFile, "replace = \"history/units\"\n");
	fprintf(modFile, "replace = \"history/pops/1836.1.1\"\n");
	fprintf(modFile, "replace = \"common/religion.txt\"\n");
	fprintf(modFile, "replace = \"common/cultures.txt\"\n");
	fprintf(modFile, "replace = \"common/countries.txt\"\n");
	fprintf(modFile, "replace = \"common/countries/\"\n");
	fprintf(modFile, "replace = \"gfx/interface/icon_religion.dds\"\n");
	fprintf(modFile, "replace = \"localisation/0_Names.csv\"\n");
	fprintf(modFile, "replace = \"localisation/0_Cultures.csv\"\n");
	fprintf(modFile, "replace = \"localisation/0_Religions.csv\"\n");
	fprintf(modFile, "replace = \"history/wars\"\n");
	fclose(modFile);
	string renameCommand = "move /Y output\\output output\\" + Configuration::getOutputName();	// the command to rename the mod correctly
	system(renameCommand.c_str());
	destWorld.setFlagColourMapping(flagColourMapping);
	destWorld.output();

	LOG(LogLevel::Info) << "* Conversion complete *";
	return 0;
}


int main(const int argc, const char * argv[])
{
	try
	{
		LOG(LogLevel::Info) << "Converter version 1.0";
		const char* const defaultEU4SaveFileName = "input.eu4";	// the default name for a save to convert
		string EU4SaveFileName;												// the actual name for the save to convert
		if (argc >= 2)
		{
			EU4SaveFileName = argv[1];
			LOG(LogLevel::Info) << "Using input file " << EU4SaveFileName;
		}
		else
		{
			EU4SaveFileName = defaultEU4SaveFileName;
			LOG(LogLevel::Info) << "No input file given, defaulting to " << defaultEU4SaveFileName;
		}
		return ConvertEU4ToV2(EU4SaveFileName);
	}
	catch (const std::exception& e)	// catch any errors
	{
		LOG(LogLevel::Error) << e.what();
		return -1;
	}
}
