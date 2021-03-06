#include "SickJumps.h"

#include <locale>
#include <string>

#include <avisynth.h>



AVSValue __cdecl Create_SickJumps(AVSValue args, void* user_data, IScriptEnvironment* env)
{
	PClip clip = args[0].AsClip();

	VideoInfo vi = clip->GetVideoInfo();

	int firstFrame = args[1].AsInt(0);
	int lastFrame = args[2].AsInt(vi.num_frames - 1);

	SFLOAT startMultiplier = static_cast<SFLOAT>(args[3].AsFloat(1.0f));
	SFLOAT fullMultiplier = static_cast<SFLOAT>(args[4].AsFloat(2.0f));

	double fps = vi.fps_numerator / vi.fps_denominator;
	double seconds = vi.num_frames / fps;

	SFLOAT upSeconds = static_cast<SFLOAT>(args[5].AsFloat(static_cast<SFLOAT>(seconds / 4.0)));
	SFLOAT downSeconds = static_cast<SFLOAT>(args[6].AsFloat(static_cast<SFLOAT>(seconds / 4.0)));

	std::string scriptVariable = args[7].AsString("");

	SFLOAT endMultiplier = static_cast<SFLOAT>(args[8].AsFloat(startMultiplier));

	SickJumps* s;
	try
	{
		s = new SickJumps(clip, firstFrame, lastFrame, startMultiplier, fullMultiplier, endMultiplier, upSeconds, downSeconds, scriptVariable, env);
	}
	catch (std::invalid_argument& e)
	{
		env->ThrowError("SickJumps: %s", e.what());
	}

	return s;
}



const AVS_Linkage* AVS_linkage = 0;
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors)
{

	AVS_linkage = vectors;

	env->AddFunction("SickJumps", "c[first_frame]i[last_frame]i[start_multiplier]f[full_multiplier]f[up_seconds]f[down_seconds]f[script_variable]s[end_multiplier]f", Create_SickJumps, 0);

	return "`SickJumps' - Speed ramping effect";

}
