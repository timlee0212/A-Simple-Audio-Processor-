#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "dRowAudio_LoopingAudioSource.h"
#include "dRowAudio_ReversibleAudioSource.h"
#include "dRowAudio_SoundTouchAudioSource.h"

class UltimateAudioSource : public PositionableAudioSource
{
public:
	EffectAudioSource(PositionableAudioSource &inputSource) 
		: loopSource(&inputSource, false),
		touchSource(&loopSource,false),
		reverseSource(&loopSource, false)
	{

	}
	~UltimateAudioSource() {}

	//===================================================
	//Override Functions of Base Class of Positionable Source
	//==============================================================================
	/** @internal. */
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) { reverseSource.prepareToPlay(samplesPerBlockExpected, sampleRate); }

	/** @internal. */
	void releaseResources() { reverseSource.releaseResources(); };

	/** @internal. */
	void getNextAudioBlock(const AudioSourceChannelInfo& info) { reverseSource.getNextAudioBlock(info); };

	//==============================================================================
	/** Implements the PositionableAudioSource method. */
	void setNextReadPosition(int64 newPosition) { touchSource.setNextReadPosition(newPosition); }

	/** Implements the PositionableAudioSource method. */
	int64 getNextReadPosition() const { return touchSource.getNextReadPosition(); }

	/** Implements the PositionableAudioSource method. */
	int64 getTotalLength() const { return touchSource.getTotalLength(); }

	/** Implements the PositionableAudioSource method. */
	bool isLooping() const { return touchSource.isLooping(); }

	/** Implements the PositionableAudioSource method. */
	void setLooping(bool shouldLoop) { touchSource.setLooping(shouldLoop); }
	//===============================================================================


private:
	LoopingAudioSource loopSource;
	ReversibleAudioSource reverseSource;
	SoundTouchAudioSource touchSource;
};