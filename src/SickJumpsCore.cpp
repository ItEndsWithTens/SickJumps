#include "SickJumpsCore.h"

#include <cmath>



SickJumpsCore::SickJumpsCore()
{
}



SickJumpsCore::SickJumpsCore(int _frameCount, int _firstFrame, int _lastFrame, double _fps,
	double _upSeconds, double _downSeconds,	double _startMultiplier, double _fullMultiplier,
	__int64 _audioSamplesPerFrame, int _mode)
	:
	upSeconds(_upSeconds), downSeconds(_downSeconds),
	startMultiplier(_startMultiplier), fullMultiplier(_fullMultiplier),
	mode(_mode)
{
	originalFrameCount = _frameCount;

	rampUpFirstInputFrame = _firstFrame;
	rampDownLastInputFrame = _lastFrame;

	// Ranges in Avisynth tend to feel more natural when inclusive, but for a pair
	// of durations explicitly set by the user, exclusive is more precise.
	int rampUpOutputFrames = static_cast<int>(_fps * upSeconds);
	int rampDownOutputFrames = static_cast<int>(_fps * downSeconds);

	rampUpFirstOutputFrame = static_cast<int>(std::round(rampUpFirstInputFrame / startMultiplier));
	rampUpLastOutputFrame = (rampUpFirstOutputFrame + rampUpOutputFrames) - 1;

	// The total number of ramp frames will be the same no matter where it starts,
	// so using an input range of 0 through rampUpOutputFrames works just fine.
	rampUpLastInputFrame = (rampUpFirstInputFrame + CalculateRampInputFrames(0, rampUpOutputFrames, startMultiplier, fullMultiplier, mode)) - 1;

	fullSpeedFirstInputFrame = rampUpLastInputFrame + static_cast<int>(std::round(fullMultiplier));

	// After an initial rough placement, snap the full speed frame count to the
	// nearest multiple of the full multiplier, relative to the section start.
	fullSpeedLastInputFrame = rampDownLastInputFrame - CalculateRampInputFrames(0, rampDownOutputFrames, startMultiplier, fullMultiplier, mode);
	int fullSpeedTotalInputFrames = (fullSpeedLastInputFrame - fullSpeedFirstInputFrame) + 1;
	fullSpeedLastInputFrame = fullSpeedFirstInputFrame + static_cast<int>(std::round(fullSpeedTotalInputFrames / fullMultiplier) * fullMultiplier);
	fullSpeedTotalInputFrames = (fullSpeedLastInputFrame - fullSpeedFirstInputFrame) + 1;

	fullSpeedFirstOutputFrame = rampUpLastOutputFrame + 1;
	int fullSpeedTotalOutputFrames = static_cast<int>(std::round(fullSpeedTotalInputFrames / fullMultiplier));
	fullSpeedLastOutputFrame = (fullSpeedFirstOutputFrame + fullSpeedTotalOutputFrames) - 1;

	rampDownFirstOutputFrame = fullSpeedLastOutputFrame + 1;
	rampDownLastOutputFrame = (rampDownFirstOutputFrame + rampDownOutputFrames) - 1;

	rampDownFirstInputFrame = fullSpeedLastInputFrame + static_cast<int>(std::round(fullMultiplier));

	afterFirstInputFrame = rampDownLastInputFrame + static_cast<int>(std::round(startMultiplier));

	// Since frame numbers are zero-based, the number of the ramp up's first frame
	// is also the total "before" frame count.
	int totalOutputFramesBefore = rampUpFirstOutputFrame;
	int totalOutputFramesDuring = rampUpOutputFrames + fullSpeedTotalOutputFrames + rampDownOutputFrames;
	int totalOutputFramesAfter = static_cast<int>(std::floor((originalFrameCount - 1 - _lastFrame) / startMultiplier));

	adjustedFrameCount = totalOutputFramesBefore + totalOutputFramesDuring + totalOutputFramesAfter;

	// Since frames in Avisynth are numbered from zero, a given frame number also
	// serves as the count of frames before it; the same goes for samples.
	rampUpFirstOutputSample = _audioSamplesPerFrame * rampUpFirstOutputFrame;
	rampUpLastOutputSample = (_audioSamplesPerFrame * (rampUpLastOutputFrame + 1)) - 1;

	fullSpeedFirstOutputSample = _audioSamplesPerFrame * fullSpeedFirstOutputFrame;
	fullSpeedLastOutputSample = (_audioSamplesPerFrame * (fullSpeedLastOutputFrame + 1)) - 1;

	rampDownFirstOutputSample = _audioSamplesPerFrame * rampDownFirstOutputFrame;
	rampDownLastOutputSample = (_audioSamplesPerFrame * (rampDownLastOutputFrame + 1)) - 1;

	rampUpFirstInputSample = _audioSamplesPerFrame * rampUpFirstInputFrame;
	rampDownLastInputSample = (_audioSamplesPerFrame * (rampDownLastInputFrame + 1)) - 1;

	__int64 rampUpInputSamples = CalculateRampInputSamples(0, _audioSamplesPerFrame * rampUpOutputFrames, startMultiplier, fullMultiplier, mode);
	fullSpeedFirstInputSample = ((rampUpFirstInputSample + rampUpInputSamples) - 1) + static_cast<__int64>(std::round(fullMultiplier));

	afterFirstInputSample = rampDownLastInputSample + static_cast<__int64>(std::round(startMultiplier));

	adjustedSampleCount = _audioSamplesPerFrame * adjustedFrameCount;
}



