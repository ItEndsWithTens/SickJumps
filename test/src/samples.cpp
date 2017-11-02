#include "catch/catch.hpp"

#include "SickJumps.h"

TEST_CASE("Ramps start and end on proper input samples with a full multiplier of 1.0")
{
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 1.0, 800);

	CHECK(c.rampUpFirstInputSample == 8000000);
	CHECK(c.rampUpLastInputSample == 8095999);
	CHECK(c.fullSpeedFirstInputSample == 8096000);
	CHECK(c.fullSpeedLastInputSample == 71904799);
	CHECK(c.rampDownFirstInputSample == 71904800);
	CHECK(c.rampDownLastInputSample == 72000799);
	CHECK(c.afterFirstInputSample == 72000800);

	CHECK(c.rampUpFirstOutputSample == 8000000);
	CHECK(c.rampUpLastOutputSample == 8095999);
	CHECK(c.fullSpeedFirstOutputSample == 8096000);
	CHECK(c.fullSpeedLastOutputSample == 71904799);
	CHECK(c.rampDownFirstOutputSample == 71904800);
	CHECK(c.rampDownLastOutputSample == 72000799);
}

TEST_CASE("Ramps start and end on proper input samples with a full multiplier of 8.0")
{
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 8.0, 800);

	CHECK(c.rampUpFirstInputSample == 8000000);
	CHECK(c.rampUpLastInputSample == 8431996);
	CHECK(c.fullSpeedFirstInputSample == 8432004);
	CHECK(c.fullSpeedLastInputSample == 71568796);
	CHECK(c.rampDownFirstInputSample == 71568804);
	CHECK(c.rampDownLastInputSample == 72000799);
	CHECK(c.afterFirstInputSample == 72000800);

	CHECK(c.rampUpFirstOutputSample == 8000000);
	CHECK(c.rampUpLastOutputSample == 8095999);
	CHECK(c.fullSpeedFirstOutputSample == 8096000);
	CHECK(c.fullSpeedLastOutputSample == 15988099);
	CHECK(c.rampDownFirstOutputSample == 15988100);
	CHECK(c.rampDownLastOutputSample == 16084099);
}
