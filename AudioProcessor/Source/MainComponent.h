/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <chrono>
#include <thread>

#include "SimpleThumbailComponent.h"
#include "Recorder.h"
#include "DSPLib.h"
#include "AudioSource/dRowAudio_AudioFilePlayerExt.h"
#include "MixerComponent.h"
#include "Plugins\plugins_inlcude.h"
#include "Meters\ff_meters.h"
#include "FFTComponent\SimpleFFTDemo.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
struct BurgerMenuHeader : public Component
{
	BurgerMenuHeader(SidePanel& sp)
		: sidePanel(sp)
	{
		static const unsigned char burgerMenuPathData[]
			= { 110,109,0,0,128,64,0,0,32,65,108,0,0,224,65,0,0,32,65,98,254,212,232,65,0,0,32,65,0,0,240,65,252,
			169,17,65,0,0,240,65,0,0,0,65,98,0,0,240,65,8,172,220,64,254,212,232,65,0,0,192,64,0,0,224,65,0,0,
			192,64,108,0,0,128,64,0,0,192,64,98,16,88,57,64,0,0,192,64,0,0,0,64,8,172,220,64,0,0,0,64,0,0,0,65,
			98,0,0,0,64,252,169,17,65,16,88,57,64,0,0,32,65,0,0,128,64,0,0,32,65,99,109,0,0,224,65,0,0,96,65,108,
			0,0,128,64,0,0,96,65,98,16,88,57,64,0,0,96,65,0,0,0,64,4,86,110,65,0,0,0,64,0,0,128,65,98,0,0,0,64,
			254,212,136,65,16,88,57,64,0,0,144,65,0,0,128,64,0,0,144,65,108,0,0,224,65,0,0,144,65,98,254,212,232,
			65,0,0,144,65,0,0,240,65,254,212,136,65,0,0,240,65,0,0,128,65,98,0,0,240,65,4,86,110,65,254,212,232,
			65,0,0,96,65,0,0,224,65,0,0,96,65,99,109,0,0,224,65,0,0,176,65,108,0,0,128,64,0,0,176,65,98,16,88,57,
			64,0,0,176,65,0,0,0,64,2,43,183,65,0,0,0,64,0,0,192,65,98,0,0,0,64,254,212,200,65,16,88,57,64,0,0,208,
			65,0,0,128,64,0,0,208,65,108,0,0,224,65,0,0,208,65,98,254,212,232,65,0,0,208,65,0,0,240,65,254,212,
			200,65,0,0,240,65,0,0,192,65,98,0,0,240,65,2,43,183,65,254,212,232,65,0,0,176,65,0,0,224,65,0,0,176,
			65,99,101,0,0 };

		Path p;
		p.loadPathFromData(burgerMenuPathData, sizeof(burgerMenuPathData));
		burgerButton.setShape(p, true, true, false);

		burgerButton.onClick = [this] { showOrHide(); };
		addAndMakeVisible(burgerButton);

	}

	~BurgerMenuHeader()
	{
		sidePanel.showOrHide(false);
	}

private:
	void paint(Graphics& g) override
	{
		auto titleBarBackgroundColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId)
			.darker();

		g.setColour(titleBarBackgroundColour);
		g.fillRect(getLocalBounds());
	}

	void resized() override
	{
		auto r = getLocalBounds();

		burgerButton.setBounds(r.removeFromRight(40).withSizeKeepingCentre(20, 20));

		//titleLabel.setFont(Font(getHeight() * 0.5f, Font::plain));
		//titleLabel.setBounds(r);
	}

	void showOrHide()
	{
		sidePanel.showOrHide(!sidePanel.isPanelShowing());
	}

	SidePanel& sidePanel;
	ShapeButton burgerButton{ "burgerButton", Colours::lightgrey, Colours::lightgrey, Colours::white };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BurgerMenuHeader)
};          

