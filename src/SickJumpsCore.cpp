#include "SickJumpsCore.h"

#include <cmath>

#include <sstream>



SickJumpsCore::SickJumpsCore()
{
}



SickJumpsCore::SickJumpsCore(int _frameCount, int _firstFrame, int _lastFrame, double _fps,
	double _upSeconds, double _downSeconds, double _startMultiplier, double _fullMultiplier, double _endMultiplier,
	__int64 _audioSamplesPerFrame)
	:
	upSeconds(_upSeconds), downSeconds(_downSeconds),
	startMultiplier(_startMultiplier), fullMultiplier(_fullMultiplier), endMultiplier(_endMultiplier),
	audioSamplesPerFrame(_audioSamplesPerFrame),
	originalFrameCount(_frameCount), originalSampleCount(_audioSamplesPerFrame * _frameCount),
	averageMultiplier((startMultiplier + fullMultiplier) / 2.0),
	downAverageVideoMultiplier((endMultiplier + fullMultiplier) / 2.0),
	downAverageAudioMultiplier(downAverageVideoMultiplier)
{
	// -- Video -- //

	// Ranges in Avisynth tend to feel more natural when inclusive, but for a pair
	// of durations explicitly set by the user, exclusive is more precise.
	int rampUpOutputFrames = static_cast<int>(_fps * upSeconds);
	int rampDownOutputFrames = static_cast<int>(_fps * downSeconds);

	// -- Input frames

	int rampUpInputFrames = static_cast<int>(CalculateRampInputCount(0, rampUpOutputFrames, startMultiplier, fullMultiplier));
	int rampDownInputFrames = static_cast<int>(CalculateRampInputCount(0, rampDownOutputFrames, endMultiplier, fullMultiplier));

	// Establish preliminary locations.
	rampUpFirstInputFrame = _firstFrame;
	rampUpLastInputFrame = (rampUpFirstInputFrame + rampUpInputFrames) - 1;
	rampDownLastInputFrame = _lastFrame;
	rampDownFirstInputFrame = (rampDownLastInputFrame - rampDownInputFrames) + 1;
	fullSpeedFirstInputFrame = rampUpLastInputFrame + static_cast<int>(std::round(fullMultiplier));
	fullSpeedLastInputFrame = rampDownFirstInputFrame - static_cast<int>(std::round(fullMultiplier));
	afterFirstInputFrame = rampDownLastInputFrame + static_cast<int>(std::round(endMultiplier));

	// Tweak as necessary.
	int fullDiff = fullSpeedLastInputFrame - fullSpeedFirstInputFrame;
	int fullDiffSnapped = static_cast<int>(std::round(fullDiff / fullMultiplier) * fullMultiplier);
	fullSpeedLastInputFrame = fullSpeedFirstInputFrame + fullDiffSnapped;
	rampDownFirstInputFrame = fullSpeedLastInputFrame + static_cast<int>(std::round(fullMultiplier));

	// At this point the last frame of the full speed section and the first frame
	// of the down ramp have been snapped, but the number of output frames for the
	// down ramp remains the same. To fit the new input frame count into the same
	// output frame count, the down ramp needs its own multiplier tweaked as well.
	int downDistanceOutput = rampDownOutputFrames - 1;
	int calculatedDownDistanceInput = static_cast<int>(std::round(downDistanceOutput * downAverageVideoMultiplier));
	double originalDownAverageMultiplier = downAverageVideoMultiplier;
	int expectedDownDistanceInputFrames = rampDownLastInputFrame - rampDownFirstInputFrame;
	downAverageVideoMultiplier = static_cast<double>(expectedDownDistanceInputFrames) / static_cast<double>(downDistanceOutput);

	if (rampUpLastInputFrame >= rampDownFirstInputFrame)
	{
		std::ostringstream err;

		err
			<< "Ramps cannot overlap! For this clip, first_frame and last_frame must be at least "
			<< rampUpInputFrames + 1 + rampDownInputFrames
			<< " frames apart!";

		throw std::invalid_argument(err.str());
	}

	// -- Output frames

	rampUpFirstOutputFrame = static_cast<int>(std::round(rampUpFirstInputFrame / startMultiplier));
	rampUpLastOutputFrame = (rampUpFirstOutputFrame + rampUpOutputFrames) - 1;
	fullSpeedFirstOutputFrame = rampUpLastOutputFrame + 1;

	int fullSpeedTotalInputFrames = (fullSpeedLastInputFrame - fullSpeedFirstInputFrame) + 1;
	int fullSpeedTotalOutputFrames = static_cast<int>(std::ceil(fullSpeedTotalInputFrames / fullMultiplier));
	fullSpeedLastOutputFrame = (fullSpeedFirstOutputFrame + fullSpeedTotalOutputFrames) - 1;

	rampDownFirstOutputFrame = fullSpeedLastOutputFrame + 1;
	rampDownLastOutputFrame = (rampDownFirstOutputFrame + rampDownOutputFrames) - 1;

	// Since frame numbers are zero-based, the number of the ramp up's first frame
	// is also the total "before" frame count.
	int totalOutputFramesBefore = rampUpFirstOutputFrame;
	int totalOutputFramesDuring = rampUpOutputFrames + fullSpeedTotalOutputFrames + rampDownOutputFrames;
	int totalOutputFramesAfter = static_cast<int>(std::floor((originalFrameCount - 1 - _lastFrame) / endMultiplier));

	adjustedFrameCount = totalOutputFramesBefore + totalOutputFramesDuring + totalOutputFramesAfter;

	// -- Audio -- //

	__int64 rampUpOutputSamples = audioSamplesPerFrame * rampUpOutputFrames;
	__int64 rampDownOutputSamples = audioSamplesPerFrame * rampDownOutputFrames;

	// -- Input samples

	__int64 rampUpInputSamples = CalculateRampInputCount(0, rampUpOutputSamples, startMultiplier, fullMultiplier);
	__int64 rampDownInputSamples = CalculateRampInputCount(0, rampDownOutputSamples, endMultiplier, fullMultiplier);

	rampUpFirstInputSample = _audioSamplesPerFrame * rampUpFirstInputFrame;
	rampUpLastInputSample = (rampUpFirstInputSample + rampUpInputSamples) - 1;
	rampDownLastInputSample = (_audioSamplesPerFrame * (rampDownLastInputFrame + 1)) - 1;
	rampDownFirstInputSample = (rampDownLastInputSample - rampDownInputSamples) + 1;
	fullSpeedFirstInputSample = rampUpLastInputSample + static_cast<__int64>(std::round(fullMultiplier));
	fullSpeedLastInputSample = rampDownFirstInputSample - static_cast<__int64>(std::round(fullMultiplier));
	afterFirstInputSample = rampDownLastInputSample + static_cast<__int64>(std::round(endMultiplier));

	__int64 fullDiffSamples = fullSpeedLastInputSample - fullSpeedFirstInputSample;
	__int64 fullDiffSamplesSnapped = static_cast<__int64>(std::round(fullDiffSamples / fullMultiplier) * fullMultiplier);
	fullSpeedLastInputSample = fullSpeedFirstInputSample + fullDiffSamplesSnapped;
	rampDownFirstInputSample = fullSpeedLastInputSample + static_cast<__int64>(std::round(fullMultiplier));

	__int64 downDistanceOutputSamples = rampDownOutputSamples - 1;
	__int64 calculatedDownDistanceInputSamples = static_cast<int>(std::round(downDistanceOutputSamples * originalDownAverageMultiplier));
	__int64 expectedDownDistanceInputSamples = rampDownLastInputSample - rampDownFirstInputSample;
	downAverageAudioMultiplier = static_cast<double>(expectedDownDistanceInputSamples) / static_cast<double>(downDistanceOutputSamples);

	// -- Output samples

	rampUpFirstOutputSample = static_cast<__int64>(std::round(rampUpFirstInputSample / startMultiplier));
	rampUpLastOutputSample = (rampUpFirstOutputSample + rampUpOutputSamples) - 1;
	fullSpeedFirstOutputSample = rampUpLastOutputSample + 1;

	__int64 fullSpeedTotalInputSamples = (fullSpeedLastInputSample - fullSpeedFirstInputSample) + 1;
	__int64 fullSpeedTotalOutputSamples = static_cast<__int64>(std::ceil(fullSpeedTotalInputSamples / fullMultiplier));
	fullSpeedLastOutputSample = (fullSpeedFirstOutputSample + fullSpeedTotalOutputSamples) - 1;

	rampDownFirstOutputSample = fullSpeedLastOutputSample + 1;
	rampDownLastOutputSample = (rampDownFirstOutputSample + rampDownOutputSamples) - 1;

	__int64 totalOutputSamplesBefore = rampUpFirstOutputSample;
	__int64 totalOutputSamplesDuring = rampUpOutputSamples + fullSpeedTotalOutputSamples + rampDownOutputSamples;
	__int64 totalOutputSamplesAfter = static_cast<__int64>(std::floor((originalSampleCount - 1 - rampDownLastInputSample) / endMultiplier));

	adjustedSampleCount = totalOutputSamplesBefore + totalOutputSamplesDuring + totalOutputSamplesAfter;
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
		adjustedSample = afterFirstInputSample + static_cast<__int64>(std::round(distance * endMultiplier));
	}
	else if (n >= fullSpeedFirstOutputSample && n <= fullSpeedLastOutputSample)
	{
		double multiplier = fullMultiplier;
		double distance = static_cast<double>(n - fullSpeedFirstOutputSample);
		adjustedSample = fullSpeedFirstInputSample + static_cast<__int64>(std::round(distance * multiplier));
	}
	else if (n >= rampUpFirstOutputSample && n <= rampUpLastOutputSample)
	{
		double multiplier = GetCurrentMultiplier(n, rampUpFirstOutputSample, rampUpLastOutputSample, startMultiplier, averageMultiplier);
		double distance = static_cast<double>(n - rampUpFirstOutputSample);
		adjustedSample = rampUpFirstInputSample + static_cast<__int64>(std::round(distance * multiplier));
	}
	else // Ramp down
	{
		double multiplier = GetCurrentMultiplier(n, rampDownFirstOutputSample, rampDownLastOutputSample, endMultiplier, downAverageAudioMultiplier);
		multiplier = (endMultiplier + downAverageAudioMultiplier) - multiplier;
		double distance = static_cast<double>(rampDownLastOutputSample - n);
		adjustedSample = rampDownLastInputSample - static_cast<__int64>(std::round(distance * multiplier));
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
		multiplier = endMultiplier;
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
		multiplier = GetCurrentMultiplier(n, rampUpFirstOutputFrame, rampUpLastOutputFrame, startMultiplier, averageMultiplier);
		int distance = n - rampUpFirstOutputFrame;
		adjustedFrame = rampUpFirstInputFrame + static_cast<int>(std::round(distance * multiplier));

		multiplier = GetCurrentMultiplier(n, rampUpFirstOutputFrame, rampUpLastOutputFrame, startMultiplier, fullMultiplier);
		text = "ramp up";
	}
	else // Ramp down
	{
		multiplier = GetCurrentMultiplier(n, rampDownFirstOutputFrame, rampDownLastOutputFrame, endMultiplier, downAverageVideoMultiplier);
		multiplier = (endMultiplier + downAverageVideoMultiplier) - multiplier;
		int distance = rampDownLastOutputFrame - n;
		adjustedFrame = rampDownLastInputFrame - static_cast<int>(std::round(distance * multiplier));

		// Now get the friendly display version of the multiplier.
		multiplier = GetCurrentMultiplier(n, rampDownFirstOutputFrame, rampDownLastOutputFrame, endMultiplier, fullMultiplier);
		multiplier = (endMultiplier + fullMultiplier) - multiplier;
		text = "ramp down";
	}

	return std::make_tuple(adjustedFrame, multiplier, text);
}



double GetCurrentMultiplier(__int64 current, __int64 first, __int64 last, double startMultiplier, double fullMultiplier)
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



// Calculate the number of input frames, or samples, required for a speed ramp.
__int64 CalculateRampInputCount(__int64 first, __int64 total, double startMultiplier, double endMultiplier)
{
	__int64 inCount = 0;

	double averageMultiplier = (startMultiplier + endMultiplier) / 2.0;

	for (__int64 i = 0; i < total; ++i)
	{
		double step = GetCurrentMultiplier(i, 0, total - 1, startMultiplier, averageMultiplier);
		inCount = static_cast<__int64>(std::round(step * static_cast<double>(i)));
	}

	if (inCount > 0)
	{
		// Don't forget to include the first one!
		inCount++;
	}

	return inCount;
}
