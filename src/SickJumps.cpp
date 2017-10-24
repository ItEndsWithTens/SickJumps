#include "SickJumps.h"

#include <cmath>
#include <cstring>

#include <sstream>
#include <vector>

#include <avisynth.h>



SickJumps::SickJumps(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _fullMultiplier,
	SFLOAT _upSeconds, SFLOAT _downSeconds, int _mode, std::string _scriptVariable, IScriptEnvironment * env)
	:
	GenericVideoFilter(_child), startMultiplier(_startMultiplier), fullMultiplier(_fullMultiplier),
	upSeconds(_upSeconds), downSeconds(_downSeconds), mode(_mode), scriptVariable(_scriptVariable), setScriptVariable(_scriptVariable != "" ? true : false)
{
	rampUpFirstInputFrame = _firstFrame;
	rampDownLastInputFrame = _lastFrame;

	double fps = vi.fps_numerator / vi.fps_denominator;

	// Ranges in Avisynth tend to feel more natural when inclusive, but for a pair
	// of durations explicitly set by the user, exclusive is more precise.
	int rampUpOutputFrames = static_cast<int>(fps * upSeconds);
	int rampDownOutputFrames = static_cast<int>(fps * downSeconds);

	rampUpFirstOutputFrame = static_cast<int>(std::round(rampUpFirstInputFrame / _startMultiplier));
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

	afterFirstInputFrame = rampDownLastInputFrame + static_cast<int>(std::round(startMultiplier));

	// Since frame numbers are zero-based, the number of the ramp up's first frame
	// is also the total "before" frame count.
	int totalOutputFramesBefore = rampUpFirstOutputFrame;
	int totalOutputFramesDuring = rampUpOutputFrames + fullSpeedTotalOutputFrames + rampDownOutputFrames;
	int totalOutputFramesAfter = static_cast<int>(std::floor((vi.num_frames - 1 - _lastFrame) / startMultiplier));

	vi.num_frames = totalOutputFramesBefore + totalOutputFramesDuring + totalOutputFramesAfter;

	// Since frames in Avisynth are numbered from zero, a given frame number also
	// serves as the count of frames before it; the same goes for samples.
	rampUpFirstOutputSample = vi.AudioSamplesFromFrames(rampUpFirstOutputFrame);
	rampUpLastOutputSample = vi.AudioSamplesFromFrames(rampUpLastOutputFrame + 1) - 1;

	fullSpeedFirstOutputSample = vi.AudioSamplesFromFrames(fullSpeedFirstOutputFrame);
	fullSpeedLastOutputSample = vi.AudioSamplesFromFrames(fullSpeedLastOutputFrame + 1) - 1;

	rampDownFirstOutputSample = vi.AudioSamplesFromFrames(rampDownFirstOutputFrame);
	rampDownLastOutputSample = vi.AudioSamplesFromFrames(rampDownLastOutputFrame + 1) - 1;

	rampUpFirstInputSample = vi.AudioSamplesFromFrames(rampUpFirstInputFrame) + static_cast<__int64>(std::round(startMultiplier));
	fullSpeedFirstInputSample = vi.AudioSamplesFromFrames(fullSpeedFirstInputFrame) + static_cast<__int64>(std::round(fullMultiplier));
	rampDownLastInputSample = vi.AudioSamplesFromFrames(rampDownLastInputFrame) - static_cast<__int64>(std::round(startMultiplier));

	afterFirstInputSample = rampDownLastInputSample + static_cast<__int64>(std::round(startMultiplier));

	vi.num_audio_samples = vi.AudioSamplesFromFrames(vi.num_frames);
}



SickJumps::~SickJumps()
{
}



void __stdcall SickJumps::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
{
	std::vector<SFLOAT> outputChunk;
	for (__int64 i = 0; i < count * vi.AudioChannels(); ++i)
	{
		outputChunk.push_back((SFLOAT)1.0f);
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
			double distance = static_cast<double>(offset);
			adjustedSample = static_cast<__int64>(std::round(distance * startMultiplier));
		}
		else if (offset > rampDownLastOutputSample)
		{
			double distance = static_cast<double>(offset - (rampDownLastOutputSample + 1));
			adjustedSample = afterFirstInputSample + static_cast<__int64>(std::round(distance * startMultiplier));
		}
		else if (offset >= fullSpeedFirstOutputSample && offset <= fullSpeedLastOutputSample)
		{
			double multiplier = fullMultiplier;
			double distance = static_cast<double>(offset - fullSpeedFirstOutputSample);
			adjustedSample = fullSpeedFirstInputSample + static_cast<__int64>(std::round(distance * multiplier));
		}
		else if (offset >= rampUpFirstOutputSample && offset <= rampUpLastOutputSample)
		{
			SFLOAT averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
			double multiplier = GetCurrentMultiplier(offset, rampUpFirstOutputSample, rampUpLastOutputSample, startMultiplier, averageMultiplier, mode, env);
			double distance = static_cast<double>(offset - rampUpFirstOutputSample);
			adjustedSample = rampUpFirstInputSample + static_cast<__int64>(std::round(distance * multiplier));
		}
		else // Ramp down
		{
			SFLOAT averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
			double multiplier = GetCurrentMultiplier(offset, rampDownFirstOutputSample, rampDownLastOutputSample, startMultiplier, averageMultiplier, mode, env);
			multiplier = (startMultiplier + averageMultiplier) - multiplier;
			double distance = static_cast<double>(rampDownLastOutputSample - offset);
			adjustedSample = rampDownLastInputSample - static_cast<__int64>(std::floor(distance * multiplier));
		}

		child->GetAudio(sample.data(), static_cast<__int64>(adjustedSample), 1, env);

		memcpy(outputChunk.data() + (i * vi.AudioChannels()), sample.data(), vi.BytesFromAudioSamples(1));
	}

	__int64 bytesPerSample = vi.BytesFromAudioSamples(1);
	memcpy((SFLOAT*)buf, outputChunk.data(), count * bytesPerSample);
}



