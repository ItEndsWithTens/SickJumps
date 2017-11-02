#include "SickJumps.h"

#include <cmath>
#include <cstring>

#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>

#include "avisynth.h"



SickJumps::SickJumps(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _fullMultiplier,
	SFLOAT _upSeconds, SFLOAT _downSeconds, std::string _scriptVariable, IScriptEnvironment * env)
	:
	GenericVideoFilter(_child), startMultiplier(_startMultiplier), fullMultiplier(_fullMultiplier),
	upSeconds(_upSeconds), downSeconds(_downSeconds), scriptVariable(_scriptVariable),
	setScriptVariable(_scriptVariable != "" ? true : false)
{
	core = SickJumpsCore(vi.num_frames, _firstFrame, _lastFrame, vi.fps_numerator / vi.fps_denominator,
		_upSeconds, _downSeconds, _startMultiplier, _fullMultiplier, vi.AudioSamplesFromFrames(1));

	vi.num_frames = core.adjustedFrameCount;
	vi.num_audio_samples = core.adjustedSampleCount;
}



SickJumps::~SickJumps()
{
}



void __stdcall SickJumps::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
{
	std::vector<SFLOAT> outputChunk;
	for (__int64 i = 0; i < count * vi.AudioChannels(); ++i)
	{
		outputChunk.push_back(1.0f);
	}

	for (__int64 i = 0; i < count; ++i)
	{
		std::vector<SFLOAT> sample;
		for (int j = 0; j < vi.AudioChannels(); ++j)
		{
			sample.push_back(1.0f);
		}

		__int64 adjustedSample = core.GetAdjustedSampleNumber(start + i);

		child->GetAudio(sample.data(), static_cast<__int64>(adjustedSample), 1, env);

		memcpy(outputChunk.data() + (i * vi.AudioChannels()), sample.data(), vi.BytesFromAudioSamples(1));
	}

	__int64 bytesPerSample = vi.BytesFromAudioSamples(1);
	memcpy(reinterpret_cast<SFLOAT*>(buf), outputChunk.data(), count * bytesPerSample);
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
