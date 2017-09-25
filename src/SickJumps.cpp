#include "SickJumps.h"

#include <cmath>
#include <cstring>

#include <sstream>
#include <vector>

#include <avisynth.h>



SickJumps::SickJumps(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _fullMultiplier,
	SFLOAT _inSeconds, SFLOAT _outSeconds, int _mode, std::string _scriptVariable, IScriptEnvironment * env)
	:
	GenericVideoFilter(_child), startMultiplier(_startMultiplier), fullMultiplier(_fullMultiplier),
	upSeconds(_inSeconds), downSeconds(_outSeconds), mode(_mode), scriptVariable(_scriptVariable), setScriptVariable(_scriptVariable != "" ? true : false)
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
	__int64 samplesPerFrame = vi.AudioSamplesFromFrames(1);
	rampUpFirstOutputSample = (vi.AudioSamplesFromFrames(rampUpFirstOutputFrame) - samplesPerFrame) + 1;
	rampUpLastOutputSample = vi.AudioSamplesFromFrames(rampUpLastOutputFrame);

	fullSpeedFirstOutputSample = (vi.AudioSamplesFromFrames(fullSpeedFirstOutputFrame) - samplesPerFrame) + 1;
	fullSpeedLastOutputSample = vi.AudioSamplesFromFrames(fullSpeedLastOutputFrame);

	rampDownFirstOutputSample = vi.AudioSamplesFromFrames(rampDownFirstOutputFrame - 1) + 1;
	rampDownLastOutputSample = vi.AudioSamplesFromFrames(rampDownLastOutputFrame);

	rampUpFirstInputSample = (vi.AudioSamplesFromFrames(rampUpFirstInputFrame) - samplesPerFrame) + 1;
	rampUpLastInputSample = vi.AudioSamplesFromFrames(rampUpLastOutputFrame);

	fullSpeedFirstInputSample = (vi.AudioSamplesFromFrames(fullSpeedFirstInputFrame) - samplesPerFrame) + 1;
	fullSpeedLastInputSample = vi.AudioSamplesFromFrames(fullSpeedLastInputFrame);

	rampDownFirstInputSample = (vi.AudioSamplesFromFrames(rampDownFirstInputFrame) - samplesPerFrame) + 1;
	rampDownLastInputSample = vi.AudioSamplesFromFrames(rampDownLastInputFrame);

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
			adjustedSample = offset;
		}
		else if (offset > rampDownLastOutputSample)
		{
			__int64 distance = (offset - (rampDownLastOutputSample + 1));
			adjustedSample = rampDownLastInputSample + 1 + distance;
		}
		else if (offset >= fullSpeedFirstOutputSample && offset <= fullSpeedLastOutputSample)
		{
			double multiplier = fullMultiplier;
			__int64 distance = offset - fullSpeedFirstOutputSample;
			adjustedSample = fullSpeedFirstInputSample + static_cast<__int64>(distance * multiplier);
		}
		else if (offset >= rampUpFirstOutputSample && offset <= rampUpLastOutputSample)
		{
			SFLOAT averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
			double multiplier = GetCurrentMultiplier(offset, rampUpFirstOutputSample, rampUpLastOutputSample, startMultiplier, averageMultiplier, mode, env);
			__int64 distance = offset - rampUpFirstOutputSample;
			adjustedSample = rampUpFirstInputSample + static_cast<__int64>(std::round(distance * multiplier));
		}
		else // Ramp down
		{
			SFLOAT averageMultiplier = (startMultiplier + fullMultiplier) / 2.0f;
			double multiplier = GetCurrentMultiplier(offset, rampDownFirstOutputSample, rampDownLastOutputSample, startMultiplier, averageMultiplier, mode, env);
			multiplier = (startMultiplier + averageMultiplier) - multiplier;
			__int64 distance = rampDownLastOutputSample - offset;
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
		adjustedFrame = n;

		scriptVariableValue = startMultiplier;
	}
	else if (n > rampDownLastOutputFrame)
	{
		int distance = (n - (rampDownLastOutputFrame + 1));
		adjustedFrame = rampDownLastInputFrame + 1 + distance;

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
