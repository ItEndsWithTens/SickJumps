#ifndef SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E
#define SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E



#include <vector>

#include <avisynth.h>



class SickJumps : public GenericVideoFilter
{
public:
	enum
	{
		MODE_LINEAR = 0,
		MODE_SPLINE
	};

  SickJumps(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _fullMultiplier, SFLOAT _inSeconds, SFLOAT _outSeconds, int _mode, IScriptEnvironment* env);
  ~SickJumps();

	void SickJumpsOldConstructor(PClip _child, int _firstFrame, int _lastFrame, SFLOAT _startMultiplier, SFLOAT _endMultiplier, SFLOAT _inSeconds, SFLOAT _outSeconds, int _mode, IScriptEnvironment * env);

	void __stdcall GetAudio(void* buf, __int64 start, __int64 count, IScriptEnvironment* env);
	PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env);
	int __stdcall SetCacheHints(int cachehints, int frame_range) { return cachehints == CACHE_GET_MTMODE ? MT_NICE_FILTER : 0; }

	int mode;

	int firstFrame;
	int lastFrame;

	SFLOAT startMultiplier;
	SFLOAT fullMultiplier;

	SFLOAT upSeconds;
	SFLOAT downSeconds;

private:
	int CalculateRampInputFrames(int _firstInputFrame, int _totalOutputFrames, SFLOAT _startMultiplier, SFLOAT _endMultiplier, IScriptEnvironment* env);

	int rampUpFirstOutputFrame;
	int rampUpLastOutputFrame;
	int rampUpFirstInputFrame;
	int rampUpLastInputFrame;
	
	int fullSpeedFirstOutputFrame;
	int fullSpeedLastOutputFrame;
	int fullSpeedFirstInputFrame;
	int fullSpeedLastInputFrame; 
	
	int rampDownFirstOutputFrame;
	int rampDownLastOutputFrame;
	int rampDownFirstInputFrame;
	int rampDownLastInputFrame;

	__int64 rampUpFirstOutputSample;
	__int64 rampUpLastOutputSample;
	__int64 rampUpFirstInputSample;
	__int64 rampUpLastInputSample;

	__int64 fullSpeedFirstOutputSample;
	__int64 fullSpeedLastOutputSample;
	__int64 fullSpeedFirstInputSample;
	__int64 fullSpeedLastInputSample;

	__int64 rampDownFirstOutputSample;
	__int64 rampDownLastOutputSample;
	__int64 rampDownFirstInputSample;
	__int64 rampDownLastInputSample;

	std::vector<std::pair<int, int>> crazyVec;
};



double GetCurrentMultiplier(__int64 n, __int64 first, __int64 last, SFLOAT startMultiplier, SFLOAT fullMultiplier, int mode, IScriptEnvironment * env);
double GetDistance(__int64 current, __int64 first, __int64 last, int mode, IScriptEnvironment * env);
double ScaleToRange(double value, double inMin, double inMax, double outMin, double outMax);



#endif // SICKJUMPS_SRC_SICKJUMPS_H_INCLUDED_E35F2A23AB8EFD3BEF369000E18BDD3E
