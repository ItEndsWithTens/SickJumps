#include "SickJumps.h"

#include <cmath>
#include <cstring>

#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>

#include "avisynth.h"



SickJumps::SickJumps(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _fullMultiplier,
	SFLOAT _endMultiplier, SFLOAT _upSeconds, SFLOAT _downSeconds, std::string _scriptVariable, IScriptEnvironment * env)
	:
	GenericVideoFilter(_child), scriptVariable(_scriptVariable), setScriptVariable(_scriptVariable != "" ? true : false)
{
	core = SickJumpsCore(vi.num_frames, _firstFrame, _lastFrame, vi.fps_numerator / vi.fps_denominator,
		_upSeconds, _downSeconds, _startMultiplier, _fullMultiplier, _endMultiplier, vi.AudioSamplesFromFrames(1));

	vi.num_frames = core.adjustedFrameCount;
	vi.num_audio_samples = core.adjustedSampleCount;
}



SickJumps::~SickJumps()
{
}



void __stdcall SickJumps::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
{
	// C++ arrays' [] operator takes a size_t argument, which means that although
	// count is provided as an __int64, buf could never be indexed with values
	// larger than an unsigned int on x86 systems.
	size_t size = static_cast<size_t>(count);

	std::vector<SFLOAT> outputChunk;

	for (size_t i = 0; i < size; ++i)
	{
		std::vector<SFLOAT> sample;
		for (int j = 0; j < vi.AudioChannels(); ++j)
		{
			sample.push_back(1.0f);
		}

		__int64 adjustedSample = core.GetAdjustedSampleNumber(start + i);

		child->GetAudio(sample.data(), adjustedSample, 1, env);

		for (size_t k = 0; k < sample.size(); ++k)
		{
			outputChunk.push_back(sample[k]);
		}
	}

	size_t bytes = vi.BytesPerAudioSample() * size;
	std::memcpy(reinterpret_cast<SFLOAT*>(buf), outputChunk.data(), bytes);
}



PVideoFrame __stdcall SickJumps::GetFrame(int n, IScriptEnvironment* env)
{
	int adjustedFrame;
	double multiplier;
	std::string text;

	std::tie(adjustedFrame, multiplier, text) = core.GetAdjustedFrameProperties(n);

	if (setScriptVariable)
	{
		std::ostringstream ss;

		ss
			<< "frame:" << adjustedFrame
			<< ":multiplier:" << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << multiplier
			<< ":section:" << text;

		env->SetVar(scriptVariable.c_str(), env->SaveString(ss.str().c_str()));
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
