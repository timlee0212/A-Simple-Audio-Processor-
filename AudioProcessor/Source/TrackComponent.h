#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class TrackComponent : 
	public Component,
	public PositionableAudioSource,
	public Slider::Listener,
	public Timer
{
public:
	TrackComponent() :
		thumbnailCache(30),
		thumbnail(64, formatManager, thumbnailCache, transportSource)
	{
		addAndMakeVisible(&trackName);

		addAndMakeVisible(&gainLabel);
		gainLabel.setText("Gain", dontSendNotification);

		addAndMakeVisible(&openButton);
		openButton.setButtonText("Open File");
		openButton.onClick = [this] {openButtonClicked(); };

		addAndMakeVisible(&gainSlider);
		gainSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
		gainSlider.setRange(0.0f, 2.0f, 0.01);
		gainSlider.setValue(1.0f, dontSendNotification);
		gainSlider.addListener(this);
		gainSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 40, 20);

		addAndMakeVisible(thumbnail);

		formatManager.registerBasicFormats();

		startTimer(50); //Update Thumbnail

	}

	~TrackComponent()
	{

	}

	void resized() override
	{
		Rectangle<int> localBound = getLocalBounds();
		trackName.setBounds(localBound.getX() + 10, localBound.getY() + 20, localBound.getWidth()*0.25, 20);
		openButton.setBounds(localBound.getX() + 10, localBound.getY() + 50, 80, 20);
		gainLabel.setBounds(localBound.getX() + 10, localBound.getY() + 80, 40, 20);
		gainSlider.setBounds(localBound.getX() + 60, localBound.getY() + 80, 160, 20);

		Rectangle<int> thumbnailBound(getLocalBounds().proportionOfWidth(0.3), 0,
			getLocalBounds().proportionOfWidth(0.7), getLocalBounds().getHeight());
		thumbnail.setBounds(thumbnailBound);

	}

	void paint(Graphics &g) override
	{
		g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));


	}

	//============================================================================
	//Implementation Of AudioSource
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
	{
		transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
	}
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		transportSource.setGain(gain);
		transportSource.getNextAudioBlock(bufferToFill);
	}
	void releaseResources() override
	{
		transportSource.releaseResources();
	}
	//===================================================================================

	//===================================================================================
	//Implementation Of Positionable
	//===================================================================================
	void setNextReadPosition(int64 newPosition) override 
	{ 
		transportSource.setNextReadPosition(newPosition);
	}

	int64 getNextReadPosition() const override 
	{ 
		return transportSource.getNextReadPosition();
	}

	/** Returns the total length of the stream (in samples). */
	int64 getTotalLength() const override
	{
		return transportSource.getTotalLength();
	}

	/** Returns true if this source is actually playing in a loop. */
	bool isLooping() const override 
	{ 
		return transportSource.isLooping();
	}

	/** Tells the source whether you'd like it to play in a loop. */
	void setLooping(bool shouldLoop) override
	{ 
		return transportSource.setLooping(shouldLoop);
	}

	//===============================================================================

	void sliderValueChanged(Slider* slider) override
	{
		gain = slider->getValue();
	}

	void timerCallback() override
	{
		thumbnail.repaint();
	}

	AudioTransportSource* getTransportSource() { return &transportSource; }

private:
	void openButtonClicked()
	{
		FileChooser chooser("Select a Wave file to play...",
			File::nonexistent,
			"*.wav;*.mp3");
		if (chooser.browseForFileToOpen())
		{
			auto file = chooser.getResult();
			auto *reader = formatManager.createReaderFor(file);
			if (reader != nullptr)
			{
				//Release Source When Needed
				std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
				transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
				readerSource.reset(newSource.release());
				thumbnail.setFile(file);
				thumbnail.disableClick();
				trackName.setText(file.getFileName(), dontSendNotification);
			}
		}
		repaint();
	}

	SimpleThumbnailComponent thumbnail;
	AudioThumbnailCache thumbnailCache;
	AudioFormatManager formatManager;
	std::unique_ptr<AudioFormatReaderSource> readerSource;
	AudioTransportSource transportSource;
	double gain = 1.0;

	TextButton openButton;
	Slider gainSlider;
	Label gainLabel, trackName;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackComponent);
};