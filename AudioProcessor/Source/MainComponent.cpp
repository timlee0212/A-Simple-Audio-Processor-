/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
  */

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() 
	: state(Stopped), 
	thumbnailCache(50),
	thumbnail(64, formatManager, thumbnailCache, transportSource),
	recorder(thumbnail.getThumbnail())
{
	// Make sure you set the size of the component after
	// you add any child components.

	loadIcons();
	
	addAndMakeVisible(&openButton);
	openButton.setButtonText("Open...");
	openButton.onClick = [this] {openButtonClicked(); };

	addAndMakeVisible(&playButton);
	playButton.setImages(false, true, true, icon_play, 1.0f, Colours::transparentBlack,
		icon_play, 0.8f, Colours::transparentBlack, icon_play, 0.5f, Colours::transparentBlack);
	playButton.onClick = [this] {playButtonClicked(); };
	playButton.setEnabled(false);

	addAndMakeVisible(&stopButton);
	stopButton.onClick = [this] {stopButtonClicked(); };
	stopButton.setImages(false, true, true, icon_stop, 1.0f, Colours::transparentBlack,
		icon_stop, 0.8f, Colours::transparentBlack, icon_stop, 0.5f, Colours::transparentBlack);
	stopButton.setColour(TextButton::buttonColourId, Colours::red);
	stopButton.setEnabled(false);

	addAndMakeVisible(&currentPositionLabel);
	currentPositionLabel.setText("Stopped", dontSendNotification);
	currentPositionLabel.setFont(Font(20, Font::bold));

	addAndMakeVisible(&settingButton);
	settingButton.setButtonText("Setting");
	settingButton.onClick = [this] {settingButtonClicked(); };

	addAndMakeVisible(&recordButton);
	recordButton.setImages(false, true, true, icon_record, 1.0f, Colours::transparentBlack,
		icon_record, 0.8f, Colours::transparentBlack, icon_record, 0.5f, Colours::transparentBlack);
	recordButton.onClick = [this] {recordButtonClicked(); };

	addAndMakeVisible(&DSPButton);
	DSPButton.setButtonText("Filter");
	DSPButton.onClick = [this] {filterButtonClicked(); };

	addAndMakeVisible(&reverbButton);
	reverbButton.setButtonText("Reverb");
	reverbButton.onClick = [this] {/*reverbButtonClicked();*/  };

	addAndMakeVisible(&thumbnail);

	formatManager.registerBasicFormats();

	deviceManager.addAudioCallback(&player);

	transportSource.addChangeListener(this);
	//deviceManager.addAudioCallback(&recorder);

	//Below Code is for Test
	auto editor = gateProc.createEditor();
	DialogWindow::LaunchOptions(NGEditor);
	NGEditor.content.setNonOwned(editor);
	NGEditor.dialogTitle = "Configure Audio";
	NGEditor.componentToCentreAround = this;
	NGEditor.dialogBackgroundColour = Colours::darkgrey;
	NGEditor.escapeKeyTriggersCloseButton = true;
	NGEditor.useNativeTitleBar = true;
	NGEditor.resizable = false;
	NGEditor.launchAsync();
	player.setProcessor(&gateProc);

	setSize(1280, 720);

	// specify the number of input and output channels that we want to open
	setAudioChannels(2, 2);

	startTimer(15);
}

MainComponent::~MainComponent()
{
	deviceManager.removeAudioCallback(&recorder);
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

void MainComponent::loadIcons()
{
	PNGImageFormat loader = PNGImageFormat();
	MemoryInputStream *mis = new MemoryInputStream(BinaryData::icon_play_png, BinaryData::icon_play_pngSize, false);
	icon_play = loader.decodeImage(*mis); delete mis;
	mis = new MemoryInputStream(BinaryData::icon_pause_png, BinaryData::icon_pause_pngSize, false);
	icon_pause = loader.decodeImage(*mis); delete mis;
	mis = new MemoryInputStream(BinaryData::icon_record_png, BinaryData::icon_record_pngSize, false);
	icon_record = loader.decodeImage(*mis); delete mis;
	mis = new MemoryInputStream(BinaryData::icon_resume_png, BinaryData::icon_resume_pngSize, false);
	icon_resume = loader.decodeImage(*mis); delete mis;
	mis = new MemoryInputStream(BinaryData::icon_stop_png, BinaryData::icon_stop_pngSize, false);
	icon_stop = loader.decodeImage(*mis); delete mis;

}

void MainComponent::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == &transportSource)
	{
		if (transportSource.isPlaying())
			changeState(Playing);
		else if (state == Stopping || state == Playing)
			changeState(Stopped);
		else if (state == Pausing)
			changeState(Paused);
	}
}

