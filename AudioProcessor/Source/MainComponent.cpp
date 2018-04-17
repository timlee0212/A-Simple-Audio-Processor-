#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() 
	: state(Stopped), 
	thumbnailCache(40),
	thumbnail(64, formatManager, thumbnailCache, *(audioPlayer.getAudioTransportSource()),zoomSlider),
	recorder(thumbnail.getThumbnail()),
	mixer(deviceManager)
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
	addAndMakeVisible(sidePanel);
	//===============================================================================================
	loadIcons();
	
	//addAndMakeVisible(&openButton);
	//openButton.setButtonText("Save");
	//openButton.onClick = [this] {openButtonClicked(); };
	//========================================================
	//zoom
	addAndMakeVisible(zoomLabel);//zoomLable
	zoomLabel.setFont(Font(15.00f, Font::plain));
	zoomLabel.setJustificationType(Justification::centredRight);
	zoomLabel.setEditable(false, false, false);
	zoomLabel.setColour(TextEditor::textColourId, Colours::black);
	zoomLabel.setColour(TextEditor::backgroundColourId, Colour(0x00000000));

	addAndMakeVisible(followTransportButton);//你动我不动，你不动我动
	followTransportButton.onClick = [this] { updateFollowTransportState(); };

	addAndMakeVisible(zoomSlider);//zoomSlider
	zoomSlider.setRange(0, 1, 0);
	zoomSlider.onValueChange = [this] { thumbnail2->setZoomFactor(zoomSlider.getValue()); };
	zoomSlider.setSkewFactor(2);

	addAndMakeVisible(&thumbnail);

	thumbnail2.reset(new SimpleThumbnailComponent(512,formatManager, thumbnailCache, *(audioPlayer.getAudioTransportSource()), zoomSlider));
	addAndMakeVisible(thumbnail2.get());
	thumbnail2->addChangeListener(this);


	formatManager.registerBasicFormats();
	thread.startThread(3);

	audioDeviceManager.addAudioCallback(&audioSourcePlayer);
	audioSourcePlayer.setSource(&transportSource);

	setOpaque(true);
	//================================================================

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
		if (!reverse.getToggleState())
			audioPlayer.getTimeSliceThread()->wait(500);
		else
			audioPlayer.getTimeSliceThread()->notify();
		reverseSource->setPlayDirection(!reverse.getToggleState());
	}; 

	addAndMakeVisible(&swap);
	swap.setButtonText("Swap Channels");
	swap.onClick = [this] { swapped = swap.getToggleState(); };

	addAndMakeVisible(&addProcButton);
	addProcButton.setButtonText("+");
	addProcButton.onClick = [this] { addProcButtonClicked();};

	addAndMakeVisible(&removeProcButton);
	removeProcButton.setButtonText("-");
	removeProcButton.onClick = [this] {	removeProcButtonClicked();};

	addAndMakeVisible(&procSettingButton);
	procSettingButton.setButtonText("Setting");
	procSettingButton.onClick = [this] {procSettingButtonClicked(); };

	addAndMakeVisible(&availProcList);
	availProcList.addItemList({ "MoorerReverb", "PingPongDelay","StereoChorus", "Frequalizer"}, 1);

	addAndMakeVisible(&currentProcList);


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

	//audioPlayer.addListener(this);
	//deviceManager.addAudioCallback(&recorder);
	//deviceManager.addAudioCallback(&player);


	setSize(1280, 720);

	// specify the number of input and output channels that we want to open
	setAudioChannels(2, 2);

	startTimer(15);
}

void MainComponent::procSettingButtonClicked()
{
	if (currentProcList.getSelectedId() != 0)
	{
		DialogWindow::LaunchOptions(ProcSetting);
		ProcSetting.content.setNonOwned(processorEditorChain[currentProcList.getSelectedId() - 1]);
		ProcSetting.dialogTitle = "Plugin Setting";
		ProcSetting.componentToCentreAround = this;
		ProcSetting.dialogBackgroundColour = Colours::darkgrey;
		ProcSetting.escapeKeyTriggersCloseButton = true;
		ProcSetting.useNativeTitleBar = true;
		ProcSetting.resizable = false;
		ProcSetting.runModal();
	}
}

