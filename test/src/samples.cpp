#include "catch/catch.hpp"

#include <cstdint>

#include "avisynth.h"

#include "SickJumps.h"



extern IScriptEnvironment* env;



int32_t Get24bitInt(uint8_t* buf);



TEST_CASE("Ramps start and end on proper input samples with a full multiplier of 1.0")
{
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 1.0, 1.0, 800);

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
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 8.0, 1.0, 800);

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

TEST_CASE("Ramps start and end on proper input samples with a full multiplier of 4.0 "
	"and an end multiplier of 2.0")
{
	SickJumpsCore c = SickJumpsCore(100000, 10000, 90000, 60.0, 2.0, 2.0, 1.0, 4.0, 2.0, 800);

	CHECK(c.rampUpFirstInputSample == 8000000);
	CHECK(c.rampUpLastInputSample == 8239998);
	CHECK(c.fullSpeedFirstInputSample == 8240002);
	CHECK(c.fullSpeedLastInputSample == 71712798);
	CHECK(c.rampDownFirstInputSample == 71712802);
	CHECK(c.rampDownLastInputSample == 72000799);
	CHECK(c.afterFirstInputSample == 72000801);

	CHECK(c.rampUpFirstOutputSample == 8000000);
	CHECK(c.rampUpLastOutputSample == 8095999);
	CHECK(c.fullSpeedFirstOutputSample == 8096000);
	CHECK(c.fullSpeedLastOutputSample == 23964199);
	CHECK(c.rampDownFirstOutputSample == 23964200);
	CHECK(c.rampDownLastOutputSample == 24060199);

	// Before
	CHECK(c.GetAdjustedSampleNumber(0) == 0);
	CHECK(c.GetAdjustedSampleNumber(7999999) == 7999999);

	// Ramp up
	CHECK(c.GetAdjustedSampleNumber(8000000) == 8000000);
	CHECK(c.GetAdjustedSampleNumber(8095999) == 8239998);

	// Full speed
	CHECK(c.GetAdjustedSampleNumber(8096000) == 8240002);
	CHECK(c.GetAdjustedSampleNumber(23964199) == 71712798);

	// Ramp down
	CHECK(c.GetAdjustedSampleNumber(23964200) == 71712802);
	CHECK(c.GetAdjustedSampleNumber(24060199) == 72000799);

	// After
	CHECK(c.GetAdjustedSampleNumber(24060200) == 72000801);
}