void MainComponent::timerCallback()
{
	if (transportSource.isPlaying())
	{
		RelativeTime position(transportSource.getCurrentPosition());
		auto minutes = ((int)position.inMinutes()) % 60;
		auto seconds = ((int)position.inSeconds()) % 60;
		auto millis = ((int)position.inMilliseconds()) % 1000;

		auto positionString = String::formatted("%02d:%02d:%03d", minutes, seconds, millis);

		currentPositionLabel.setText(positionString, dontSendNotification);
	}
	else if(state == Stopped)
	{
		currentPositionLabel.setText("Stopped", dontSendNotification);
	}
	repaint();
}

void MainComponent::changeState(TransportState newState)
{
	if (state != newState)
	{
		state = newState;
		switch (state)
		{
		case Stopped:
			playButton.setImages(false, true, true, icon_play, 1.0f, Colours::transparentBlack,
				icon_play, 0.8f, Colours::transparentBlack, icon_play, 0.5f, Colours::transparentBlack);
			stopButton.setImages(false, true, true, icon_stop, 1.0f, Colours::transparentBlack,
				icon_stop, 0.8f, Colours::transparentBlack, icon_stop, 0.5f, Colours::transparentBlack);
			stopButton.setEnabled(false);
			playButton.setEnabled(true);
			recordButton.setEnabled(true);
			transportSource.setPosition(0.0);
			break;
		case Starting:
			playButton.setEnabled(false);
			transportSource.start();
			break;
		case Playing:
			playButton.setImages(false, true, true, icon_pause, 1.0f, Colours::transparentBlack,
				icon_pause, 0.8f, Colours::transparentBlack, icon_pause, 0.5f, Colours::transparentBlack);
			stopButton.setImages(false, true, true, icon_stop, 1.0f, Colours::transparentBlack,
				icon_stop, 0.8f, Colours::transparentBlack, icon_stop, 0.5f, Colours::transparentBlack);
			stopButton.setEnabled(true);
			playButton.setEnabled(true);
			break;
		case recording:
			stopButton.setEnabled(true);
			playButton.setEnabled(false);
			recordButton.setEnabled(false);
			break;
		case Pausing:
			transportSource.stop();
			break;
		case Paused:
			playButton.setImages(false, true, true, icon_pause, 1.0f, Colours::transparentBlack,
				icon_pause, 0.8f, Colours::transparentBlack, icon_pause, 0.5f, Colours::transparentBlack);
			stopButton.setEnabled(true);
		case Stopping:
			transportSource.stop();
			break;
		}
	}
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
	gateProc.prepareToPlay(samplesPerBlockExpected, sampleRate);
	if (filterDSP.get() != nullptr)
	{
		filterDSP->prepareToPlay(samplesPerBlockExpected, sampleRate);
	}
	if (reverbDSP.get() != nullptr)
	{
		reverbDSP->prepareToPlay(samplesPerBlockExpected, sampleRate);
	}
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
	if (readerSource.get() == nullptr)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}
	
	if (filterDSP.get() != nullptr && FilterEnable)
	{
		filterDSP->getNextAudioBlock(bufferToFill);
	}
	else if (reverbDSP.get() != nullptr && reverbEnable)
	{
		reverbDSP->getNextAudioBlock(bufferToFill);
	}
	else
	{
		transportSource.getNextAudioBlock(bufferToFill);
	}
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
	transportSource.releaseResources();
}

void MainComponent::startRecording()
{
	if (!RuntimePermissions::isGranted(RuntimePermissions::writeExternalStorage))
	{
		SafePointer<MainComponent> safeThis(this);

		RuntimePermissions::request(RuntimePermissions::writeExternalStorage,
			[safeThis](bool granted) mutable 
			{
				if (granted)
					safeThis->startRecording();
		});
		return;
	}
	deviceManager.addAudioCallback(&recorder);
	FileChooser chooser("Select the place to save", 
		File::nonexistent,
		"*.wav");
	if (chooser.browseForFileToSave(true))
	{
		transportSource.releaseResources();
		changeState(recording);
		lastRecording = chooser.getResult();
		recorder.startRecord(lastRecording);
	}

}

void MainComponent::openAudioFile(File &file)
{
	auto *reader = formatManager.createReaderFor(file);

	if (reader != nullptr)
	{
		if (state != Stopped)
		{
			if (state == Paused || state == recorded)
				changeState(Stopped);
			else
				changeState(Stopping);
		}
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
		playButton.setEnabled(true);
		thumbnail.setFile(file);
		readerSource.reset(newSource.release());
	}
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

	g.setColour(Colours::grey);
	g.fillRect(Rectangle<float>(0, 0, 300, getHeight()));
    // You can add your drawing code here!
}


