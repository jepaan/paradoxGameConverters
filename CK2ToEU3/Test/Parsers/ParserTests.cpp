/*Copyright (c) 2015 The Paradox Game Converters Project

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

#include <sstream>
#include <string>
#include "boost\bind.hpp"
#include "boost\ref.hpp"
#include "gtest\gtest.h"
#include "Parsers\Parser.h"

using namespace testing;

namespace ck2
{
namespace unittests
{

class ParserShould : public Test
{
};

TEST_F(ParserShould, ThrowExceptionIfBufferIsValidatedWithoutParserBeingInitialized)
{
	std::istringstream buffer("sample");

	ASSERT_THROW(boost::bind(&validateBuffer, boost::ref(buffer)), std::domain_error);
}

class InitializedParserShould : public Test
{
protected:

	virtual void SetUp()
	{
		initParser();
	}
};

TEST_F(InitializedParserShould, RecognizeUnicodeSWithCaronInNestedAssignments)
{
	std::istringstream buffer("e_scandinavia = {\n\
	ugricbaltic = Ik�kila\n\
}");

	ASSERT_TRUE(validateBuffer(buffer));
}

TEST_F(InitializedParserShould, RecognizeUnicodeZWithCaronInNestedAssignments)
{
	std::istringstream buffer("e_scandinavia = {\n\
	polish = Limba�i\n\
}");

	ASSERT_TRUE(validateBuffer(buffer));
}

} //namespace unittests
} //namespace ck2