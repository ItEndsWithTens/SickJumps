#include "catch/catch.hpp"

#include <string>
#include <tuple>

#include "avisynth.h"

#include "SickJumps.h"



extern IScriptEnvironment* env;



TEST_CASE("Ramp input frame count matches output frame count when multipliers are both 1.0")
{
	CHECK(CalculateRampInputCount(0, 0, 1.0, 1.0) == 0);
	CHECK(CalculateRampInputCount(0, 10, 1.0, 1.0) == 10);
	CHECK(CalculateRampInputCount(0, 100, 1.0, 1.0) == 100);
	CHECK(CalculateRampInputCount(0, 1000, 1.0, 1.0) == 1000);

	CHECK(CalculateRampInputCount(0, 667, 1.0, 1.0) == 667);
	CHECK(CalculateRampInputCount(0, 5353, 1.0, 1.0) == 5353);
	CHECK(CalculateRampInputCount(0, 9999, 1.0, 1.0) == 9999);
}

TEST_CASE("Ramps start and end on proper input frames with a full multiplier of 1.0")
{
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 1.0, 800);

	CHECK(c.rampUpFirstInputFrame == 10000);
	CHECK(c.rampUpLastInputFrame == 10119);
	CHECK(c.fullSpeedFirstInputFrame == 10120);
	CHECK(c.fullSpeedLastInputFrame == 89880);
	CHECK(c.rampDownFirstInputFrame == 89881);
	CHECK(c.rampDownLastInputFrame == 90000);

	int adjustedFrame;
	double multiplier;
	std::string text;

	// Yes, there will be float equality checks here, but only at the bounds of the
	// ramps, where the values are meant to be exact.

	// Before
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(0);
	CHECK(adjustedFrame == 0);
	CHECK(multiplier == 1.0);
	CHECK(text == "before");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(9999);
	CHECK(adjustedFrame == 9999);
	CHECK(multiplier == 1.0);
	CHECK(text == "before");

	// Ramp up
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10000);
	CHECK(adjustedFrame == 10000);
	CHECK(multiplier == 1.0);
	CHECK(text == "ramp up");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10119);
	CHECK(adjustedFrame == 10119);
	CHECK(multiplier == 1.0);
	CHECK(text == "ramp up");

	// Full speed
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10120);
	CHECK(adjustedFrame == 10120);
	CHECK(multiplier == 1.0);
	CHECK(text == "full speed");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(89880);
	CHECK(adjustedFrame == 89880);
	CHECK(multiplier == 1.0);
	CHECK(text == "full speed");

	// Ramp down
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(89881);
	CHECK(adjustedFrame == 89881);
	CHECK(multiplier == 1.0);
	CHECK(text == "ramp down");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(90000);
	CHECK(adjustedFrame == 90000);
	CHECK(multiplier == 1.0);
	CHECK(text == "ramp down");

	// After
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(90001);
	CHECK(adjustedFrame == 90001);
	CHECK(multiplier == 1.0);
	CHECK(text == "after");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(99999);
	CHECK(adjustedFrame == 99999);
	CHECK(multiplier == 1.0);
	CHECK(text == "after");
}