SickJumpsCore::~SickJumpsCore()
{
}



__int64 SickJumpsCore::GetAdjustedSampleNumber(__int64 n)
{
	__int64 adjustedSample = n;

	if (n < rampUpFirstOutputSample)
	{
		double distance = static_cast<double>(n);
		adjustedSample = static_cast<__int64>(std::round(distance * startMultiplier));
	}
	else if (n > rampDownLastOutputSample)
	{
		double distance = static_cast<double>(n - (rampDownLastOutputSample + 1));
		adjustedSample = afterFirstInputSample + static_cast<__int64>(std::round(distance * startMultiplier));
	}
	else if (n >= fullSpeedFirstOutputSample && n <= fullSpeedLastOutputSample)
	{
		double multiplier = fullMultiplier;
		double distance = static_cast<double>(n - fullSpeedFirstOutputSample);
		adjustedSample = fullSpeedFirstInputSample + static_cast<__int64>(std::round(distance * multiplier));
	}
	else if (n >= rampUpFirstOutputSample && n <= rampUpLastOutputSample)
	{
		double averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
		double multiplier = GetCurrentMultiplier(n, rampUpFirstOutputSample, rampUpLastOutputSample, startMultiplier, averageMultiplier, mode);
		double distance = static_cast<double>(n - rampUpFirstOutputSample);
		adjustedSample = rampUpFirstInputSample + static_cast<__int64>(std::round(distance * multiplier));
	}
	else // Ramp down
	{
		double averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
		double multiplier = GetCurrentMultiplier(n, rampDownFirstOutputSample, rampDownLastOutputSample, startMultiplier, averageMultiplier, mode);
		multiplier = (startMultiplier + averageMultiplier) - multiplier;
		double distance = static_cast<double>(rampDownLastOutputSample - n);
		adjustedSample = rampDownLastInputSample - static_cast<__int64>(std::floor(distance * multiplier));
	}

	return adjustedSample;
}



