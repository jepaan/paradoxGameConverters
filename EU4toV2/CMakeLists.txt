cmake_minimum_required(VERSION 2.6)
project(eu4tov2)

ADD_DEFINITIONS(
    -std=c++11
)

set(source 
  Source/Color.cpp
  Source/Configuration.cpp
  Source/CountryMapping.cpp
  Source/Date.cpp
  Source/EU4toV2Converter.cpp
  Source/FlagUtils.cpp
  Source/Log.cpp
  Source/Mapper.cpp
  Source/targa.cpp
  Source/tempFuncs.cpp
  #Source/WinUtils.cpp
  Source/OSCompatabilityLayer.cpp
  Source/EU4World/EU4Army.cpp
  Source/EU4World/EU4Country.cpp
  Source/EU4World/EU4Diplomacy.cpp
  Source/EU4World/EU4Leader.cpp
  Source/EU4World/EU4Localisation.cpp
  Source/EU4World/EU4Province.cpp
  Source/EU4World/EU4Relations.cpp
  Source/EU4World/EU4Religion.cpp
  Source/EU4World/EU4Version.cpp
  Source/EU4World/EU4World.cpp
  Source/V2World/V2Army.cpp
  Source/V2World/V2Country.cpp
  Source/V2World/V2Creditor.cpp
  Source/V2World/V2Diplomacy.cpp
  Source/V2World/V2Factory.cpp
  Source/V2World/V2Flags.cpp
  Source/V2World/V2Leader.cpp
  Source/V2World/V2LeaderTraits.cpp
  Source/V2World/V2Localisation.cpp
  Source/V2World/V2Party.cpp
  Source/V2World/V2Pop.cpp
  Source/V2World/V2Province.cpp
  Source/V2World/V2Reforms.cpp
  Source/V2World/V2Relations.cpp
  Source/V2World/V2State.cpp
  Source/V2World/V2TechSchools.cpp
  Source/V2World/V2World.cpp
  Source/Parsers/Object.cpp
  Source/Parsers/Parser.cpp)

add_executable(eu4tov2 ${source})

install(TARGETS eu4tov2 RUNTIME DESTINATION bin)


