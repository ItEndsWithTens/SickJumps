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



template<typename T>
void SickJumps::FillAudioBuffer(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
{
	// C++ arrays' [] operator takes a size_t argument, which means that although
	// count is provided as an __int64, buf could never be indexed with values
	// larger than an unsigned int on x86 systems.
	size_t size = static_cast<size_t>(count);
	size_t bytes = vi.BytesPerAudioSample() * size;

	int sampleSize;
	if (vi.sample_type == SAMPLE_INT24)
	{
		sampleSize = vi.BytesPerAudioSample();
	}
	else
	{
		sampleSize = vi.AudioChannels();
	}

	std::vector<T> outputChunk;

	for (size_t i = 0; i < size; ++i)
	{
		std::vector<T> sample;
		for (int j = 0; j < sampleSize; ++j)
		{
			sample.push_back(0);
		}

		__int64 adjustedSample = core.GetAdjustedSampleNumber(start + i);

		child->GetAudio(sample.data(), adjustedSample, 1, env);

		for (size_t k = 0; k < sample.size(); ++k)
		{
			outputChunk.push_back(sample[k]);
		}
	}

	std::memcpy(buf, outputChunk.data(), bytes);
}



void __stdcall SickJumps::GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env)
{
	if (vi.sample_type == SAMPLE_FLOAT)
	{
		FillAudioBuffer<SFLOAT>(buf, start, count, env);
	}
	else if (vi.sample_type == SAMPLE_INT32)
	{
		FillAudioBuffer<int32_t>(buf, start, count, env);
	}
	else if (vi.sample_type == SAMPLE_INT16)
	{
		FillAudioBuffer<int16_t>(buf, start, count, env);
	}
	else
	{
		FillAudioBuffer<int8_t>(buf, start, count, env);
	}
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

	if (cachehints == CACHE_GETCHILD_ACCESS_COST)
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
	else if (cachehints == CACHE_GETCHILD_AUDIO_MODE)
	{
		hints = CACHE_AUDIO;
	}
	else if (cachehints == CACHE_GETCHILD_COST)
	{
		hints = CACHE_COST_LOW;
	}
	else if (cachehints == CACHE_GETCHILD_THREAD_MODE)
	{
		hints = CACHE_THREAD_SAFE;
	}
	else if (cachehints == CACHE_GET_MTMODE)
	{
		hints = MT_NICE_FILTER;
	}

	return hints;
}