TEST_CASE("GetAudio retrieves expected samples regardless of sample type", "[avisynth]")
{
	AVSValue blankArgs[3] = { 1920, 1080, 60.0 };
	char* blankNames[3] = { "width", "height", "fps" };
	PClip blank = env->Invoke("BlankClip", AVSValue(blankArgs, 3), blankNames).AsClip();

	AVSValue toneArgs[3] = { 240, "triangle", 1.0 };
	char* toneNames[3] = { "frequency", "type", "level" };
	PClip tone = env->Invoke("Tone", AVSValue(toneArgs, 3), toneNames).AsClip();

	AVSValue dubArgs[2] = { blank, tone };
	PClip dubbed = env->Invoke("AudioDub", AVSValue(dubArgs, 2)).AsClip();

	SECTION("Floating point")
	{
		VideoInfo vi = dubbed->GetVideoInfo();

		REQUIRE(vi.sample_type == SAMPLE_FLOAT);

		SFLOAT buf[2];

		SECTION("Baseline")
		{
			dubbed->GetAudio(&buf, 0, 1, env);
			CHECK(buf[0] == Approx(0.0f));
			CHECK(buf[1] == Approx(0.0f));

			dubbed->GetAudio(&buf, 25, 1, env);
			CHECK(buf[0] == Approx(0.5f));
			CHECK(buf[1] == Approx(0.5f));

			dubbed->GetAudio(&buf, 50, 1, env);
			CHECK(buf[0] == Approx(1.0f));
			CHECK(buf[1] == Approx(1.0f));

			dubbed->GetAudio(&buf, 75, 1, env);
			CHECK(buf[0] == Approx(0.5f));
			CHECK(buf[1] == Approx(0.5f));

			dubbed->GetAudio(&buf, 100, 1, env);
			CHECK(buf[0] == Approx(0.0f));
			CHECK(buf[1] == Approx(0.0f));

			dubbed->GetAudio(&buf, 125, 1, env);
			CHECK(buf[0] == Approx(-0.5f));
			CHECK(buf[1] == Approx(-0.5f));

			dubbed->GetAudio(&buf, 150, 1, env);
			CHECK(buf[0] == Approx(-1.0f));
			CHECK(buf[1] == Approx(-1.0f));

			dubbed->GetAudio(&buf, 175, 1, env);
			CHECK(buf[0] == Approx(-0.5f));
			CHECK(buf[1] == Approx(-0.5f));

			dubbed->GetAudio(&buf, 200, 1, env);
			CHECK(buf[0] == Approx(0.0f));
			CHECK(buf[1] == Approx(0.0f));
		}

		SECTION("SickJumps")
		{
			AVSValue args[3] = { dubbed, 1.0f, 1.0f };
			char* names[3] = { 0, "start_multiplier", "full_multiplier" };

			PClip result = env->Invoke("SickJumps", AVSValue(args, 3), names).AsClip();

			result->GetAudio(&buf, 0, 1, env);
			CHECK(buf[0] == Approx(0.0f));
			CHECK(buf[1] == Approx(0.0f));

			result->GetAudio(&buf, 25, 1, env);
			CHECK(buf[0] == Approx(0.5f));
			CHECK(buf[1] == Approx(0.5f));

			result->GetAudio(&buf, 50, 1, env);
			CHECK(buf[0] == Approx(1.0f));
			CHECK(buf[1] == Approx(1.0f));

			result->GetAudio(&buf, 75, 1, env);
			CHECK(buf[0] == Approx(0.5f));
			CHECK(buf[1] == Approx(0.5f));

			result->GetAudio(&buf, 100, 1, env);
			CHECK(buf[0] == Approx(0.0f));
			CHECK(buf[1] == Approx(0.0f));

			result->GetAudio(&buf, 125, 1, env);
			CHECK(buf[0] == Approx(-0.5f));
			CHECK(buf[1] == Approx(-0.5f));

			result->GetAudio(&buf, 150, 1, env);
			CHECK(buf[0] == Approx(-1.0f));
			CHECK(buf[1] == Approx(-1.0f));

			result->GetAudio(&buf, 175, 1, env);
			CHECK(buf[0] == Approx(-0.5f));
			CHECK(buf[1] == Approx(-0.5f));

			result->GetAudio(&buf, 200, 1, env);
			CHECK(buf[0] == Approx(0.0f));
			CHECK(buf[1] == Approx(0.0f));
		}
	}

	SECTION("32 bit integer")
	{
		PClip converted = env->Invoke("ConvertAudioTo32bit", dubbed).AsClip();
		VideoInfo vi = converted->GetVideoInfo();

		REQUIRE(vi.sample_type == SAMPLE_INT32);

		// 2 channels per sample.
		int32_t buf[2];

		SECTION("Baseline")
		{
			converted->GetAudio(&buf, 0, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);

			converted->GetAudio(&buf, 25, 1, env);
			CHECK(buf[0] == 1073741824);
			CHECK(buf[1] == 1073741824);

			converted->GetAudio(&buf, 50, 1, env);
			CHECK(buf[0] == 2147483647);
			CHECK(buf[1] == 2147483647);

			converted->GetAudio(&buf, 75, 1, env);
			CHECK(buf[0] == 1073741824);
			CHECK(buf[1] == 1073741824);

			converted->GetAudio(&buf, 100, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);

			converted->GetAudio(&buf, 125, 1, env);
			CHECK(buf[0] == -1073741824);
			CHECK(buf[1] == -1073741824);

			converted->GetAudio(&buf, 150, 1, env);
			CHECK(buf[0] == -2147483648i32);
			CHECK(buf[1] == -2147483648i32);

			converted->GetAudio(&buf, 175, 1, env);
			CHECK(buf[0] == -1073741824);
			CHECK(buf[1] == -1073741824);

			converted->GetAudio(&buf, 200, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);
		}

		SECTION("SickJumps")
		{
			AVSValue args[3] = { converted, 1.0f, 1.0f };
			char* names[3] = { 0, "start_multiplier", "full_multiplier" };

			PClip result = env->Invoke("SickJumps", AVSValue(args, 3), names).AsClip();

			result->GetAudio(&buf, 0, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);

			result->GetAudio(&buf, 25, 1, env);
			CHECK(buf[0] == 1073741824);
			CHECK(buf[1] == 1073741824);

			result->GetAudio(&buf, 50, 1, env);
			CHECK(buf[0] == 2147483647);
			CHECK(buf[1] == 2147483647);

			result->GetAudio(&buf, 75, 1, env);
			CHECK(buf[0] == 1073741824);
			CHECK(buf[1] == 1073741824);

			result->GetAudio(&buf, 100, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);

			result->GetAudio(&buf, 125, 1, env);
			CHECK(buf[0] == -1073741824);
			CHECK(buf[1] == -1073741824);

			result->GetAudio(&buf, 150, 1, env);
			CHECK(buf[0] == -2147483648i32);
			CHECK(buf[1] == -2147483648i32);

			result->GetAudio(&buf, 175, 1, env);
			CHECK(buf[0] == -1073741824);
			CHECK(buf[1] == -1073741824);

			result->GetAudio(&buf, 200, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);
		}
	}

	SECTION("24 bit integer")
	{
		PClip converted = env->Invoke("ConvertAudioTo24bit", dubbed).AsClip();
		VideoInfo vi = converted->GetVideoInfo();

		REQUIRE(vi.sample_type == SAMPLE_INT24);

		// 2 channels per sample * 3 bytes per channel.
		uint8_t buf[2 * 3];

		SECTION("Baseline")
		{
			converted->GetAudio(&buf, 0, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 0);
			CHECK(Get24bitInt(&buf[3]) == 0);

			converted->GetAudio(&buf, 25, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 4194304);
			CHECK(Get24bitInt(&buf[3]) == 4194304);

			converted->GetAudio(&buf, 50, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 8388607);
			CHECK(Get24bitInt(&buf[3]) == 8388607);

			converted->GetAudio(&buf, 75, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 4194304);
			CHECK(Get24bitInt(&buf[3]) == 4194304);

			converted->GetAudio(&buf, 100, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 0);
			CHECK(Get24bitInt(&buf[3]) == 0);

			converted->GetAudio(&buf, 125, 1, env);
			CHECK(Get24bitInt(&buf[0]) == -4194303);
			CHECK(Get24bitInt(&buf[3]) == -4194303);

			converted->GetAudio(&buf, 150, 1, env);
			CHECK(Get24bitInt(&buf[0]) == -8388608);
			CHECK(Get24bitInt(&buf[3]) == -8388608);

			converted->GetAudio(&buf, 175, 1, env);
			CHECK(Get24bitInt(&buf[0]) == -4194303);
			CHECK(Get24bitInt(&buf[3]) == -4194303);

			converted->GetAudio(&buf, 200, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 0);
			CHECK(Get24bitInt(&buf[3]) == 0);
		}

		SECTION("SickJumps")
		{
			AVSValue args[3] = { converted, 1.0f, 1.0f };
			char* names[3] = { 0, "start_multiplier", "full_multiplier" };

			PClip result = env->Invoke("SickJumps", AVSValue(args, 3), names).AsClip();

			converted->GetAudio(&buf, 0, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 0);
			CHECK(Get24bitInt(&buf[3]) == 0);

			converted->GetAudio(&buf, 25, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 4194304);
			CHECK(Get24bitInt(&buf[3]) == 4194304);

			converted->GetAudio(&buf, 50, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 8388607);
			CHECK(Get24bitInt(&buf[3]) == 8388607);

			converted->GetAudio(&buf, 75, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 4194304);
			CHECK(Get24bitInt(&buf[3]) == 4194304);

			converted->GetAudio(&buf, 100, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 0);
			CHECK(Get24bitInt(&buf[3]) == 0);

			converted->GetAudio(&buf, 125, 1, env);
			CHECK(Get24bitInt(&buf[0]) == -4194303);
			CHECK(Get24bitInt(&buf[3]) == -4194303);

			converted->GetAudio(&buf, 150, 1, env);
			CHECK(Get24bitInt(&buf[0]) == -8388608);
			CHECK(Get24bitInt(&buf[3]) == -8388608);

			converted->GetAudio(&buf, 175, 1, env);
			CHECK(Get24bitInt(&buf[0]) == -4194303);
			CHECK(Get24bitInt(&buf[3]) == -4194303);

			converted->GetAudio(&buf, 200, 1, env);
			CHECK(Get24bitInt(&buf[0]) == 0);
			CHECK(Get24bitInt(&buf[3]) == 0);
		}
	}

	SECTION("16 bit integer")
	{
		PClip converted = env->Invoke("ConvertAudioTo16bit", dubbed).AsClip();
		VideoInfo vi = converted->GetVideoInfo();

		REQUIRE(vi.sample_type == SAMPLE_INT16);

		int16_t buf[2];

		SECTION("Baseline")
		{
			converted->GetAudio(&buf, 0, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);

			converted->GetAudio(&buf, 25, 1, env);
			CHECK(buf[0] == 16384);
			CHECK(buf[1] == 16384);

			converted->GetAudio(&buf, 50, 1, env);
			CHECK(buf[0] == 32767);
			CHECK(buf[1] == 32767);

			converted->GetAudio(&buf, 75, 1, env);
			CHECK(buf[0] == 16384);
			CHECK(buf[1] == 16384);

			converted->GetAudio(&buf, 100, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);

			converted->GetAudio(&buf, 125, 1, env);
			CHECK(buf[0] == -16383);
			CHECK(buf[1] == -16383);

			converted->GetAudio(&buf, 150, 1, env);
			CHECK(buf[0] == -32768);
			CHECK(buf[1] == -32768);

			converted->GetAudio(&buf, 175, 1, env);
			CHECK(buf[0] == -16383);
			CHECK(buf[1] == -16383);

			converted->GetAudio(&buf, 200, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);
		}

		SECTION("SickJumps")
		{
			AVSValue args[3] = { converted, 1.0f, 1.0f };
			char* names[3] = { 0, "start_multiplier", "full_multiplier" };

			PClip result = env->Invoke("SickJumps", AVSValue(args, 3), names).AsClip();

			result->GetAudio(&buf, 0, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);

			result->GetAudio(&buf, 25, 1, env);
			CHECK(buf[0] == 16384);
			CHECK(buf[1] == 16384);

			result->GetAudio(&buf, 50, 1, env);
			CHECK(buf[0] == 32767);
			CHECK(buf[1] == 32767);

			result->GetAudio(&buf, 75, 1, env);
			CHECK(buf[0] == 16384);
			CHECK(buf[1] == 16384);

			result->GetAudio(&buf, 100, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);

			result->GetAudio(&buf, 125, 1, env);
			CHECK(buf[0] == -16383);
			CHECK(buf[1] == -16383);

			result->GetAudio(&buf, 150, 1, env);
			CHECK(buf[0] == -32768);
			CHECK(buf[1] == -32768);

			result->GetAudio(&buf, 175, 1, env);
			CHECK(buf[0] == -16383);
			CHECK(buf[1] == -16383);

			result->GetAudio(&buf, 200, 1, env);
			CHECK(buf[0] == 0);
			CHECK(buf[1] == 0);
		}
	}

	SECTION("8 bit integer")
	{
		PClip converted = env->Invoke("ConvertAudioTo8bit", dubbed).AsClip();
		VideoInfo vi = converted->GetVideoInfo();

		REQUIRE(vi.sample_type == SAMPLE_INT8);

		uint8_t buf[2];

		SECTION("Baseline")
		{
			converted->GetAudio(&buf, 0, 1, env);
			CHECK(buf[0] - 128 == 0);
			CHECK(buf[1] - 128 == 0);

			converted->GetAudio(&buf, 25, 1, env);
			CHECK(buf[0] - 128 == 64);
			CHECK(buf[1] - 128 == 64);

			converted->GetAudio(&buf, 50, 1, env);
			CHECK(buf[0] - 128 == 127);
			CHECK(buf[1] - 128 == 127);

			converted->GetAudio(&buf, 75, 1, env);
			CHECK(buf[0] - 128 == 64);
			CHECK(buf[1] - 128 == 64);

			converted->GetAudio(&buf, 100, 1, env);
			CHECK(buf[0] - 128 == 0);
			CHECK(buf[1] - 128 == 0);

			converted->GetAudio(&buf, 125, 1, env);
			CHECK(buf[0] - 128 == -63);
			CHECK(buf[1] - 128 == -63);

			converted->GetAudio(&buf, 150, 1, env);
			CHECK(buf[0] - 128 == -128);
			CHECK(buf[1] - 128 == -128);

			converted->GetAudio(&buf, 175, 1, env);
			CHECK(buf[0] - 128 == -63);
			CHECK(buf[1] - 128 == -63);

			converted->GetAudio(&buf, 200, 1, env);
			CHECK(buf[0] - 128 == 0);
			CHECK(buf[1] - 128 == 0);
		}

		SECTION("SickJumps")
		{
			AVSValue args[3] = { converted, 1.0f, 1.0f };
			char* names[3] = { 0, "start_multiplier", "full_multiplier" };

			PClip result = env->Invoke("SickJumps", AVSValue(args, 3), names).AsClip();

			result->GetAudio(&buf, 0, 1, env);
			CHECK(buf[0] - 128 == 0);
			CHECK(buf[1] - 128 == 0);

			result->GetAudio(&buf, 25, 1, env);
			CHECK(buf[0] - 128 == 64);
			CHECK(buf[1] - 128 == 64);

			result->GetAudio(&buf, 50, 1, env);
			CHECK(buf[0] - 128 == 127);
			CHECK(buf[1] - 128 == 127);

			result->GetAudio(&buf, 75, 1, env);
			CHECK(buf[0] - 128 == 64);
			CHECK(buf[1] - 128 == 64);

			result->GetAudio(&buf, 100, 1, env);
			CHECK(buf[0] - 128 == 0);
			CHECK(buf[1] - 128 == 0);

			result->GetAudio(&buf, 125, 1, env);
			CHECK(buf[0] - 128 == -63);
			CHECK(buf[1] - 128 == -63);

			result->GetAudio(&buf, 150, 1, env);
			CHECK(buf[0] - 128 == -128);
			CHECK(buf[1] - 128 == -128);

			result->GetAudio(&buf, 175, 1, env);
			CHECK(buf[0] - 128 == -63);
			CHECK(buf[1] - 128 == -63);

			result->GetAudio(&buf, 200, 1, env);
			CHECK(buf[0] - 128 == 0);
			CHECK(buf[1] - 128 == 0);
		}
	}
}

int32_t Get24bitInt(uint8_t* buf)
{
	int32_t number = static_cast<int32_t>((buf[2] << 16) | (buf[1] << 8) | buf[0]);

	if (number >= 8388608)
	{
		number -= 16777216;
	}

	return number;
}
