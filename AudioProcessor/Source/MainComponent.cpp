#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() 
	: state(Stopped), 
	thumbnailCache(40),
	thumbnail(64, formatManager, thumbnailCache, *(audioPlayer.getAudioTransportSource()), &audioPlayer),
	recorder(thumbnail.getThumbnail(), fft),
	mixer(deviceManager),
	tempfile(".wav"),
	fft(deviceManager)
{
	// Make sure you set the size of the component after
	// you add any child components.

	menuBar.reset(new MenuBarComponent(this));
	addAndMakeVisible(menuBar.get());
	commandManager.registerAllCommandsForTarget(this);
	setApplicationCommandManagerToWatch(&commandManager);

	// this lets the command manager use keypresses that arrive in our window to send out commands
	addKeyListener(commandManager.getKeyMappings());

    addChildComponent(menuHeader);
	addAndMakeVisible(sidePanel);
	//===============================================================================================
	loadIcons();

	addAndMakeVisible(&applyButton);
	applyButton.setButtonText("Apply");
	applyButton.onClick = [this] {applyEffects(); };

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

	addAndMakeVisible(&fileNameLabel);
	fileNameLabel.setText("No File Loaded", dontSendNotification);

	addAndMakeVisible(&recordButton);
	recordButton.setImages(false, true, true, icon_record, 1.0f, Colours::transparentBlack,
		icon_record, 0.8f, Colours::transparentBlack, icon_record, 0.5f, Colours::transparentBlack);
	recordButton.onClick = [this] {recordButtonClicked(); };

	addAndMakeVisible(&reverse);
	reverse.setButtonText("Reverse");
	reverse.onClick = [this] {	reverseToggleChanged();}; 

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
	availProcList.addItemList(procName, 1);

	addAndMakeVisible(&currentProcList);

	meter_lnf = new FFAU::LevelMeterLookAndFeel();
	meter_lnf->setColour(FFAU::LevelMeter::lmMeterGradientLowColour, Colours::green);
	meter_lnf->setColour(FFAU::LevelMeter::lmMeterGradientMidColour, Colours::orange);
	meter_lnf->setColour(FFAU::LevelMeter::lmMeterGradientMaxColour, Colours::red);
	meter_lnf->setColour(FFAU::LevelMeter::lmBackgroundColour, Colours::transparentBlack);

	meter = new FFAU::LevelMeter(FFAU::LevelMeter::Horizontal | FFAU::LevelMeter::Minimal);
	meter->setLookAndFeel(meter_lnf);
	meter->setMeterSource(&meterSource);
	addAndMakeVisible(meter);
	addAndMakeVisible(&fft);

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

	startTimer(40);
}

