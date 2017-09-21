#include "SickJumps.h"

#include <cmath>
#include <cstring>

#include <sstream>
#include <vector>

#include <avisynth.h>



SickJumps::SickJumps(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _endMultiplier,
	SFLOAT _inSeconds, SFLOAT _outSeconds, int _mode, IScriptEnvironment * env)
	:
	GenericVideoFilter(_child),	startMultiplier(_startMultiplier), fullMultiplier(_endMultiplier),
	upSeconds(_inSeconds), downSeconds(_outSeconds), mode(_mode)
{
	rampUpFirstInputFrame = _firstFrame;
	rampDownLastInputFrame = _lastFrame;

	double fps = vi.fps_numerator / vi.fps_denominator;

	// Ranges in Avisynth tend to feel more natural when inclusive, but for a pair
	// of durations explicitly set by the user, exclusive is more precise.
	int rampUpOutputFrames = static_cast<int>(fps * upSeconds);
	int rampDownOutputFrames = static_cast<int>(fps * downSeconds);

	rampUpFirstOutputFrame = rampUpFirstInputFrame;
	rampUpLastOutputFrame = (rampUpFirstOutputFrame + rampUpOutputFrames) - 1;

	// The total number of ramp frames will be the same no matter where it starts,
	// so using an input range of 0 through rampUpOutputFrames works just fine.
	rampUpLastInputFrame = (rampUpFirstInputFrame + CalculateRampInputFrames(0, rampUpOutputFrames, startMultiplier, fullMultiplier, env)) - 1;

	fullSpeedFirstInputFrame = rampUpLastInputFrame + static_cast<int>(std::round(fullMultiplier));

	// After an initial rough placement, snap the full speed frame count to the
	// nearest multiple of the full multiplier, relative to the section start.
	fullSpeedLastInputFrame = rampDownLastInputFrame - CalculateRampInputFrames(0, rampDownOutputFrames, startMultiplier, fullMultiplier, env);
	int fullSpeedTotalInputFrames = (fullSpeedLastInputFrame - fullSpeedFirstInputFrame) + 1;
	fullSpeedLastInputFrame = fullSpeedFirstInputFrame + static_cast<int>(std::round(fullSpeedTotalInputFrames / fullMultiplier) * fullMultiplier);

	fullSpeedFirstOutputFrame = rampUpLastOutputFrame + 1;
	int fullSpeedTotalOutputFrames = static_cast<int>(std::round(fullSpeedTotalInputFrames / fullMultiplier));
	fullSpeedLastOutputFrame = (fullSpeedFirstOutputFrame + fullSpeedTotalOutputFrames) - 1;

	rampDownFirstOutputFrame = fullSpeedLastOutputFrame + 1;
	rampDownLastOutputFrame = (rampDownFirstOutputFrame + rampDownOutputFrames) - 1;

	rampDownFirstInputFrame = fullSpeedLastInputFrame + static_cast<int>(std::round(fullMultiplier));

	int totalOutputFrames = rampUpFirstOutputFrame + rampUpOutputFrames + fullSpeedTotalOutputFrames + rampDownOutputFrames + (vi.num_frames - 1 - _lastFrame);

	vi.num_frames = totalOutputFrames;

	// Subtract or add as necessary to ensure the audio ramps run all the way
	// through the start and end video frames of their respective video ramps.
	__int64 hz = vi.AudioSamplesFromFrames(1);
	rampUpFirstOutputSample = (vi.AudioSamplesFromFrames(rampUpFirstOutputFrame) - hz) + 1;
	rampUpLastOutputSample = vi.AudioSamplesFromFrames(rampUpLastOutputFrame);

	fullSpeedFirstOutputSample = (vi.AudioSamplesFromFrames(fullSpeedFirstOutputFrame) - hz) + 1;
	fullSpeedLastOutputSample = vi.AudioSamplesFromFrames(fullSpeedLastOutputFrame);

	rampDownFirstOutputSample = vi.AudioSamplesFromFrames(rampDownFirstOutputFrame - 1) + 1;
	rampDownLastOutputSample = vi.AudioSamplesFromFrames(rampDownLastOutputFrame);

	rampUpFirstInputSample = (vi.AudioSamplesFromFrames(rampUpFirstInputFrame) - hz) + 1;
	rampUpLastInputSample = vi.AudioSamplesFromFrames(rampUpLastOutputFrame);

	fullSpeedFirstInputSample = (vi.AudioSamplesFromFrames(fullSpeedFirstInputFrame) - hz) + 1;
	fullSpeedLastInputSample = vi.AudioSamplesFromFrames(fullSpeedLastInputFrame);

	rampDownFirstInputSample = (vi.AudioSamplesFromFrames(rampDownFirstInputFrame) - hz) + 1;
	rampDownLastInputSample = vi.AudioSamplesFromFrames(rampDownLastInputFrame);
}



