/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "SimpleThumbailComponent.h"
#include "Recorder.h"
#include "DSPLib.h"
#include "AudioSource/dRowAudio_AudioFilePlayerExt.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public AudioAppComponent,
	public AudioFilePlayerExt::Listener,
	public Timer,
	public Slider::Listener
{
public:
	//==============================================================================
	MainComponent();
	~MainComponent();

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	//==============================================================================
	void paint(Graphics& g) override;
	void resized() override;
	void sliderValueChanged(Slider* slider) override;
	void timerCallback() override;
	void playerStoppedOrStarted(AudioFilePlayer* player) override;
	void fileChanged(AudioFilePlayer* player) override {}

private:
    //==============================================================================
    // Your private member variables go here...


	void openButtonClicked();
	void playButtonClicked();
	void stopButtonClicked();
	void settingButtonClicked();
	void recordButtonClicked();
	void filterButtonClicked();
	void reverbButtonClicked();

	void loadIcons();

	void startRecording();

	void openAudioFile(File &file);

	const int leftPanelWidth = 300;

	enum TransportState
	{
		Stopped,
		Starting,
		Playing,
		Pausing,
		Paused,
		recording,
		recorded,
		Stopping
	};

	enum PlayerControls
	{
		lowEQ,
		midEQ,
		highEQ,
		rate,
		tempo,
		pitch,
		numControls
	};

	TextButton openButton;
    ImageButton playButton;
    ImageButton stopButton;
	ImageButton recordButton;
	TextButton settingButton;
	TextButton DSPButton;
	TextButton reverbButton;
	ToggleButton reverse;

	ScopedPointer<DSPParametersComponent> parametersComponent;

	ScopedPointer<DSPProcessor<I2RFilter>> filterDSP;
	ScopedPointer<DSPProcessor<ReverbDSP>> reverbDSP;
    AudioFormatManager formatManager;
	AudioFilePlayerExt audioPlayer;

	OwnedArray<Slider> playerControls;
	OwnedArray<Label> playerControlLabels;

    TransportState state;
	Label currentPositionLabel;
	AudioThumbnailCache thumbnailCache;
	SimpleThumbnailComponent thumbnail;

    Recorder recorder;
	File lastRecording;

	AudioProcessorPlayer player;

	bool FilterEnable = false;
	bool reverbEnable = false;

	//Icon Resource
	Image icon_play;
	Image icon_pause;
	Image icon_record;
	Image icon_resume;
	Image icon_stop;

	void changeState(TransportState newState);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
