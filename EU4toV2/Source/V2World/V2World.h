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



#ifndef V2WORLD_H_
#define V2WORLD_H_

#include <set>

#include "V2Country.h"
#include "V2Diplomacy.h"
#include "V2Factory.h"
#include "V2TechSchools.h"
#include "V2Party.h"
#include "V2Province.h"
#include "../EU4World/EU4Army.h"
#include "../EU4World/EU4Province.h"
#include "../CountryMapping.h"
#include "../Mapper.h"


class V2Country;
class V2Army;
class V2LeaderTraits;



class V2World {
	public:
		V2World(const minorityPopMapping& minorities);
		void output() const;
		void createProvinceFiles(const EU4World& sourceWorld, const provinceMapping& provinceMap);
		const void setFlagColourMapping(FlagColourMapping flagColours) { flagColourMapping = flagColours; };
		
		void convertCountries(const EU4World& sourceWorld, const CountryMapping& countryMap, const cultureMapping& cultureMap, const unionCulturesMap& unionCultures, const religionMapping& religionMap, const governmentMapping& governmentMap, const inverseProvinceMapping& inverseProvinceMap, const std::vector<techSchool>& techSchools, std::map<int, int>& leaderMap, const V2LeaderTraits& lt, const CK2TitleMapping& ck2titlemap, colonyFlagset& colonyFlags, const std::map<std::string, double>& UHLiberalIdeas, const std::map<std::string, double>& UHReactionaryIdeas, const std::vector< pair<std::string, int> >& literacyIdeas, const std::map<std::string, int>& orderIdeas, const std::map<std::string, int>& libertyIdeas, const std::map<std::string, int>& equalityIdeas, const EU4RegionsMapping& regionsMap);
		void convertDiplomacy(const EU4World& sourceWorld, const CountryMapping& countryMap);
		void convertProvinces(const EU4World& sourceWorld, const provinceMapping& provinceMap, const resettableMap& resettableProvinces, const CountryMapping& countryMap, const cultureMapping& cultureMap, const cultureMapping& slaveCultureMap, const religionMapping& religionMap, const stateIndexMapping& stateIndexMap, const EU4RegionsMapping& regionsMap);
		void setupColonies(const adjacencyMapping& adjacencyMap, const continentMapping& continentMap);
		void setupStates(const stateMapping&);
		void convertUncivReforms();
		void setupPops(EU4World& sourceWorld);
		void addUnions(const unionMapping& unionMap);
		void convertArmies(const EU4World& sourceWorld, const inverseProvinceMapping& inverseProvinceMap, const std::map<int,int>& leaderIDMap, adjacencyMapping adjacencyMap);
		void convertTechs(const EU4World& sourceWorld, std::map<std::string, double>& armyTechIdeas, std::map<std::string, double>& commerceTechIdeas, std::map<std::string, double>& cultureTechIdeas, std::map<std::string, double>& industryTechIdeas, std::map<std::string, double>& navyTechIdeas);
		void allocateFactories(const EU4World& sourceWorld, const V2FactoryFactory& factoryBuilder);

		std::map<std::string, V2Country*>	getPotentialCountries()	const;
		std::map<std::string, V2Country*>	getDynamicCountries()	const;

	private:
		std::vector<V2Demographic>	determineDemographics(std::vector<EU4PopRatio> popRatios, EU4Province* eProv, V2Province* vProv, EU4Country* oldOwner, const cultureMapping& cultureMap, const cultureMapping& slaveCultureMap, const religionMapping& religionMap, const EU4RegionsMapping& regionsMap, int destNum, double provPopRatio);

		void				outputPops() const;
		void				getProvinceLocalizations(std::string file);
		V2Country*		getCountry(std::string tag);

		std::map<int, V2Province*>		provinces;
		std::map<std::string, V2Country*>		countries;
		std::vector<V2Country*>			potentialCountries;
		std::map<std::string, V2Country*>		dynamicCountries;
		V2Diplomacy						diplomacy;
		std::map< int, std::set<std::string> >		colonies;

		CK2TitleMapping				ck2titles;
		colonyFlagset					colonyFlagMap;
		std::map<std::string, list<int>* >	popRegions;

		long								totalWorldPopulation;

		FlagColourMapping		flagColourMapping;
		bool						isRandomWorld;
};



#endif // V2WORLD_H_
