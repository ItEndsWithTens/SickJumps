#ifndef SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E
#define SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E



#include <vector>

#include <string>

#include "avisynth.h"

#include "SickJumpsCore.h"



class SickJumps : public GenericVideoFilter
{
public:
	SickJumps(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _fullMultiplier,
		SFLOAT _endMultiplier, SFLOAT _inSeconds, SFLOAT _outSeconds, std::string _scriptVariable, IScriptEnvironment* env);
	~SickJumps();

	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	int __stdcall SetCacheHints(int cachehints, int frame_range);

	std::string scriptVariable;

private:
	template<typename T>
	void FillAudioBuffer(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);

	bool setScriptVariable;

	SickJumpsCore core;
};



#endif // SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E
