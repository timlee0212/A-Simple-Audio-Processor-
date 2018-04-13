#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "dRowAudio_LoopingAudioSource.h"
#include "dRowAudio_ReversibleAudioSource.h"
#include "dRowAudio_SoundTouchAudioSource.h"

class UltimateAudioSource : public PositionableAudioSource
{
public:
	UltimateAudioSource(PositionableAudioSource &inputSource) 
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

	//===============================================================================
	//Featured Setting Method
	/** Sets all of the settings at once.*/
	void setPlaybackSettings(SoundTouchProcessor::PlaybackSettings newSettings) { touchSource.setPlaybackSettings(newSettings); }

	/** Returns all of the settings.*/
	SoundTouchProcessor::PlaybackSettings getPlaybackSettings() { return touchSource.getPlaybackSettings(); }

	void setPlayDirection(bool shouldPlayForwards) { reverseSource.setPlayDirection(shouldPlayForwards); }

	bool getPlayDirection() { return reverseSource.getPlayDirection(); }

	//==============================================================================
	/** Sets the start and end times of the loop.
	This doesn't actually activate the loop, use setLoopBetweenTimes() to toggle this.
	*/
	void setLoopTimes(double startTime, double endTime) { loopSource.setLoopTimes(startTime, endTime); }

	/** Sets the arguments to the currently set start and end times.
	*/
	void getLoopTimes(double& startTime, double& endTime) { loopSource.getLoopTimes(startTime, endTime); }

	/** Enables the loop point set.
	*/
	void setLoopBetweenTimes(bool shouldLoop) { loopSource.setLoopBetweenTimes(shouldLoop); }

	/** Returns true if the loop is activated.
	*/
	bool getLoopBetweenTimes() { loopSource.getLoopBetweenTimes(); }

private:
	LoopingAudioSource loopSource;
	ReversibleAudioSource reverseSource;
	SoundTouchAudioSource touchSource;
};