void MainComponent::removeProcButtonClicked()
{
	if (currentProcList.getSelectedId() != 0)
	{
		processorEditorChain.remove(currentProcList.getSelectedId() - 1);
		processorChain.remove(currentProcList.getSelectedId() - 1);
		currentProcList.clear();
		for (auto proc : processorChain)
		{
			currentProcList.addItem(proc->getName(), currentProcList.getNumItems() + 1);
		}
	}
}

void MainComponent::addProcButtonClicked()
{
	deviceManager.closeAudioDevice();
	switch (availProcList.getSelectedId() - 1)
	{
	case MoorerReverb:
		processorChain.add(new MoorerReverbAudioProcessor());		
		break;
	case CrossStereoDelay:
		processorChain.add(new CrossStereoDelayAudioProcessor());
		break;
	case StereoChorus:
		processorChain.add(new StereoChorusAudioProcessor());
		break;
	case filter:
		processorChain.add(new FrequalizerAudioProcessor());
		break;
	default:
		return;
	}
	processorEditorChain.add(processorChain.getLast()->createEditor());
	currentProcList.addItem(processorChain.getLast()->getName(), currentProcList.getNumItems() + 1);
	deviceManager.restartLastAudioDevice();
}

MainComponent::~MainComponent()
{
	//MenuBarModel::setMacMainMenu(nullptr);
	deviceManager.removeAudioCallback(&recorder);
	transportSource.setSource(nullptr);
	audioSourcePlayer.setSource(nullptr);
	thumbnail2->removeChangeListener(this);

	audioDeviceManager.removeAudioCallback(&audioSourcePlayer);
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

	for (auto proc : processorChain)
	{
		proc->prepareToPlay(sampleRate, samplesPerBlockExpected);
	}

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
	/*
	if (filterDSP.get() != nullptr && FilterEnable)
	{
		filterDSP->getNextAudioBlock(bufferToFill);
	}
	else if (reverbDSP.get() != nullptr && reverbEnable)
	{
		reverbDSP->getNextAudioBlock(bufferToFill);
	}*/	
	//Update position when forward and fill the buffer when backward
	if (!reverse.getToggleState())
	{
		audioPlayer.getNextAudioBlock(bufferToFill);
	}
	else
	{
		audioPlayer.getAudioTransportSource()->setNextReadPosition(audioPlayer.getAudioFormatReaderSource()->getNextReadPosition());
	}
	if (reverseSource.get() != nullptr && !isSaving)
	{
		reverseSource->getNextAudioBlock(bufferToFill);
	}

	if (swapped && bufferToFill.buffer->getNumChannels()==2)
	{
		for (auto i = 0; i < bufferToFill.buffer->getNumSamples(); i++)
		{
			float temp = bufferToFill.buffer->getSample(0, i);
			bufferToFill.buffer->setSample(0, i, bufferToFill.buffer->getSample(1, i));
			bufferToFill.buffer->setSample(1, i, temp);
		}
	}

	for (auto processor : processorChain)
	{
		if(!bufferToFill.buffer->hasBeenCleared())
			processor->processBlock(*(bufferToFill.buffer), MidiBuffer());
	}


}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
	for (auto proc : processorChain)
		proc->releaseResources();
	audioPlayer.releaseResources();
	if(reverseSource.get()!=nullptr)
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
		thumbnail2->setFile(file);
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

	//=========================================================================
	//ControlBar
	int controlBarsY = getHeight() - 100;
	int controlBarsXcenter = (getWidth() + leftPanelWidth) / 2;
	playButton.setBounds(controlBarsXcenter - 100, controlBarsY, 40, 40);
	stopButton.setBounds(controlBarsXcenter - 40, controlBarsY , 40, 40);
	recordButton.setBounds(controlBarsXcenter + 20, controlBarsY, 40, 40);

	reverse.setBounds(controlBarsXcenter + 100, controlBarsY, 200, 40);
	swap.setBounds(controlBarsXcenter + 180, controlBarsY, 120, 40);

	//Filter Group
	for (int i = 0; i < 3; i++)
	{
		playerControls[i]->setBounds(20 + i * 90, 80, 85, 85);
	}
	for (int i = 3; i < numControls; i++)
	{
		playerControls[i]->setBounds(20 + (i-3) * 90, 200, 85, 85);
	}


	//================================================================================
	auto r = getLocalBounds().reduced(20);

	auto controls = r.removeFromBottom(130);
	auto zoom = controls.removeFromTop(25);
	zoomLabel.setBounds(zoom.removeFromLeft(350));//zoom设置位置
	zoomSlider.setBounds(zoom);

	followTransportButton.setBounds(controlBarsXcenter + 300, controlBarsY, 200, 40);//transport设置位置

	r.removeFromBottom(6);
	thumbnail2->setBounds(300, 80, getWidth() - 310, getHeight() - 230);
	r.removeFromBottom(6);

	//========================================================================
	//openButton.setBounds( 15 , 315, 100, 40);
	//settingButton.setBounds(15, 60, 100, 40);
	//DSPButton.setBounds(15, 120, 100, 40);
	//reverbButton.setBounds(130, 120, 100, 40);
	currentPositionLabel.setBounds(leftPanelWidth + 50, getHeight() - 120 , 200 , 20);

	availProcList.setBounds(15, getHeight() - 240, 150, 30);
	addProcButton.setBounds(180, getHeight() - 240, 30, 30);

	currentProcList.setBounds(15, getHeight() - 200, 150, 30);
	removeProcButton.setBounds(180, getHeight() - 200, 30, 30);
	procSettingButton.setBounds(220, getHeight() - 200, 60, 30);

	Rectangle<int> thumbnailBounds(300, 40, getWidth() - 310, 60);
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
///=============================================================================
///Menu Related Section
///=============================================================================
PopupMenu MainComponent::getMenuForIndex(int menuIndex, const String& /*menuName*/)
{
	PopupMenu menu;

	if (menuIndex == 0)
	{
		menu.addCommandItem(&commandManager, CommandIDs::fileOpen);
		menu.addCommandItem(&commandManager, CommandIDs::fileSave);
		menu.addCommandItem(&commandManager, CommandIDs::fileSaveas);
		menu.addCommandItem(&commandManager, CommandIDs::fileNewrecording);
	}
	else if (menuIndex == 1)
	{
		menu.addCommandItem(&commandManager, CommandIDs::settingsSetting);
		menu.addCommandItem(&commandManager, CommandIDs::settingsColour);
	}
	else if (menuIndex == 2)
	{
		menu.addCommandItem(&commandManager, CommandIDs::effectLinein);
		menu.addCommandItem(&commandManager, CommandIDs::effectLineout);
		menu.addCommandItem(&commandManager, CommandIDs::effectReverb);
		menu.addCommandItem(&commandManager, CommandIDs::effectEcho);
	}
	else if (menuIndex == 3)
	{
		menu.addCommandItem(&commandManager, CommandIDs::functionFilter);
		menu.addCommandItem(&commandManager, CommandIDs::functionUpend);
		menu.addCommandItem(&commandManager, CommandIDs::functionExchange);
		menu.addCommandItem(&commandManager, CommandIDs::functionRecognize);
	}

	return menu;
}

