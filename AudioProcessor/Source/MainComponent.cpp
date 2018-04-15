/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
  */

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() 
	: state(Stopped), 
	thumbnailCache(40),
	thumbnail(64, formatManager, thumbnailCache, audioPlayer),
	recorder(thumbnail.getThumbnail())
{
	// Make sure you set the size of the component after
	// you add any child components.

	menuBar.reset(new MenuBarComponent(this));
	addAndMakeVisible(menuBar.get());
	setApplicationCommandManagerToWatch(&commandManager);
	commandManager.registerAllCommandsForTarget(this);

	// this lets the command manager use keypresses that arrive in our window to send out commands
	addKeyListener(commandManager.getKeyMappings());

    addChildComponent(menuHeader);
	addAndMakeVisible(settingsCommandTarget);
	addAndMakeVisible(sidePanel);
	//===============================================================================================
	loadIcons();
	
	//addAndMakeVisible(&openButton);
	//openButton.setButtonText("Open...");
	//openButton.onClick = [this] {openButtonClicked(); };

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

	addAndMakeVisible(&recordButton);
	recordButton.setImages(false, true, true, icon_record, 1.0f, Colours::transparentBlack,
		icon_record, 0.8f, Colours::transparentBlack, icon_record, 0.5f, Colours::transparentBlack);
	recordButton.onClick = [this] {recordButtonClicked(); };

	addAndMakeVisible(&reverse);
	reverse.setButtonText("Reverse");
	reverse.onClick = [this] {
		//To Avoid Reader Conflict of the TimeSliced Thread
		if (!reverse.getToggleState())
		{
			//audioPlayer.getAudioFormatReaderSource()->setNextReadPosition(audioPlayer.getAudioTransportSource()->getNextReadPosition());
			audioPlayer.getTimeSliceThread()->wait(500);
		}
		else
			audioPlayer.getTimeSliceThread()->notify();
		reverseSource->setPlayDirection(!reverse.getToggleState());

	}; 

	addAndMakeVisible(&swap);
	swap.setButtonText("Swap Channels");
	swap.onClick = [this] { swapped = swap.getToggleState(); };

	addAndMakeVisible(&thumbnail);

	formatManager.registerBasicFormats();


	//Create Controls of Basic Audio Transform
	//==============================================================================
	for (int i = 0; i < numControls; i++)
	{
		playerControls.add(new Slider());
		playerControlLabels.add(new Label());
		addAndMakeVisible(playerControls[i]);
		addAndMakeVisible(playerControlLabels[i]);
		playerControlLabels[i]->setFont(12.0f);
		playerControlLabels[i]->attachToComponent(playerControls[i], false);
		playerControls[i]->addListener(this);
		playerControls[i]->setValue(1.0);
		playerControls[i]->setSliderStyle(Slider::LinearHorizontal);
		playerControls[i]->setTextBoxStyle(Slider::TextBoxBelow, false, 50, 16);

		Justification centreJustification(Justification::centred);
		playerControlLabels[i]->setJustificationType(centreJustification);
		playerControlLabels[i]->setColour(Label::textColourId, Colours::white);
	}

	playerControls[lowEQ]->setRange(0.05, 2, 0.001);
	playerControls[midEQ]->setRange(0.05, 2, 0.001);
	playerControls[highEQ]->setRange(0.05, 2, 0.001);

	playerControls[rate]->setRange(0.5, 1.5, 0.001);
	playerControls[tempo]->setRange(0.5, 1.5, 0.001);
	playerControls[pitch]->setRange(0.5, 1.5, 0.001);

	for (int i = 0; i < numControls; i++)
		playerControls[i]->setSkewFactorFromMidPoint(1.0);

	playerControlLabels[lowEQ]->setText("Low EQ", dontSendNotification);
	playerControlLabels[midEQ]->setText("Mid EQ", dontSendNotification);
	playerControlLabels[highEQ]->setText("High EQ", dontSendNotification);
	playerControlLabels[rate]->setText("Rate", dontSendNotification);
	playerControlLabels[tempo]->setText("Tempo", dontSendNotification);
	playerControlLabels[pitch]->setText("Pitch", dontSendNotification);
	//===========================================================================

	audioPlayer.addListener(this);
	deviceManager.addAudioCallback(&recorder);
	deviceManager.addAudioCallback(&player);

	setSize(1280, 720);

	// specify the number of input and output channels that we want to open
	setAudioChannels(2, 2);

	startTimer(15);
}