std::tuple<int, double, std::string> SickJumpsCore::GetAdjustedFrameProperties(int n)
{
	int adjustedFrame = n;
	double multiplier = 0.0;
	std::string text = "";

	if (n < rampUpFirstOutputFrame)
	{
		multiplier = startMultiplier;
		adjustedFrame = static_cast<int>(std::round(n * multiplier));
		text = "before";
	}
	else if (n > rampDownLastOutputFrame)
	{
		multiplier = startMultiplier;
		int distance = (n - (rampDownLastOutputFrame + 1));
		adjustedFrame = afterFirstInputFrame + static_cast<int>(std::round(distance * multiplier));
		text = "after";
	}
	else if (n >= fullSpeedFirstOutputFrame && n <= fullSpeedLastOutputFrame)
	{
		multiplier = fullMultiplier;
		int distance = n - fullSpeedFirstOutputFrame;
		adjustedFrame = fullSpeedFirstInputFrame + static_cast<int>(std::round(distance * multiplier));
		text = "full speed";
	}
	else if (n >= rampUpFirstOutputFrame && n <= rampUpLastOutputFrame)
	{
		// The highest multiplier during ramps needs to be the average of the range,
		// or clips will speed up too far; like walking while on a moving sidewalk.
		double averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
		int distance = n - rampUpFirstOutputFrame;
		multiplier = GetCurrentMultiplier(n, rampUpFirstOutputFrame, rampUpLastOutputFrame, startMultiplier, averageMultiplier, mode);
		adjustedFrame = rampUpFirstInputFrame + static_cast<int>(std::round(distance * multiplier));

		multiplier = GetCurrentMultiplier(n, rampUpFirstOutputFrame, rampUpLastOutputFrame, startMultiplier, fullMultiplier, mode);
		text = "ramp up";
	}
	else // Ramp down
	{
		double averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
		multiplier = GetCurrentMultiplier(n, rampDownFirstOutputFrame, rampDownLastOutputFrame, startMultiplier, averageMultiplier, mode);
		multiplier = (startMultiplier + averageMultiplier) - multiplier;
		int distance = rampDownLastOutputFrame - n;

		// The inside of the down ramp is a good place to hide the potential seam from
		// allowing arbitrary start/end frames and start/full multipliers; this gets
		// floored to try to avoid overlapping the full speed segment's end.
		adjustedFrame = rampDownLastInputFrame - static_cast<int>(std::floor(distance * multiplier));

		// Now get the friendly display version of the multiplier.
		multiplier = GetCurrentMultiplier(n, rampDownFirstOutputFrame, rampDownLastOutputFrame, startMultiplier, fullMultiplier, mode);
		multiplier = (startMultiplier + fullMultiplier) - multiplier;
		text = "ramp down";
	}

	return std::make_tuple(adjustedFrame, multiplier, text);
}



double GetCurrentMultiplier(__int64 current, __int64 first, __int64 last, double startMultiplier, double fullMultiplier, int mode)
{
	double multiplier;

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
		multiplier = ScaleToRange(current, first, last, startMultiplier, fullMultiplier);
	}

	return multiplier;
}



double ScaleToRange(__int64 value, __int64 inMin, __int64 inMax, double outMin, double outMax)
{
	__int64 rangeIn = inMax - inMin;
	double rangeOut = outMax - outMin;

	double scaled;
	if (outMin == outMax)
	{
		scaled = outMin;
	}
	else
	{
		double distance = static_cast<double>(value - inMin);
		scaled = ((rangeOut * distance) / static_cast<double>(rangeIn)) + outMin;
	}

	return scaled;
}



// Calculate the number of input frames required for a speed ramp.
int CalculateRampInputFrames(int firstInputFrame, int totalOutputFrames, double startMultiplier, double endMultiplier, int mode)
{
	int inFrames = 0;

	for (int i = 0; i < totalOutputFrames; ++i)
	{
		double averageMultiplier = (startMultiplier + endMultiplier) / 2.0;
		double step = GetCurrentMultiplier(i, 0, totalOutputFrames - 1, startMultiplier, averageMultiplier, mode);
		inFrames = static_cast<int>(std::round(step * i));
		int breakvar = 4;
	}

	if (inFrames > 0)
	{
		// Don't forget to include the first frame!
		inFrames++;
	}

	return inFrames;
}



// Calculate the number of input samples required for a speed ramp.
__int64 CalculateRampInputSamples(__int64 firstInputSample, __int64 totalOutputSamples, double startMultiplier, double endMultiplier, int mode)
{
	__int64 inFrames = 0;

	for (__int64 i = 0; i < totalOutputSamples; ++i)
	{
		double averageMultiplier = (startMultiplier + endMultiplier) / 2.0;
		double step = GetCurrentMultiplier(i, 0, totalOutputSamples - 1, startMultiplier, averageMultiplier, mode);
		inFrames = static_cast<__int64>(std::round(step * static_cast<double>(i)));
		int breakvar = 4;
	}

	if (inFrames > 0)
	{
		// Don't forget to include the first frame!
		inFrames++;
	}

	return inFrames;
}
