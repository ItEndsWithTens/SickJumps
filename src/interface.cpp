#include "SickJumps.h"

#include <locale>
#include <string>

#include <avisynth.h>



AVSValue __cdecl Create_SickJumps(AVSValue args, void* user_data, IScriptEnvironment* env)
{
  PClip clip = args[0].AsClip();

	VideoInfo vi = clip->GetVideoInfo();

	if (!vi.IsSampleType(SAMPLE_FLOAT))
	{
		env->ThrowError("SickJumps: Input audio type must be float!");
	}

	int firstFrame = args[1].AsInt(0);
	int lastFrame = args[2].AsInt(vi.num_frames - 1);

	SFLOAT startMultiplier = static_cast<SFLOAT>(args[3].AsFloat(1.0f));
	SFLOAT fullMultiplier = static_cast<SFLOAT>(args[4].AsFloat(1.0f));

	double fps = vi.fps_numerator / vi.fps_denominator;
	double seconds = vi.num_frames / fps;

	SFLOAT upSeconds = static_cast<SFLOAT>(args[5].AsFloat(static_cast<SFLOAT>(seconds / 4.0)));
	SFLOAT downSeconds = static_cast<SFLOAT>(args[6].AsFloat(static_cast<SFLOAT>(seconds / 4.0)));

	int mode;
	std::string modeString = env->Invoke("LCase", args[7].AsString("linear")).AsString();
	if (modeString == "spline")
	{
		mode = SickJumps::MODE_SPLINE;
	}
	else
	{
		mode = SickJumps::MODE_LINEAR;
	}

	std::string scriptVariable = args[8].AsString("");

  return new SickJumps(clip, firstFrame, lastFrame, startMultiplier, fullMultiplier, upSeconds, downSeconds, mode, scriptVariable, env);

}



const AVS_Linkage* AVS_linkage = 0;
extern "C" __declspec(dllexport) const char* __stdcall AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors)
{

  AVS_linkage = vectors;

  env->AddFunction("SickJumps", "c[first_frame]i[last_frame]i[start_multiplier]f[full_multiplier]f[up_seconds]f[down_seconds]f[mode]s[script_variable]s", Create_SickJumps, 0);

  return "`SickJumps' - Speed ramping effect";

}