void MainComponent::getAllCommands(Array<CommandID>& c)
{
	Array<CommandID> commands{
		//===========================================
		//File Relating
		CommandIDs::fileOpen,
		CommandIDs::fileSave,
		CommandIDs::fileSaveas,
		CommandIDs::fileNewrecording,
		//============================================
		//Setting Relating
		CommandIDs::settingsSetting,
		CommandIDs::settingsColour,
		//=======================================
		//Effects Relating
		CommandIDs::effectReverb,
		CommandIDs::effectLinein,
		CommandIDs::effectLineout,
		CommandIDs::effectEcho,
		//=======================================
		//Functions Relating
		CommandIDs::functionFilter,
		CommandIDs::functionUpend,
		CommandIDs::functionExchange,
		CommandIDs::functionRecognize
	};
	c.addArray(commands);
}

void MainComponent::getCommandInfo(CommandID commandID, ApplicationCommandInfo& result)
{
	switch (commandID)
	{
		//===================================================
		//File Relating
	case CommandIDs::fileOpen:
		result.setInfo("Open..", "Open an audio file", "Menu", 0);
		//result.setTicked();
		result.addDefaultKeypress('w', ModifierKeys::shiftModifier);
		break;
	case CommandIDs::fileSave:
		result.setInfo("Save", "Save the change", "Menu", 0);
		//result.setTicked();
		result.addDefaultKeypress('g', ModifierKeys::shiftModifier);
		break;
	case CommandIDs::fileSaveas:
		result.setInfo("Save as..", "Save the current file as a new file", "Menu", 0);
		//result.setTicked();
		result.addDefaultKeypress('b', ModifierKeys::shiftModifier);
		break;
	case CommandIDs::fileNewrecording:
		result.setInfo("New recording file", "Build a new file beginning with recording", "Menu", 0);
		//result.setTicked();
		result.addDefaultKeypress('q', ModifierKeys::shiftModifier);
		break;
		//================================================================
		//Setting Relating
	case CommandIDs::settingsSetting:
		result.setInfo("Setting", "Setting what? I don't know", "Settings", 0);
		//result.setTicked(currentColour == Colours::red);
		result.addDefaultKeypress('r', ModifierKeys::commandModifier);
		break;
	case CommandIDs::settingsColour:
		result.setInfo("Colour", "Sets the UI Colour", "Settings", 0);
		//result.setTicked(currentColour == Colours::green);
		result.addDefaultKeypress('g', ModifierKeys::commandModifier);
		break;

		//================================================================
		//Effect Relating
	case CommandIDs::effectReverb:
		result.setInfo("Reverb", "Get a reverb", "Effect", 0);
		//result.setTicked(currentColour == Colours::red);
		result.addDefaultKeypress('r', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::effectLinein:
		result.setInfo("Line in", "Sets the inner colour to green", "Effect", 0);
		//result.setTicked(currentColour == Colours::green);
		result.addDefaultKeypress('i', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::effectLineout:
		result.setInfo("Line out", "Sets the inner colour to blue", "Effect", 0);
		//result.setTicked(currentColour == Colours::blue);
		result.addDefaultKeypress('o', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::effectEcho:
		result.setInfo("Echo", "Sets the inner colour to blue", "Effect", 0);
		//result.setTicked(currentColour == Colours::blue);
		result.addDefaultKeypress('e', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;

		//==============================================================
		//Function Relating
	case CommandIDs::functionFilter:
		result.setInfo("Filter", "Filter the wave", "Function", 0);
		//result.setTicked(currentColour == Colours::red);
		result.addDefaultKeypress('f', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::functionUpend:
		result.setInfo("Upend", "Sets the inner colour to green", "Function", 0);
		//result.setTicked(currentColour == Colours::green);
		result.addDefaultKeypress('u', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::functionExchange:
		result.setInfo("Exchange", "Sets the inner colour to blue", "Function", 0);
		//result.setTicked(currentColour == Colours::blue);
		result.addDefaultKeypress('x', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::functionRecognize:
		result.setInfo("Recognize", "Distinguish between male and female voices", "Inner", 0);
		//result.setTicked(currentColour == Colours::blue);
		result.addDefaultKeypress('b', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;

	default:
		break;
	}
}

bool MainComponent::perform(const InvocationInfo& info)
{
	switch (info.commandID)
	{
		//===========================================
		//File Relating
	case CommandIDs::fileOpen:
		openButtonClicked();
		break;
	case CommandIDs::fileSave:
		saveButtonClicked();
		break;
	case CommandIDs::fileSaveas:
		saveButtonClicked();
		break;
	case CommandIDs::fileNewrecording:
		recordButtonClicked();
		break;
		//==================================
		//Setting Relating
	case CommandIDs::settingsSetting:
		settingButtonClicked();
		break;
	case CommandIDs::settingsColour:
		//currentColour = Colours::green;
		break;
		//====================================
		//Effects Relating
	case CommandIDs::effectReverb:
		//MainComponent::reverbButtonClicked();
		break;
	case CommandIDs::effectLinein:
		//currentColour = Colours::green;
		break;
	case CommandIDs::effectLineout:
		//currentColour = Colours::blue;
		break;
	case CommandIDs::effectEcho:
		//currentColour = Colours::blue;
		break;
		//========================================
		//Function Relating
	case CommandIDs::functionFilter:
		//filterButtonClicked();///
		break;
	case CommandIDs::functionUpend:
		//currentColour = Colours::green;
		break;
	case CommandIDs::functionExchange:
		//currentColour = Colours::blue;
		break;
	case CommandIDs::functionRecognize:
		//currentColour = Colours::blue;
		break;
	default:
		return false;
	}

	return true;
}
///Menu Section
///===================================================================================

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

void MainComponent::mixerButtonClicked()
{
	deviceManager.addAudioCallback(&mixer);
	DialogWindow::LaunchOptions(mixerWin);
	mixerWin.content.setNonOwned(&mixer);
	mixerWin.dialogTitle = "Configure Audio";
	mixerWin.componentToCentreAround = this;
	mixerWin.dialogBackgroundColour = Colours::darkgrey;
	mixerWin.escapeKeyTriggersCloseButton = true;
	mixerWin.useNativeTitleBar = true;
	mixerWin.resizable = true;
	mixerWin.runModal();
	deviceManager.removeAudioCallback(&mixer);
}

void MainComponent::addProcessorButtonClicked()
{

}
void MainComponent::removeProcessorButtonClickde(int index)
{
}
void MainComponent::saveButtonClicked()
{
	FileChooser chooser("Select a the place to save...",
		File::nonexistent,	"*.wav;*.mp3");
	if (chooser.browseForFileToSave(true))
	{
		auto file = chooser.getResult();
		file.deleteFile();
		saveCurrentWave(file);
	}
}
void MainComponent::saveCurrentWave(File &file)
{

	int64 totalSamples = audioPlayer.getTotalLength();

	audioPlayer.stop();

	//Get the information of channels and SampleRate
	AudioDeviceManager::AudioDeviceSetup setup;
	deviceManager.getAudioDeviceSetup(setup);

	std::unique_ptr<AudioFormat> format;
	String extensionName = file.getFileExtension();
	if (extensionName == "MP3" || extensionName == "mp3")
		format.reset(new MP3AudioFormat());
	else
		format.reset(new WavAudioFormat());

	ScopedPointer<OutputStream> outStream(file.createOutputStream());
	if (outStream != nullptr)
	{
		ScopedPointer<AudioFormatWriter> writer(format->createWriterFor(outStream,
			setup.sampleRate, 2, 16, {}, 0));
		if (writer != nullptr)
		{
			outStream.release();
			//deviceManager.closeAudioDevice(); //Avoid Artifacts While Writing
			isSaving = true;

			if (reverseSource.get() != nullptr && reverse.getToggleState())
			{
				audioPlayer.setNextReadPosition(audioPlayer.getTotalLength());
				reverseSource->updatePreviousReadPosition(audioPlayer.getNextReadPosition());
			}
			else
				audioPlayer.setPosition(0.0f);
			audioPlayer.start();
			deviceManager.closeAudioDevice();
			this->prepareToPlay(setup.bufferSize, setup.sampleRate);
			writer->writeFromAudioSource(*this, totalSamples);
			writer = nullptr;

			isSaving = false;
			openAudioFile(file);
			audioPlayer.setPosition(0.0f);
			reverseSource->updatePreviousReadPosition(0);
			deviceManager.restartLastAudioDevice();
		}
	}

}
void MainComponent::applyEffects()
{
	File temp = tempfile.getFile();
	temp.deleteFile();
	saveCurrentWave(temp);
	processorChain.clear(true);
	processorEditorChain.clear(true);
	for (int i = 0; i < numControls; i++)
		playerControls[i]->setValue(1.0);	//Set to default value
}