MainComponent::~MainComponent()
{
	//MenuBarModel::setMacMainMenu(nullptr);
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

void MainComponent::playerStoppedOrStarted(AudioFilePlayer* player)
{
	if (player->isPlaying())
		changeState(Playing);
	else if (state == Stopping || state == Playing)
		changeState(Stopped);
	else if (state == Pausing)
		changeState(Paused);
}

void MainComponent::timerCallback()
{
	if (audioPlayer.isPlaying())
	{
		RelativeTime position(audioPlayer.getCurrentPosition());
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
			audioPlayer.setPosition(0.0f);
			break;
		case Starting:
			playButton.setEnabled(false);
			audioPlayer.start();
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
			audioPlayer.stop();
			break;
		case Paused:
			playButton.setImages(false, true, true, icon_play, 1.0f, Colours::transparentBlack,
				icon_play, 0.8f, Colours::transparentBlack, icon_play, 0.5f, Colours::transparentBlack);
			stopButton.setEnabled(true);
		case Stopping:
			audioPlayer.stop();
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
	audioPlayer.prepareToPlay(samplesPerBlockExpected, sampleRate);
	if (reverseSource.get() != nullptr)
	{
		reverseSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
	}
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
	
	if (audioPlayer.getAudioFormatReaderSource() == nullptr)
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
		//Update position when forward and fill the buffer when backward
		if (!reverse.getToggleState())
		{
			audioPlayer.getNextAudioBlock(bufferToFill);
		}
		else
		{
			audioPlayer.getAudioTransportSource()->setNextReadPosition(audioPlayer.getAudioFormatReaderSource()->getNextReadPosition());
		}
		if (reverseSource.get() != nullptr)
		{
			reverseSource->getNextAudioBlock(bufferToFill);
		}
	}

	if (swapped && bufferToFill.buffer->getNumChannels()==2)
	{
		ScopedLock sl(lock);
		for (auto i = 0; i < bufferToFill.buffer->getNumSamples(); i++)
		{
			float temp = bufferToFill.buffer->getSample(0, i);
			bufferToFill.buffer->setSample(0, i, bufferToFill.buffer->getSample(1, i));
			bufferToFill.buffer->setSample(1, i, temp);
		}
	}


}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()

	audioPlayer.releaseResources();
	reverseSource->releaseResources();
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
		audioPlayer.releaseResources();
		changeState(recording);
		lastRecording = chooser.getResult();
		recorder.startRecord(lastRecording);
	}

}

void MainComponent::openAudioFile(File &file)
{
	//auto *reader = formatManager.createReaderFor(file);
	ScopedLock sl(lock);
	if (audioPlayer.setFile(file))
	{
		if (state != Stopped)
		{
			if (state == Paused || state == recorded)
				changeState(Stopped);
			else
				changeState(Stopping);
		}
		//audioPlayer.setLooping(false);
		audioPlayer.setLoopBetweenTimes(false);
		reverseSource.reset(new ReversibleAudioSource(audioPlayer.getAudioFormatReaderSource(), false));

		//Swap Channels Only Valid for Stereo Audio
		if (audioPlayer.getAudioFormatReaderSource()->getAudioFormatReader()->numChannels == 2)
			swap.setEnabled(true);
		else
			swap.setEnabled(false);
		/*
		std::unique_ptr<AudioFormatReaderSource> newSource(new AudioFormatReaderSource(reader, true));
		transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
		readerSource.reset(newSource.release());
		*/
		
		playButton.setEnabled(true);
		thumbnail.setFile(file);
	}
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

	g.setColour(Colours::lightcoral);
	g.fillRoundedRectangle(10, 40, 280,getHeight()-50,10);
	//Thumbnail Wave
	g.fillRoundedRectangle(300,40,getWidth()-310,60,10);
	//Main Wave
	g.fillRoundedRectangle(300, 110, getWidth() - 310, getHeight() - 260, 10);
	
    // You can add your drawing code here!
}