SickJumps::~SickJumps()
{
}



void __stdcall SickJumps::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
{
	__int64 totalSamples = count * vi.AudioChannels();

	std::vector<SFLOAT> currentSample;
	for (__int64 i = 0; i < totalSamples; ++i)
	{
		currentSample.push_back((SFLOAT)1.0f);
	}

	for (__int64 i = 0; i < count; ++i)
	{
		__int64 offset = start + i;

		std::vector<SFLOAT> sample;
		for (int j = 0; j < vi.AudioChannels(); ++j)
		{
			sample.push_back(1.0f);
		}

		__int64 adjustedSample = i;

		if (offset < rampUpFirstOutputSample)
		{
			adjustedSample = offset;
		}
		else if (offset > rampDownLastOutputSample)
		{
			int distance = (offset - (rampDownLastOutputSample + 1));
			adjustedSample = rampDownLastInputSample + 1 + distance;
		}
		else if (offset >= fullSpeedFirstOutputSample && offset <= fullSpeedLastOutputSample)
		{
			double step = fullMultiplier;
			int distance = offset - fullSpeedFirstOutputSample;
			adjustedSample = fullSpeedFirstInputSample + (step * distance);
		}
		else if (offset >= rampUpFirstOutputSample && offset <= rampUpLastOutputSample)
		{
			double averageMultiplier = (startMultiplier + fullMultiplier) / 2.0;
			double step = GetCurrentMultiplier(offset, rampUpFirstOutputSample, rampUpLastOutputSample, startMultiplier, averageMultiplier, mode, env);
			int distance = offset - rampUpFirstOutputSample;
			adjustedSample = rampUpFirstInputSample + static_cast<int>(std::round(step * distance));
		}
		else // Ramp down
		{
			double averageMultiplier = (startMultiplier + fullMultiplier) / 2.0;

			double step = GetCurrentMultiplier(offset, rampDownFirstOutputSample, rampDownLastOutputSample, startMultiplier, averageMultiplier, mode, env);
			step = (startMultiplier + averageMultiplier) - step;
			int distance = rampDownLastOutputSample - offset;
			adjustedSample = rampDownLastInputSample - static_cast<int>(std::floor(step * distance));
		}

		child->GetAudio(sample.data(), static_cast<__int64>(adjustedSample), 1, env);

		memcpy(currentSample.data() + (i * vi.AudioChannels()), sample.data(), vi.BytesFromAudioSamples(1));
	}

	__int64 bytesPerSample = vi.BytesFromAudioSamples(1);
	memcpy((SFLOAT*)buf, currentSample.data(), count * bytesPerSample);
}




PVideoFrame __stdcall SickJumps::GetFrame(int n, IScriptEnvironment* env)
{
	int adjustedFrame = n;

	std::string text = "";

	if (n < rampUpFirstOutputFrame)
	{
		adjustedFrame = n;
		//text = "Before";
	}
	else if (n > rampDownLastOutputFrame)
	{
		int distance = (n - (rampDownLastOutputFrame + 1));
		adjustedFrame = rampDownLastInputFrame + 1 + distance;
		//text = "After";
	}
	else if (n >= fullSpeedFirstOutputFrame && n <= fullSpeedLastOutputFrame)
	{
		double step = fullMultiplier;
		int distance = n - fullSpeedFirstOutputFrame;
		adjustedFrame = fullSpeedFirstInputFrame + (step * distance);
		//text = "Full speed!";
	}
	else if (n >= rampUpFirstOutputFrame && n <= rampUpLastOutputFrame)
	{
		double averageMultiplier = (startMultiplier + fullMultiplier) / 2.0;
		double step = GetCurrentMultiplier(n, rampUpFirstOutputFrame, rampUpLastOutputFrame, startMultiplier, averageMultiplier, mode, env);
		int distance = n - rampUpFirstOutputFrame;
		adjustedFrame = rampUpFirstInputFrame + static_cast<int>(std::round(step * distance));
		//text = "Ramp up";
	}
	else // Ramp down
	{
		double averageMultiplier = (startMultiplier + fullMultiplier) / 2.0;

		double step = GetCurrentMultiplier(n, rampDownFirstOutputFrame, rampDownLastOutputFrame, startMultiplier, averageMultiplier, mode, env);
		step = (startMultiplier + averageMultiplier) - step;
		int distance = rampDownLastOutputFrame - n;
		adjustedFrame = rampDownLastInputFrame - static_cast<int>(std::floor(step * distance));

		//text = "Ramp down";
	}

	PVideoFrame frame = child->GetFrame(adjustedFrame, env);

	if (text != "")
	{
		std::ostringstream oss;
		oss << text;

		env->MakeWritable(&frame);
		env->ApplyMessage(&frame, vi, oss.str().c_str(), 1920, 0xFFFFFF, 0, 0);
	}

	return frame;
}



