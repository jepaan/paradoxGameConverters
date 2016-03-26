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



#ifndef V2PROVINCE_H_
#define V2PROVINCE_H_

#include "../Configuration.h"
#include "../EU4World/EU4World.h"
#include "../EU4World/EU4Country.h"

class V2Pop;
class V2Factory;
class V2Country;



struct V2Demographic
{
	std::string								culture;
	std::string								slaveCulture;
	std::string								religion;
	double								upperRatio;
	double								middleRatio;
	double								lowerRatio;
	EU4Province*						oldProvince;
	EU4Country*							oldCountry;
};


class V2Province
{
	public:
		V2Province(std::string _filename);
		void output() const;
		void outputPops(FILE*) const;
		void convertFromOldProvince(const EU4Province* oldProvince);
		void determineColonial();
		void addCore(std::string);
		void addOldPop(const V2Pop*);
		void addMinorityPop(V2Pop*);
		void doCreatePops(double popWeightRatio, V2Country* _owner, int popConversionAlgorithm);
		void addFactory(V2Factory* factory);
		void addPopDemographic(V2Demographic d);

		int				getTotalPopulation() const;
		std::vector<V2Pop*>	getPops(std::string type) const;
		V2Pop*			getSoldierPopForArmy(bool force = false);
		std::pair<int, int>	getAvailableSoldierCapacity() const;
		std::string			getRegimentName(RegimentCategory rc);
		bool				hasCulture(std::string culture, float percentOfPopulation) const;
		
		void				clearCores()									{ cores.clear(); }
		void				setCoastal(bool _coastal)					{ coastal = _coastal; }
		void				setName(std::string _name)						{ name = _name; }
		void				setOwner(std::string _owner)						{ owner = _owner; }
		void				setLandConnection(bool _connection)		{ landConnection = _connection; }
		void				setSameContinent(bool _same)				{ sameContinent = _same; }
		void				setFortLevel(int level)						{ fortLevel = level; }
		void				setNavalBaseLevel(int level)				{ navalBaseLevel = level; }
		void				setRailLevel(int level)						{ railLevel = level; }
		void				setResettable(const bool _resettable)	{ resettable = _resettable; }
		void				setSlaveProportion(const double _pro)	{ slaveProportion = _pro; }

		const EU4Province*	getSrcProvince()		const { return srcProvince; }
		int						getOldPopulation()	const	{ return oldPopulation; }
		bool						wasInfidelConquest()	const { return originallyInfidel; }
		bool						wasColony()				const { return wasColonised; }
		bool						isColonial()			const { return colonial != 0; }
		std::string					getRgoType()			const { return rgoType; }
		std::string					getOwner()				const { return owner; }
		int						getNum()					const { return num; }
		std::string					getName()				const { return name; }
		bool						isCoastal()				const { return coastal; }
		bool						hasNavalBase()			const { return (navalBaseLevel > 0); }
		bool						hasLandConnection()	const { return landConnection; }
		std::vector<V2Pop*>			getPops()				const { return pops; }

	private:
		void outputUnits(FILE*) const;

		struct pop_points;
		pop_points getPopPoints_1(const V2Demographic& demographic, double newPopulation, const V2Country* _owner); // EU4 1.0-1.11
		pop_points getPopPoints_2(const V2Demographic& demographic, double newPopulation, const V2Country* _owner); // EU4 1.12 and newer
		void createPops(const V2Demographic& demographic, double popWeightRatio, const V2Country* _owner, int popConversionAlgorithm);
		void combinePops();
		bool growSoldierPop(V2Pop* pop);

		const EU4Province*		srcProvince;

		std::string						filename;
		bool							coastal;
		int							num;
		std::string						name;
		std::string						owner;
		std::vector<std::string>				cores;
		bool							inHRE;
		int							colonyLevel;
		int							colonial;
		bool							wasColonised;
		bool							landConnection;
		bool							sameContinent;
		bool							originallyInfidel;
		int							oldPopulation;
		std::vector<V2Demographic>	demographics;
		std::vector<const V2Pop*>		oldPops;
		std::vector<V2Pop*>				minorityPops;
		std::vector<V2Pop*>				pops;
		double						slaveProportion;
		std::string						rgoType;
		std::string						terrain;
		int							lifeRating;
		bool							slaveState;
		int							unitNameCount[num_reg_categories];
		int							fortLevel;
		int							navalBaseLevel;
		int							railLevel;
		std::map<std::string, V2Factory*>	factories;

		bool							resettable;
};



#endif // V2PROVINCE_H_