PVideoFrame __stdcall SickJumps::GetFrame(int n, IScriptEnvironment* env)
{
	int adjustedFrame = n;

	double scriptVariableValue;

	if (n < rampUpFirstOutputFrame)
	{
		adjustedFrame = static_cast<int>(std::round(n * startMultiplier));

		scriptVariableValue = startMultiplier;
	}
	else if (n > rampDownLastOutputFrame)
	{
		int distance = (n - (rampDownLastOutputFrame + 1));
		adjustedFrame = afterFirstInputFrame + static_cast<int>(std::round(distance * startMultiplier));

		scriptVariableValue = startMultiplier;
	}
	else if (n >= fullSpeedFirstOutputFrame && n <= fullSpeedLastOutputFrame)
	{
		double multiplier = fullMultiplier;
		int distance = n - fullSpeedFirstOutputFrame;
		adjustedFrame = fullSpeedFirstInputFrame + static_cast<int>(std::round(distance * multiplier));

		scriptVariableValue = multiplier;
	}
	else if (n >= rampUpFirstOutputFrame && n <= rampUpLastOutputFrame)
	{
		// The highest multiplier during ramps needs to be the average of the range,
		// or clips will speed up too far; like walking while on a moving sidewalk.
		SFLOAT averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
		int distance = n - rampUpFirstOutputFrame;
		double multiplier = GetCurrentMultiplier(n, rampUpFirstOutputFrame, rampUpLastOutputFrame, startMultiplier, averageMultiplier, mode, env);
		adjustedFrame = rampUpFirstInputFrame + static_cast<int>(std::round(distance * multiplier));

		scriptVariableValue = GetCurrentMultiplier(n, rampUpFirstOutputFrame, rampUpLastOutputFrame, startMultiplier, fullMultiplier, mode, env);
	}
	else // Ramp down
	{
		SFLOAT averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
		double multiplier = GetCurrentMultiplier(n, rampDownFirstOutputFrame, rampDownLastOutputFrame, startMultiplier, averageMultiplier, mode, env);
		multiplier = (startMultiplier + averageMultiplier) - multiplier;
		int distance = rampDownLastOutputFrame - n;

		// The inside of the down ramp is a good place to hide the potential seam from
		// allowing arbitrary start/end frames and start/full multipliers; this gets
		// floored to try to avoid overlapping the full speed segment's end.
		adjustedFrame = rampDownLastInputFrame - static_cast<int>(std::floor(distance * multiplier));

		scriptVariableValue = GetCurrentMultiplier(n, rampDownFirstOutputFrame, rampDownLastOutputFrame, startMultiplier, fullMultiplier, mode, env);
		scriptVariableValue = (startMultiplier + fullMultiplier) - scriptVariableValue;
	}

	if (setScriptVariable)
	{
		env->SetVar(scriptVariable.c_str(), AVSValue(scriptVariableValue));
	}

	return child->GetFrame(adjustedFrame, env);
}



int __stdcall SickJumps::SetCacheHints(int cachehints, int frame_range)
{
	int hints = 0;

	if (cachehints == CACHE_GET_MTMODE)
	{
		hints = MT_NICE_FILTER;
	}
	else
	{
		if (setScriptVariable)
		{
			hints = CACHE_ACCESS_SEQ1;
		}
		else
		{
			hints = CACHE_ACCESS_RAND;
		}
	}

	return hints;
}



double GetCurrentMultiplier(__int64 current, __int64 first, __int64 last, SFLOAT startMultiplier, SFLOAT fullMultiplier, int mode, IScriptEnvironment * env)
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
		if (mode == SickJumps::MODE_SPLINE)
		{
			SFLOAT nudge = ((fullMultiplier - startMultiplier) * 0.1f);

			// The casts are tragic, yes, but such are the ways of Avisynth floats.
			SFLOAT
				x0 = static_cast<SFLOAT>(first),
				y0 = static_cast<SFLOAT>(startMultiplier),

				x1 = static_cast<SFLOAT>(first + ((last - first) * 0.25)),
				y1 = static_cast<SFLOAT>(startMultiplier + nudge),

				x2 = static_cast<SFLOAT>(last - ((last - first) * 0.25)),
				y2 = static_cast<SFLOAT>(fullMultiplier - nudge),

				x3 = static_cast<SFLOAT>(last),
				y3 = static_cast<SFLOAT>(fullMultiplier);

			AVSValue args[10] = { static_cast<double>(current), x0, y0, x1, y1, x2, y2, x3, y3, false };

			multiplier = env->Invoke("Spline", AVSValue(args, 10)).AsFloat();
		}
		else
		{
			multiplier = ScaleToRange(current, first, last, startMultiplier, fullMultiplier);
		}
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
int SickJumps::CalculateRampInputFrames(int _firstInputFrame, int _totalOutputFrames, SFLOAT _startMultiplier, SFLOAT _endMultiplier, IScriptEnvironment* env)
{
	int inFrames = 0;

	for (int i = 0; i < _totalOutputFrames; ++i)
	{
		SFLOAT averageMultiplier = (startMultiplier + _endMultiplier) / 2.0f;
		double step = GetCurrentMultiplier(i, 0, _totalOutputFrames - 1, startMultiplier, averageMultiplier, mode, env);
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
