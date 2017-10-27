#include <catch/catch.hpp>

#include "avisynth.h"

#include "SickJumps.h"



extern IScriptEnvironment* env;



TEST_CASE("Ramp input frame count matches output frame count when multipliers are both 1.0")
{
	REQUIRE(CalculateRampInputFrames(0, 0, 1.0, 1.0, SickJumps::MODE_LINEAR) == 0);
	REQUIRE(CalculateRampInputFrames(0, 10, 1.0, 1.0, SickJumps::MODE_LINEAR) == 10);
	REQUIRE(CalculateRampInputFrames(0, 100, 1.0, 1.0, SickJumps::MODE_LINEAR) == 100);
	REQUIRE(CalculateRampInputFrames(0, 1000, 1.0, 1.0, SickJumps::MODE_LINEAR) == 1000);

	REQUIRE(CalculateRampInputFrames(0, 667, 1.0, 1.0, SickJumps::MODE_LINEAR) == 667);
	REQUIRE(CalculateRampInputFrames(0, 5353, 1.0, 1.0, SickJumps::MODE_LINEAR) == 5353);
	REQUIRE(CalculateRampInputFrames(0, 9999, 1.0, 1.0, SickJumps::MODE_LINEAR) == 9999);
}