void MainComponent::reverseToggleChanged()
{
	if (!reverse.getToggleState())
		audioPlayer.getTimeSliceThread()->wait(500);
	else
		audioPlayer.getTimeSliceThread()->notify();
	reverseSource->setPlayDirection(!reverse.getToggleState());
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
	case Compressor:
		processorChain.add(new CompressorExpanderAudioProcessor());
		break;
	case Delay:
		processorChain.add(new StereoDelayAudioProcessor());
		break;
	case Flanger:
		processorChain.add(new FlangerAudioProcessor());
		break;
	case Panning:
		processorChain.add(new PanningAudioProcessor());
		break;
	case Phaser:
		processorChain.add(new PhaserAudioProcessor());
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
	meter->setLookAndFeel(nullptr);

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
	if (audioPlayer.isPlaying() && sampleRate > 0.0)
	{
		RelativeTime position(audioPlayer.getAudioFormatReaderSource()->getNextReadPosition() / sampleRate);
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
		case recorded:
			recorder.stop();
			changeState(Stopped);
			//Don't know why but only can release buffer in this way
			deviceManager.closeAudioDevice();
			deviceManager.restartLastAudioDevice();
			//Open the newest Recorded File
			openAudioFile(lastRecording);
			//Set Last Recording to None
			lastRecording = File();
			audioPlayer.stop();
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
	fft.prepareToPlay(samplesPerBlockExpected, sampleRate);

	this->sampleRate = sampleRate;
	for (auto proc : processorChain)
	{
		proc->prepareToPlay(sampleRate, samplesPerBlockExpected);
	}

	if (reverseSource.get() != nullptr)
	{
		reverseSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
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
	
	//Update position when forward and fill the buffer when backward
	if (!reverse.getToggleState())
	{
		audioPlayer.getNextAudioBlock(bufferToFill);
	}
	else
	{
		audioPlayer.getAudioTransportSource()->setNextReadPosition(audioPlayer.getAudioFormatReaderSource()->getNextReadPosition());
	}
	if (reverseSource.get() != nullptr )
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

	if (!isSaving)
	{
		meterSource.measureBlock(*(bufferToFill.buffer));
		fft.getNextAudioBlock(bufferToFill);
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

bool MainComponent::openAudioFile(File &file)
{
	deviceManager.closeAudioDevice();
	if (audioPlayer.setFile(file))
	{
		processorEditorChain.clear(true);
		processorChain.clear(true);
		currentProcList.clear();
		for (int i = 0; i < numControls; i++)
			playerControls[i]->setValue(1.0);	//Set to default value

		if (state != Stopped)
		{
			if (state == Paused || state == recorded)
				changeState(Stopped);
			else
				changeState(Stopping);
		}

		audioPlayer.setLoopBetweenTimes(false);
		reverseSource.reset(new ReversibleAudioSource(audioPlayer.getAudioFormatReaderSource(), false));

		//Swap Channels Only Valid for Stereo Audio
		if (audioPlayer.getAudioFormatReaderSource()->getAudioFormatReader()->numChannels == 2)
			swap.setEnabled(true);
		else
			swap.setEnabled(false);
		
		playButton.setEnabled(true);
		thumbnail.setFile(file);
		currentFile = file;
		fileNameLabel.setText(file.getFileName(), dontSendNotification);
		deviceManager.restartLastAudioDevice();
		return true;
	}
	else
	{
		audioPlayer.setFile(currentFile);
		deviceManager.restartLastAudioDevice();
		return false;
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

	fileNameLabel.setBounds(15, 300, leftPanelWidth - 15, 40);
	applyButton.setBounds(15, getHeight() - 60, 100, 40);
	currentPositionLabel.setBounds(leftPanelWidth + 50, getHeight() - 120 , 200 , 20);

	availProcList.setBounds(15, getHeight() - 160, 150, 30);
	addProcButton.setBounds(180, getHeight() - 160, 30, 30);

	currentProcList.setBounds(15, getHeight() - 120, 150, 30);
	removeProcButton.setBounds(180, getHeight() - 120, 30, 30);
	procSettingButton.setBounds(220, getHeight() - 120, 60, 30);

	fft.setBounds(15, 350, leftPanelWidth - 30, getHeight() - 550);

	Rectangle<int> thumbnailBounds( 303 , 113, getWidth() - 316, getHeight() - 266);
	thumbnail.setBounds(thumbnailBounds);

	meter->setBounds(leftPanelWidth + 50, getHeight() - 40, getWidth() - leftPanelWidth - 70, 30);
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
		//menu.addCommandItem(&commandManager, CommandIDs::settingsColour);
	}
	else if (menuIndex == 2)
	{
		menu.addCommandItem(&commandManager, CommandIDs::effectLinein);
		menu.addCommandItem(&commandManager, CommandIDs::effectLineout);
		//menu.addCommandItem(&commandManager, CommandIDs::effectReverb);
		//menu.addCommandItem(&commandManager, CommandIDs::effectEcho);
	}
	else if (menuIndex == 3)
	{
		menu.addCommandItem(&commandManager, CommandIDs::functionMixer);
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
		//CommandIDs::settingsColour,
		//=======================================
		//Effects Relating
		/*CommandIDs::effectReverb,*/
		CommandIDs::effectLinein,
		CommandIDs::effectLineout,/*
		CommandIDs::effectEcho,
		//=======================================
		//Functions Relating*/
		CommandIDs::functionMixer,
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
		result.setInfo("Setting", "Set Audio Configuration", "Settings", 0);
		//result.setTicked(currentColour == Colours::red);
		result.addDefaultKeypress('r', ModifierKeys::commandModifier);
		break;/*
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
		break;*/
	case CommandIDs::effectLinein:
		result.setInfo("Fade in", "Let Audio Fade in", "Effect", 0);
		//result.setTicked(currentColour == Colours::green);
		result.addDefaultKeypress('i', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::effectLineout:
		result.setInfo("Fade out", "Let Audio Fade out", "Effect", 0);
		//result.setTicked(currentColour == Colours::blue);
		result.addDefaultKeypress('o', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;/*
	case CommandIDs::effectEcho:
		result.setInfo("Echo", "Sets the inner colour to blue", "Effect", 0);
		//result.setTicked(currentColour == Colours::blue);
		result.addDefaultKeypress('e', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;

		//==============================================================
		//Function Relating*/
	case CommandIDs::functionMixer:
		result.setInfo("Mixer", "MultiTrackMixer", "Function", 0);
		//result.setTicked(currentColour == Colours::red);
		result.addDefaultKeypress('f', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::functionUpend:
		result.setInfo("Upend", "Sets the inner colour to green", "Function", 0);
		result.setTicked(reverse.getToggleState());
		result.addDefaultKeypress('u', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
		break;
	case CommandIDs::functionExchange:
		result.setInfo("Exchange", "Sets the inner colour to blue", "Function", 0);
		result.setTicked(swap.getToggleState());
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
		saveAsButtonClicked();
		break;
	case CommandIDs::fileNewrecording:
		recordButtonClicked();
		break;
		//==================================
		//Setting Relating
	case CommandIDs::settingsSetting:
		settingButtonClicked();
		break;/*
	case CommandIDs::settingsColour:
		//currentColour = Colours::green;
		break;
		//====================================
		//Effects Relating
	case CommandIDs::effectReverb:
		//MainComponent::reverbButtonClicked();
		break;*/
	case CommandIDs::effectLinein:
		//currentColour = Colours::green;
		break;
	case CommandIDs::effectLineout:
		//currentColour = Colours::blue;
		break;/*
	case CommandIDs::effectEcho:
		//currentColour = Colours::blue;
		break;
		//========================================
		//Function Relating*/
	case CommandIDs::functionMixer:
		mixerButtonClicked();
		break;
	case CommandIDs::functionUpend:
		reverse.setToggleState(!reverse.getToggleState(),dontSendNotification);
		reverseToggleChanged();
		break;
	case CommandIDs::functionExchange:
		swap.setToggleState(!swap.getToggleState(), dontSendNotification);
		swapped = swap.getToggleState();
		break;
	case CommandIDs::functionRecognize:
		//currentColour = Colours::blue;
		break;
	default:
		return false;
	}

	return true;
}
void MainComponent::saveAsButtonClicked()
{
	currentPositionLabel.setText("Saving...", dontSendNotification);
	FileChooser chooser("Select a the place to save...",
		File::nonexistent, "*.wav;.flac;.aiff;.mp3;");
	if (chooser.browseForFileToSave(true))
	{
		auto file = chooser.getResult();
		file.deleteFile();
		currentPositionLabel.setText("Saving File...", dontSendNotification);
		saveCurrentWave(file);
		deviceManager.restartLastAudioDevice();
	}
}
///Menu Section
///===================================================================================

void MainComponent::openButtonClicked()
{
	if (state == Playing)
		changeState(Pausing);
	else if (state == recording)
		changeState(recorded);
	else
		changeState(Stopped);
	FileChooser chooser("Select a Wave file to play...",
		File::nonexistent,
		"*.wav;*.mp3;*.flac;*.ogg;*.aiff;*.wmv,*.asf,*.wma;");
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
		changeState(recorded);
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
void MainComponent::mixerButtonClicked()
{
	deviceManager.addAudioCallback(&mixer);
	DialogWindow::LaunchOptions(mixerWin);
	mixerWin.content.setNonOwned(&mixer);
	mixerWin.dialogTitle = "MultiTrack Mixing";
	mixerWin.componentToCentreAround = this;
	mixerWin.dialogBackgroundColour = Colours::darkgrey;
	mixerWin.escapeKeyTriggersCloseButton = true;
	mixerWin.useNativeTitleBar = true;
	mixerWin.resizable = true;
	mixerWin.runModal();
	deviceManager.removeAudioCallback(&mixer);
}

void MainComponent::saveButtonClicked()
{
	TemporaryFile temp(currentFile);

	File oldFile = currentFile, tempfile = temp.getFile();
	currentPositionLabel.setText("Saving...", dontSendNotification);
	saveCurrentWave(tempfile, false);

	deviceManager.closeAudioDevice();
	audioPlayer.setFile(File());
	openAudioFile(tempfile);
	if (oldFile.getFileExtension() == ".MP3" || oldFile.getFileExtension() == ".mp3")
	{
		oldFile = File(oldFile.getFileNameWithoutExtension() + ".wav");	//Not yet support writing MP3 file
		tempfile.copyFileTo(oldFile);
	}
	else
	{
		temp.overwriteTargetFileWithTemporary();
	}
	openAudioFile(oldFile);
	currentPositionLabel.setText("Saved", dontSendNotification);

	deviceManager.restartLastAudioDevice();
	
}
void MainComponent::saveCurrentWave(File &file, bool openAfterSaved)
{

	int64 totalSamples = audioPlayer.getTotalLength();

	audioPlayer.stop();

	//Get the information of channels and SampleRate
	AudioDeviceManager::AudioDeviceSetup setup;
	deviceManager.getAudioDeviceSetup(setup);

	std::unique_ptr<AudioFormat> format;
	String extName = file.getFileExtension();
	if (extName.containsIgnoreCase(".mp3"))
		format.reset(new LAMEEncoderAudioFormat(File("LAME.EXE")));
	else if (extName.containsIgnoreCase(".flac"))
		format.reset(new FlacAudioFormat());
	else if (extName.containsIgnoreCase(".aif"))
		format.reset(new AiffAudioFormat());
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

			if (reverseSource.get() != nullptr && reverse.getToggleState())
			{
				audioPlayer.setNextReadPosition(audioPlayer.getTotalLength());
				reverseSource->updatePreviousReadPosition(audioPlayer.getNextReadPosition());
			}
			else
				audioPlayer.setPosition(0.0f);
			deviceManager.closeAudioDevice();

			audioPlayer.prepareToPlay(setup.bufferSize, setup.sampleRate);
			audioPlayer.start();

			isSaving = true;
			double currentSpeed = audioPlayer.getPlaybackSettings().rate * audioPlayer.getPlaybackSettings().tempo;
			for (auto sample = 0; sample < totalSamples / currentSpeed; sample += setup.bufferSize)
			{
				AudioSourceChannelInfo info;
				info.buffer = new AudioSampleBuffer(2, setup.bufferSize);
				info.numSamples = setup.bufferSize;
				info.startSample = 0;
				this->getNextAudioBlock(info);
				std::this_thread::sleep_for(std::chrono::microseconds(10));	//Avoid Dropping Samples
				writer->writeFromAudioSampleBuffer(*info.buffer, 0, setup.bufferSize); 
				info.buffer->clear(); delete info.buffer;		//Avoid Memory Leak
			}
			writer = nullptr;
			isSaving = false;

			if (openAfterSaved)	openAudioFile(file);
			audioPlayer.setPosition(0.0f);
			reverseSource->updatePreviousReadPosition(0);
			deviceManager.restartLastAudioDevice();

			currentPositionLabel.setText("Finished.", dontSendNotification);
			return;
		}
	}
	currentPositionLabel.setText("Failed.", dontSendNotification);

}
void MainComponent::applyEffects()
{
	File temp = tempfile.getFile();
	if (temp.exists())temp.deleteFile();
	currentPositionLabel.setText("Applying Effects...", dontSendNotification);
	saveCurrentWave(temp);
}