void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
	//=========================================================================
	auto b = getBounds();

	//if (menuBarPosition == MenuBarPosition::window)
	//{
		menuBar->setBounds(b.removeFromTop(LookAndFeel::getDefaultLookAndFeel()
			.getDefaultMenuBarHeight()));
	//}
	//else if (menuBarPosition == MenuBarPosition::burger)
	//{
		//menuHeader.setBounds(b.removeFromTop(40));
	//}

	settingsCommandTarget.setBounds(b);
	//=========================================================================
	//ControlBar
	int controlBarsY = getHeight() - 100;
	int controlBarsXcenter = (getWidth() + leftPanelWidth) / 2;
	playButton.setBounds(controlBarsXcenter - 100, controlBarsY, 40, 40);
	stopButton.setBounds(controlBarsXcenter - 40, controlBarsY , 40, 40);
	recordButton.setBounds(controlBarsXcenter + 20, controlBarsY, 40, 40);

	reverse.setBounds(controlBarsXcenter + 100, controlBarsY, 200, 40);
	swap.setBounds(controlBarsXcenter + 200, controlBarsY, 100, 40);

	//Filter Group
	for (int i = 0; i < 3; i++)
	{
		playerControls[i]->setBounds(20 + i * 90, 80, 85, 85);
	}
	for (int i = 3; i < numControls; i++)
	{
		playerControls[i]->setBounds(20 + (i-3) * 90, 200, 85, 85);
	}

	//openButton.setBounds( 15 , 15, 100, 40);
	//settingButton.setBounds(15, 60, 100, 40);
	//DSPButton.setBounds(15, 120, 100, 40);
	//reverbButton.setBounds(130, 120, 100, 40);
	currentPositionLabel.setBounds(leftPanelWidth + 50, getHeight() - 120 , 200 , 20);

	Rectangle<int> thumbnailBounds( 303 , 113, getWidth() - 316, getHeight() - 266);
	thumbnail.setBounds(thumbnailBounds);

	if (parametersComponent.get() != nullptr)
	{
		parametersComponent->setBounds(15, getHeight() - 300, leftPanelWidth-20, 300);
	}
}

void MainComponent::sliderValueChanged(Slider* slider)
{	
	if (slider == playerControls[lowEQ])
	{
		audioPlayer.setFilterGain(FilteringAudioSource::Low, (float)playerControls[lowEQ]->getValue());
	}
	else if (slider == playerControls[midEQ])
	{
		audioPlayer.setFilterGain(FilteringAudioSource::Mid, (float)playerControls[midEQ]->getValue());
	}
	else if (slider == playerControls[highEQ])
	{
		audioPlayer.setFilterGain(FilteringAudioSource::High, (float)playerControls[highEQ]->getValue());
	}
	if (audioPlayer.getSoundTouchAudioSource() != nullptr)
	{
		if (slider == playerControls[rate])
		{
			SoundTouchProcessor::PlaybackSettings settings(audioPlayer.getPlaybackSettings());
			settings.rate = (float)playerControls[rate]->getValue();
			audioPlayer.setPlaybackSettings(settings);
		}
		else if (slider == playerControls[tempo])
		{
			SoundTouchProcessor::PlaybackSettings settings(audioPlayer.getPlaybackSettings());
			settings.tempo = (float)playerControls[tempo]->getValue();
			audioPlayer.setPlaybackSettings(settings);
		}
		else if (slider == playerControls[pitch])
		{
			SoundTouchProcessor::PlaybackSettings settings(audioPlayer.getPlaybackSettings());
			settings.pitch = (float)playerControls[pitch]->getValue();
			audioPlayer.setPlaybackSettings(settings);
		}
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
		audioPlayer.releaseResources();
		reverseSource->releaseResources();

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
		filterDSP.reset(new DSPProcessor<I2RFilter>(audioPlayer));

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
		reverbDSP.reset(new DSPProcessor<ReverbDSP>(audioPlayer));
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
