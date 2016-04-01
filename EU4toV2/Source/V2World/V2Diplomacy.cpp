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

#include <boost/filesystem.hpp>

#include "../OSCompatabilityLayer.h"
#include "V2Diplomacy.h"
#include "../Log.h"
#include "../Configuration.h"



void V2Diplomacy::output() const
{
	LOG(LogLevel::Debug) << "Writing diplomacy";
        std::string alliancePath = "Output/" + Configuration::getOutputName() + "/history/diplomacy/Alliances.txt";
        boost::filesystem::file_status st = boost::filesystem::status(alliancePath);
        if(!boost::filesystem::exists(st))
	{
		LOG(LogLevel::Error) << "Could not create alliances history file";
		exit(-1);
	}
        std::string guaranteesPath = "Output/" + Configuration::getOutputName() + "/history/diplomacy/Guarantees.txt";
	st = boost::filesystem::status(alliancePath);
	if(!boost::filesystem::exists(st))
	{
		LOG(LogLevel::Error) << "Could not create guarantees history file";
		exit(-1);
	}
        
	std::string puppetStatesPath = "Output/" + Configuration::getOutputName() + "/history/diplomacy/PuppetStates.txt";
        st = boost::filesystem::status(puppetStatesPath);
	if(!boost::filesystem::exists(st))
	{
		LOG(LogLevel::Error) << "Could not create puppet states history file";
		exit(-1);
	}

	std::string unionsPath = "Output/" + Configuration::getOutputName() + "/history/diplomacy/Unions.txt";
        st = boost::filesystem::status(unionsPath);
	if(!boost::filesystem::exists(st))
	{
		LOG(LogLevel::Error) << "Could not create unions history file";
		exit(-1);
	}
	FILE* alliances;
        fopen_s(&alliances, alliancePath.c_str(), "w");
        FILE* guarantees;
        fopen_s(&guarantees, guaranteesPath.c_str(), "w");
        FILE* puppetStates;
        fopen_s(&puppetStates, puppetStatesPath.c_str(), "w");
        FILE* unions;
        fopen_s(&unions, unionsPath.c_str(), "w");
	FILE* out;
	for (vector<V2Agreement>::const_iterator itr = agreements.begin(); itr != agreements.end(); ++itr)
	{
		if (itr->type == "guarantee")
		{
			out = guarantees;
		}
		else if (itr->type == "union")
		{
			out = unions;
		}
		else if (itr->type == "vassal")
		{
			out = puppetStates;
		}
		else if (itr->type == "alliance")
		{
			out = alliances;
		}
		else
		{
			LOG(LogLevel::Warning) << "Cannot ouput diplomatic agreement type " << itr->type;
			continue;
		}
		fprintf(out, "%s=\n", itr->type.c_str());
		fprintf(out, "{\n");
		fprintf(out, "\tfirst=\"%s\"\n", itr->country1.c_str());
		fprintf(out, "\tsecond=\"%s\"\n", itr->country2.c_str());
		fprintf(out, "\tstart_date=\"%s\"\n", itr->start_date.toString().c_str());
		fprintf(out, "\tend_date=\"1936.1.1\"\n");
		fprintf(out, "}\n");
		fprintf(out, "\n");
	}
	
	fclose(alliances);
	fclose(guarantees);
	fclose(puppetStates);
	fclose(unions);
}