double GetCurrentMultiplier(__int64 current, __int64 first, __int64 last, SFLOAT startMultiplier, SFLOAT fullMultiplier, int mode, IScriptEnvironment * env)
{
	double multiplier;

	// The highest multiplier needs to be the average of the requested range, or
	// the footage will speed up too much, like walking while on a moving sidewalk.
	if (startMultiplier == fullMultiplier)
	{
		multiplier = fullMultiplier;
	}
	else if (current < first)
	{
		multiplier = startMultiplier;
	}
	else if (current > last)
	{
		multiplier = fullMultiplier;
	}
	else
	{
		double distance = ScaleToRange(current, first, last - first, 0.0, 1.0);

		if (mode == SickJumps::MODE_SPLINE)
		{
			double nudge = ((fullMultiplier - startMultiplier) * 0.25);

			double
				x0 = first,
				y0 = startMultiplier,

				x1 = first + ((last - first) * 0.1),
				y1 = startMultiplier + nudge,

				x2 = last - ((last - first) * 0.1),
				y2 = fullMultiplier - nudge,

				x3 = last,
				y3 = fullMultiplier;

			AVSValue args[10] = { static_cast<double>(current), x0, y0, x1, y1, x2, y2, x3, y3, false };

			multiplier = env->Invoke("Spline", AVSValue(args, 8)).AsFloat();
		}
		else
		{
			multiplier = ScaleToRange(current, first, last, startMultiplier, fullMultiplier);
		}
	}

	return multiplier;
}



double GetDistance(__int64 current, __int64 first, __int64 last, int mode, IScriptEnvironment * env)
{
	double distance;

	if (mode == SickJumps::MODE_SPLINE)
	{
		double nudge = 0.25;

		SFLOAT
			x0 = first,
			y0 = 0.0f,

			x1 = first + ((last - first) * 0.1),
			y1 = nudge,

			x2 = last - ((last - first) * 0.1),
			y2 = 1.0f - nudge,

			x3 = last,
			y3 = 1.0;

		AVSValue args[10] = { static_cast<SFLOAT>(current), x0, y0, x1, y1, x2, y2, x3, y3, false };

		distance = env->Invoke("Spline", AVSValue(args, 10)).AsFloat();
	}
	else
	{
		distance = ScaleToRange(current, first, last, 0.0, 1.0);
	}

	return distance;
}



double ScaleToRange(double value, double inMin, double inMax, double outMin, double outMax)
{
	double rangeIn = inMax - inMin;
	double rangeOut = outMax - outMin;

	double scaled;
	if (outMin == outMax)
	{
		scaled = outMin;
	}
	else
	{
		scaled = ((rangeOut * (value - inMin)) / rangeIn) + outMin;
	}

	return scaled;
}



// Calculate the number of input frames required for a speed ramp.
int SickJumps::CalculateRampInputFrames(int _firstInputFrame, int _totalOutputFrames, SFLOAT _startMultiplier, SFLOAT _endMultiplier, IScriptEnvironment* env)
{
	int inFrames = 0;

	for (int i = 0; i < _totalOutputFrames; ++i)
	{
		double averageMultiplier = (startMultiplier + _endMultiplier) / 2.0;
		double step = GetCurrentMultiplier(i, 0, _totalOutputFrames - 1, startMultiplier, averageMultiplier, mode, env);
		inFrames = static_cast<int>(std::round(step * i));
		int breakvar = 4;
	}

	// Don't forget to include the first frame!
	return inFrames + 1;
}