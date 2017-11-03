#include <string>
#include <tuple>



class __declspec(dllexport) SickJumpsCore
{
public:
	SickJumpsCore();
	SickJumpsCore(int _frameCount, int _firstFrame, int _lastFrame, double _fps,
		double _upSeconds, double _downSeconds, double _startMultiplier, double _endMultiplier,
		__int64 _audioSamplesPerFrame);
	~SickJumpsCore();

	__int64 GetAdjustedSampleNumber(__int64 n);
	std::tuple<int, double, std::string> GetAdjustedFrameProperties(int n);

	double upSeconds;
	double downSeconds;

	double startMultiplier;
	double fullMultiplier;

	// The highest multiplier during ramps needs to be the average of the range,
	// or clips will speed up too far; like walking while on a moving sidewalk.
	double averageMultiplier;

	// To avoid unsightly seams, the top of the down ramp is snapped relative to
	// the first ramp's end. This necessitates a distinct, adjusted multiplier for
	// the down ramp to ensure the proper frames are chosen.
	double downAverageMultiplier;

	__int64 audioSamplesPerFrame;

	int originalFrameCount;
	int adjustedFrameCount;

	__int64 originalSampleCount;
	__int64 adjustedSampleCount;

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

	int afterFirstInputFrame;

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

	__int64 afterFirstInputSample;
};



__int64 __declspec(dllexport) CalculateRampInputCount(__int64 first, __int64 total, double startMultiplier, double endMultiplier);
double __declspec(dllexport) GetCurrentMultiplier(__int64 n, __int64 first, __int64 last, double startMultiplier, double fullMultiplier);
double __declspec(dllexport) ScaleToRange(__int64 value, __int64 inMin, __int64 inMax, double outMin, double outMax);