class MainComponent : public AudioAppComponent,
	public AudioFilePlayerExt::Listener,
	public Timer,
	public Slider::Listener,
	public ApplicationCommandTarget,
	public MenuBarModel,
	public FileDragAndDropTarget
	//public PositionableAudioSource
{
public:

	//==============================================================================
	MainComponent();

	void reverseToggleChanged();

	~MainComponent();
	//==============================================================================
	enum CommandIDs
	{
		fileOpen = 1,//打开文件
		fileSave,//保存更改
		fileSaveas,//另存为
		fileNewrecording,//新建录音文件
		settingsSetting,//设置对应于原来的setting
		settingsColour,//设置界面颜色
		effectReverb,//效果混音
		effectLinein,//效果渐强
		effectLineout,//效果减弱
		effectEcho,//效果回音
		functionMixer,//功能滤波
		functionUpend,//功能倒放
		functionExchange,//功能交换声道
		functionRecognize//功能辨别男女声
	};

	StringArray getMenuBarNames() override { return { "File", "Settings", "Effect","Function" }; }

	PopupMenu getMenuForIndex(int menuIndex, const String& /*menuName*/) override;

	void menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/) override {}

	ApplicationCommandTarget* getNextCommandTarget() override { return findFirstTargetParentComponent(); }

	void getAllCommands(Array<CommandID>& c) override;

	void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override;

	bool perform(const InvocationInfo& info) override;

	void saveAsButtonClicked();

	/*void setMenuBarPosition(MenuBarPosition newPosition)
	{
		if (menuBarPosition != newPosition)
		{
			menuBarPosition = newPosition;

			if (menuBarPosition != MenuBarPosition::burger)
				sidePanel.showOrHide(false);
			//MenuBarModel::setMacMainMenu(menuBarPosition == MenuBarPosition::global ? this : nullptr);
			menuBar->setVisible(menuBarPosition == MenuBarPosition::window);
			burgerMenu.setModel(menuBarPosition == MenuBarPosition::burger ? this : nullptr);
			menuHeader.setVisible(menuBarPosition == MenuBarPosition::burger);

			sidePanel.setContent(menuBarPosition == MenuBarPosition::burger ? &burgerMenu : nullptr, false);
			menuItemsChanged();

			resized();
		}
	}*/

	//==============================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;
	
	/*
	void setNextReadPosition(int64 newPosition) override { audioPlayer.setNextReadPosition(newPosition); }

	/** Returns the position from which the next block will be returned.*/
	//int64 getNextReadPosition() const override { return audioPlayer.getNextReadPosition(); }

	/** Returns the total length of the stream (in samples). */
	//int64 getTotalLength() const override { return audioPlayer.getTotalLength(); }

	/** Returns true if this source is actually playing in a loop. */
	//bool isLooping() const override { return audioPlayer.isLooping(); }
	
	//==============================================================================
	void paint(Graphics& g) override;
	void resized() override;
	void sliderValueChanged(Slider* slider) override;
	void timerCallback() override;
	void playerStoppedOrStarted(AudioFilePlayer* player) override;
	void fileChanged(AudioFilePlayer* player) override {};

	//=============================================================================
	bool isInterestedInFileDrag(const StringArray& files) override { if (files.size() == 1)return true; else return false; }

	void filesDropped(const StringArray& files, int x, int y) { openAudioFile(File(files[0])); }


private:
    //==============================================================================
    // Your private member variables go here...

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

	enum processorType
	{
		MoorerReverb,
		CrossStereoDelay,
		StereoChorus,
		filter,
		Compressor,
		Delay,
		Flanger,
		Panning,
		Phaser
	};

	const StringArray procName = {
		"MoorerReverb",
		"CrossStereoDelay",
		"StereoChorus",
		"Frequalizer",
		"CompressorExpander",
		"Delay",
		"Flanger",
		"Panning",
		"Phaser"		
	};
	//====================================
	void openButtonClicked();
	void playButtonClicked();
	void stopButtonClicked();
	void procSettingButtonClicked();
	void removeProcButtonClicked();	
	void recordButtonClicked();
	void settingButtonClicked();
	void addProcButtonClicked();

	void saveButtonClicked();
	void saveCurrentWave(File &file, bool openAfterSaved = true);
	void applyEffects();
	void mixerButtonClicked();

	void loadIcons();
	void startRecording();

	bool openAudioFile(File &file);
	void changeState(TransportState newState);

	const int leftPanelWidth = 300;

	//==================================
	ApplicationCommandManager commandManager;

	ScopedPointer<MenuBarComponent> menuBar;
	
//	MenuBarPosition menuBarPosition = MenuBarPosition::window;
	SidePanel sidePanel{ "Menu", 300, false };

	BurgerMenuComponent burgerMenu;
	BurgerMenuHeader menuHeader{ sidePanel };
	//=========================================

	TextButton applyButton, addProcButton, removeProcButton, procSettingButton;
    ImageButton playButton, stopButton, recordButton;
	ToggleButton reverse, swap;
	ComboBox availProcList, currentProcList;

	OwnedArray<Slider> playerControls;
	OwnedArray<Label> playerControlLabels;

	Label currentPositionLabel, fileNameLabel;
	AudioThumbnailCache thumbnailCache;
	SimpleThumbnailComponent thumbnail;
	MixerComponent mixer;

	ScopedPointer<FFAU::LevelMeter> meter;
	ScopedPointer<FFAU::LevelMeterLookAndFeel> meter_lnf;
	FFAU::LevelMeterSource meterSource;
	SimpleFFTDemo fft;
	//==========================================
    AudioFormatManager formatManager;
	AudioFilePlayerExt audioPlayer;
	ScopedPointer<ReversibleAudioSource> reverseSource;

	OwnedArray<AudioProcessor> processorChain;
	OwnedArray<AudioProcessorEditor> processorEditorChain;

	AudioProcessorPlayer player;

    TransportState state;

    Recorder recorder;
	File lastRecording;		
	File currentFile;		//Save Current File Information When file change

	TemporaryFile tempfile; //To save the processed wave

	bool swapped = false;
	bool isSaving = false;

	//Icon Resource
	Image icon_play;
	Image icon_pause;
	Image icon_record;
	Image icon_resume;
	Image icon_stop;

	double sampleRate = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};