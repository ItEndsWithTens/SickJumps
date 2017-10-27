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

TEST_CASE("Frame numbers are what they should be when multipliers are both 1.0")
{
	SickJumpsCore c = SickJumpsCore(1000, 100, 500, 60.0, 2.0, 2.0, 1.0, 1.0, 800, SickJumps::MODE_LINEAR);

	REQUIRE(c.originalFrameCount == c.adjustedFrameCount);
	REQUIRE(c.originalSampleCount == c.adjustedSampleCount);

	REQUIRE(c.rampUpFirstInputFrame == 100);
	REQUIRE(c.rampDownLastInputFrame == 500);
	
	REQUIRE(c.rampUpLastInputFrame == 219);
	REQUIRE(c.rampDownFirstInputFrame == 381);
}

TEST_CASE()
{
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 8.0, 800, SickJumps::MODE_LINEAR);

	REQUIRE(c.rampUpFirstInputFrame == 10000);
	REQUIRE(c.rampDownLastInputFrame == 90000);

	double average = (c.startMultiplier + c.fullMultiplier) / 2.0;
	
	int rampUpDiff = c.rampUpLastOutputFrame - c.rampUpFirstOutputFrame;
	int rampUpLastInputFrame = c.rampUpFirstInputFrame + static_cast<int>(std::round(rampUpDiff * average));
	REQUIRE(c.rampUpLastInputFrame == rampUpLastInputFrame);

	int rampDownDiff = c.rampDownLastOutputFrame - c.rampDownFirstOutputFrame;
	int rampDownFirstInputFrame = c.rampDownLastInputFrame - static_cast<int>(std::round(rampDownDiff * average));
	rampDownFirstInputFrame += static_cast<int>(std::round(c.fullMultiplier));
	REQUIRE(c.rampDownFirstInputFrame == rampDownFirstInputFrame);
}