TEST_CASE("Ramps start and end on proper input frames with a full multiplier of 8.0")
{
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 8.0, 800);

	CHECK(c.rampUpFirstOutputFrame == 10000);
	CHECK(c.rampUpLastOutputFrame == 10119);
	CHECK(c.fullSpeedFirstOutputFrame == 10120);
	CHECK(c.fullSpeedLastOutputFrame == 19984);
	CHECK(c.rampDownFirstOutputFrame == 19985);

	CHECK(c.rampUpFirstInputFrame == 10000);
	CHECK(c.rampUpLastInputFrame == 10536);
	CHECK(c.fullSpeedFirstInputFrame == 10544);
	CHECK(c.fullSpeedLastInputFrame == 89456);
	CHECK(c.rampDownFirstInputFrame == 89464);
	CHECK(c.rampDownLastInputFrame == 90000);

	int adjustedFrame;
	double multiplier;
	std::string text;

	// Before
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(0);
	CHECK(adjustedFrame == 0);
	CHECK(multiplier == 1.0);
	CHECK(text == "before");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(9999);
	CHECK(adjustedFrame == 9999);
	CHECK(multiplier == 1.0);
	CHECK(text == "before");

	// Ramp up
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10000);
	CHECK(adjustedFrame == 10000);
	CHECK(multiplier == 1.0);
	CHECK(text == "ramp up");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10001);
	CHECK(adjustedFrame == 10001);
	CHECK(text == "ramp up");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10118);
	CHECK(adjustedFrame == 10528);
	CHECK(text == "ramp up");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10119);
	CHECK(adjustedFrame == 10536);
	CHECK(multiplier == 8.0);
	CHECK(text == "ramp up");

	// Full speed
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10120);
	CHECK(adjustedFrame == 10544);
	CHECK(multiplier == 8.0);
	CHECK(text == "full speed");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(19984);
	CHECK(adjustedFrame == 89456);
	CHECK(multiplier == 8.0);
	CHECK(text == "full speed");

	// Ramp down
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(19985);
	CHECK(adjustedFrame == 89464);
	CHECK(multiplier == 8.0);
	CHECK(text == "ramp down");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(19986);
	CHECK(adjustedFrame == 89472);
	CHECK(text == "ramp down");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(20103);
	CHECK(adjustedFrame == 89999);
	CHECK(text == "ramp down");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(20104);
	CHECK(adjustedFrame == 90000);
	CHECK(multiplier == 1.0);
	CHECK(text == "ramp down");

	// After
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(20105);
	CHECK(adjustedFrame == 90001);
	CHECK(multiplier == 1.0);
	CHECK(text == "after");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(30103);
	CHECK(adjustedFrame == 99999);
	CHECK(multiplier == 1.0);
	CHECK(text == "after");
}

TEST_CASE("Ramps start and end on proper input frames with a full multiplier of 32.0")
{
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 32.0, 800);

	CHECK(c.rampUpFirstOutputFrame == 10000);
	CHECK(c.rampUpLastOutputFrame == 10119);
	CHECK(c.fullSpeedFirstOutputFrame == 10120);
	CHECK(c.fullSpeedLastOutputFrame == 12495);
	CHECK(c.rampDownFirstOutputFrame == 12496);
	CHECK(c.rampDownLastOutputFrame == 12615);

	CHECK(c.rampUpFirstInputFrame == 10000);
	CHECK(c.rampUpLastInputFrame == 11964);
	CHECK(c.fullSpeedFirstInputFrame == 11996);
	CHECK(c.fullSpeedLastInputFrame == 87996);
	CHECK(c.rampDownFirstInputFrame == 88028);
	CHECK(c.rampDownLastInputFrame == 90000);

	int adjustedFrame;
	double multiplier;
	std::string text;

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(0);
	CHECK(adjustedFrame == 0);
	CHECK(multiplier == 1.0);
	CHECK(text == "before");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(9999);
	CHECK(adjustedFrame == 9999);
	CHECK(multiplier == 1.0);
	CHECK(text == "before");

	// Ramp up
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10000);
	CHECK(adjustedFrame == 10000);
	CHECK(multiplier == 1.0);
	CHECK(text == "ramp up");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10001);
	CHECK(adjustedFrame == 10001);
	CHECK(text == "ramp up");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10118);
	CHECK(adjustedFrame == 11932);
	CHECK(text == "ramp up");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10119);
	CHECK(adjustedFrame == 11964);
	CHECK(multiplier == 32.0);
	CHECK(text == "ramp up");

	// Full speed
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(10120);
	CHECK(adjustedFrame == 11996);
	CHECK(multiplier == 32.0);
	CHECK(text == "full speed");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(12495);
	CHECK(adjustedFrame == 87996);
	CHECK(multiplier == 32.0);
	CHECK(text == "full speed");

	// Ramp down
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(12496);
	CHECK(adjustedFrame == 88028);
	CHECK(multiplier == 32.0);
	CHECK(text == "ramp down");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(12497);
	CHECK(adjustedFrame == 88060);
	CHECK(text == "ramp down");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(12614);
	CHECK(adjustedFrame == 89999);
	CHECK(text == "ramp down");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(12615);
	CHECK(adjustedFrame == 90000);
	CHECK(multiplier == 1.0);
	CHECK(text == "ramp down");

	// After
	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(12616);
	CHECK(adjustedFrame == 90001);
	CHECK(multiplier == 1.0);
	CHECK(text == "after");

	std::tie(adjustedFrame, multiplier, text) = c.GetAdjustedFrameProperties(22614);
	CHECK(adjustedFrame == 99999);
	CHECK(multiplier == 1.0);
	CHECK(text == "after");
}