void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

	//ControlBar
	int controlBarsY = getHeight() - 100;
	int controlBarsXcenter = (getWidth() + leftPanelWidth) / 2;
	playButton.setBounds(controlBarsXcenter - 100, controlBarsY, 40, 40);
	stopButton.setBounds(controlBarsXcenter - 40, controlBarsY , 40, 40);
	recordButton.setBounds(controlBarsXcenter + 20, controlBarsY, 40, 40);


	openButton.setBounds( 10 , 10, 100, 40);
	settingButton.setBounds(10, 60, 100, 40);
	DSPButton.setBounds(10, 120, 100, 40);
	reverbButton.setBounds(130, 120, 100, 40);
	currentPositionLabel.setBounds(leftPanelWidth + 50, getHeight() - 120 , 200 , 20);

	Rectangle<int> thumbnailBounds(300, 60, getWidth() - leftPanelWidth, getHeight() - 200);

	thumbnail.setBounds(thumbnailBounds);

	if (parametersComponent.get() != nullptr)
	{
		parametersComponent->setBounds(0, getHeight() - 300, leftPanelWidth, 300);
	}
}

void MainComponent::openButtonClicked()
{
	FileChooser chooser("Select a Wave file to play...",
		File::nonexistent,
		"*.wav;*.mp3");
	if (chooser.browseForFileToOpen())
	{
		auto file = chooser.getResult();
		openAudioFile(file);
	}
}

void MainComponent::playButtonClicked()
{
	if (state == Stopped || state == Paused)
		changeState(Starting);
	else if (state == Playing)
		changeState(Pausing);
}

void MainComponent::stopButtonClicked()
{
	if (state == Paused)
		changeState(Stopped);
	else if (state == recording)
	{
		recorder.stop();
		changeState(Stopped);
		transportSource.releaseResources();

		//Don't know why but only can release buffer in this way
		deviceManager.closeAudioDevice();
		deviceManager.restartLastAudioDevice();

		//Open the newest Recorded File
		openAudioFile(lastRecording);

		//Set Last Recording to None
		lastRecording = File();
	}
	else
		changeState(Stopping);
}

void MainComponent::settingButtonClicked()
{
	AudioDeviceSelectorComponent audioConfig(deviceManager,
		0,			//Min Input Channels
		5,			//Max Input Channels
		0,			//Min Output Channels
		5,			//Max Output Channels
		false,		//Show MIDI Inputs
		false,		//Show MIDI Outputs
		false,		//Treat Channels as stereo pairs
		false);		//Hide Advanced Options
	audioConfig.setSize(500, 450);

	DialogWindow::LaunchOptions(configureWin);
	configureWin.content.setNonOwned(&audioConfig);
	configureWin.dialogTitle = "Configure Audio";
	configureWin.componentToCentreAround = this;
	configureWin.dialogBackgroundColour = Colours::darkgrey;
	configureWin.escapeKeyTriggersCloseButton = true;
	configureWin.useNativeTitleBar = true;
	configureWin.resizable = false;
	configureWin.runModal();
}

void MainComponent::recordButtonClicked()
{
	if (state != Stopped)
	{
		if (state == Paused)
			changeState(Stopped);
		else
			changeState(Stopping);
	}
	startRecording();
}

void MainComponent::filterButtonClicked()
{
	reverbEnable = false;

	if (FilterEnable)
	{

		parametersComponent.reset();
		FilterEnable = false;
	}
	else
	{
		FilterEnable = true;
		filterDSP.reset(new DSPProcessor<I2RFilter>(transportSource));

		deviceManager.closeAudioDevice();
		deviceManager.restartLastAudioDevice();

		auto& parameters = filterDSP->getParameters();
		parametersComponent.reset();
		if (parameters.size() > 0)
		{
			parametersComponent.reset(new DSPParametersComponent(parameters));
			addAndMakeVisible(parametersComponent.get());
		}
		resized();
	}

}
void MainComponent::reverbButtonClicked()
{
	FilterEnable = false;
	if (reverbEnable)
	{
		parametersComponent.reset();
		reverbEnable = false;
	}
	else
	{
		reverbEnable = true;
		reverbDSP.reset(new DSPProcessor<ReverbDSP>(transportSource));		
		deviceManager.closeAudioDevice();
		deviceManager.restartLastAudioDevice();

		auto& parameters = reverbDSP->getParameters();
		
		parametersComponent.reset();

		if (parameters.size() > 0)
		{
			parametersComponent.reset(new DSPParametersComponent(parameters));
			addAndMakeVisible(parametersComponent.get());
		}
		resized();
	}

}