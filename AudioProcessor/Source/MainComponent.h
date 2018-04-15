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
	public MenuBarModel 
{
public:
	
	//==============================================================================
	MainComponent();
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
		functionFilter,//功能滤波
		functionUpend,//功能倒放
		functionExchange,//功能交换声道
		functionRecognize//功能辨别男女声
	};
	
	StringArray getMenuBarNames() override
	{
		return { "File", "Settings", "Effect","Function" };
	}

	PopupMenu getMenuForIndex(int menuIndex, const String& /*menuName*/) override
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
	void menuItemSelected(int /*menuItemID*/, int /*topLevelMenuIndex*/) override {}

	ApplicationCommandTarget* getNextCommandTarget() override
	{
		return &settingsCommandTarget;
	}
	void getAllCommands(Array<CommandID>& c) override
	{
		Array<CommandID> commands{ CommandIDs::fileOpen,
			CommandIDs::fileSave,
			CommandIDs::fileSaveas,
		    CommandIDs::fileNewrecording};
		c.addArray(commands);
	}

	void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override
	{
		switch (commandID)
		{
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
		default:
			break;
		}
	}

	bool perform(const InvocationInfo& info) override//File相关按键点击
	{
		switch (info.commandID)
		{
		case CommandIDs::fileOpen:
			openButtonClicked();
			break;
		case CommandIDs::fileSave:
			//setMenuBarPosition(MenuBarPosition::global);
			break;
		case CommandIDs::fileSaveas:
			//setMenuBarPosition(MenuBarPosition::burger);
			break;
		case CommandIDs::fileNewrecording:
			recordButtonClicked();
			break;
		default:
			return false;
		}

		return true;
	}
	   
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

	//==============================================================================
	void paint(Graphics& g) override;
	void resized() override;
	void sliderValueChanged(Slider* slider) override;
	void timerCallback() override;
	void playerStoppedOrStarted(AudioFilePlayer* player) override;
	void fileChanged(AudioFilePlayer* player) override {};
	

private:
    //==============================================================================
    // Your private member variables go here...


	void openButtonClicked();
	void playButtonClicked();
	void stopButtonClicked();
	
	void recordButtonClicked();
	void filterButtonClicked();
	void reverbButtonClicked();
	void settingButtonClicked();
	
	

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

	//==================================
	ApplicationCommandManager commandManager;

	ScopedPointer<MenuBarComponent> menuBar;
	
//	MenuBarPosition menuBarPosition = MenuBarPosition::window;
	SidePanel sidePanel{ "Menu", 300, false };

	BurgerMenuComponent burgerMenu;
	BurgerMenuHeader menuHeader{ sidePanel };
	//=========================================

	TextButton openButton, settingButton;
    ImageButton playButton, stopButton, recordButton;
	ToggleButton reverse, swap;

	ScopedPointer<DSPParametersComponent> parametersComponent;

	ScopedPointer<DSPProcessor<I2RFilter>> filterDSP;
	ScopedPointer<DSPProcessor<ReverbDSP>> reverbDSP;
    AudioFormatManager formatManager;
	AudioFilePlayerExt audioPlayer;
	ScopedPointer<ReversibleAudioSource> reverseSource;

	OwnedArray<Slider> playerControls;
	OwnedArray<Label> playerControlLabels;

    TransportState state;
	Label currentPositionLabel;
	AudioThumbnailCache thumbnailCache;
	SimpleThumbnailComponent thumbnail;

    Recorder recorder;
	File lastRecording;
	CriticalSection lock;

	AudioProcessorPlayer player;

	bool FilterEnable = false;
	bool reverbEnable = false;
	bool swapped = false;

	//Icon Resource
	Image icon_play;
	Image icon_pause;
	Image icon_record;
	Image icon_resume;
	Image icon_stop;

	void changeState(TransportState newState);

	class SettingsCommandTarget : public Component,
		public ApplicationCommandTarget
	{
	public:
		SettingsCommandTarget(ApplicationCommandManager& m)
			: commandManager(m),
			effectCommandTarget(commandManager)
		{
			commandManager.registerAllCommandsForTarget(this);

			addAndMakeVisible(effectCommandTarget);
		}

		//==============================================================================
		ApplicationCommandTarget* getNextCommandTarget() override
		{
			return &effectCommandTarget;
		}

		void getAllCommands(Array<CommandID>& c) override
		{
			Array<CommandID> commands{ CommandIDs::settingsSetting,
				CommandIDs::settingsColour };

			c.addArray(commands);
		}

		void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override
		{
			switch (commandID)
			{
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
			default:
				break;
			}
		}

		bool perform(const InvocationInfo& info) override//settings相关按键点击
		{
			switch (info.commandID)
			{
			case CommandIDs::settingsSetting:
				//settingButtonClicked();
				break;
			case CommandIDs::settingsColour:
				//currentColour = Colours::green;
				break;
			default:
				return false;
			}
			return true;
		}

	private:
		//==============================================================================
		/**
		Command messages that aren't handled in the OuterCommandTarget will be passed
		to this class to respond to.
		*/
		class EffectCommandTarget : public Component,
			public ApplicationCommandTarget
		{
		public:
			EffectCommandTarget(ApplicationCommandManager& m)
				: commandManager(m),
				functionCommandTarget(commandManager)
			{
				commandManager.registerAllCommandsForTarget(this);

				addAndMakeVisible(functionCommandTarget);
			}

			//==============================================================================
			ApplicationCommandTarget* getNextCommandTarget() override
			{
				// this will return the next parent component that is an ApplicationCommandTarget
				return &functionCommandTarget;
			}

			void getAllCommands(Array<CommandID>& c) override
			{
				Array<CommandID> commands{ CommandIDs::effectReverb,
					CommandIDs::effectLinein,
					CommandIDs::effectLineout,
				    CommandIDs::effectEcho};

				c.addArray(commands);
			}

			void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override
			{
				switch (commandID)
				{
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
				default:
					break;
				}
			}

			bool perform(const InvocationInfo& info) override//Effect相关按键点击
			{
				switch (info.commandID)
				{
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
				default:
					return false;
				}
				return true;
			}
		private:
			struct FunctionCommandTarget : public Component,
				public ApplicationCommandTarget
			{
				FunctionCommandTarget(ApplicationCommandManager& m)
					: commandManager(m)
				{
					commandManager.registerAllCommandsForTarget(this);
				}

				//==============================================================================
				ApplicationCommandTarget* getNextCommandTarget() override
				{
					// this will return the next parent component that is an ApplicationCommandTarget
					return findFirstTargetParentComponent();
				}

				void getAllCommands(Array<CommandID>& c) override
				{
					Array<CommandID> commands{ CommandIDs::functionFilter,
						CommandIDs::functionUpend,
						CommandIDs::functionExchange,
					    CommandIDs::functionRecognize};

					c.addArray(commands);
				}

				void getCommandInfo(CommandID commandID, ApplicationCommandInfo& result) override
				{
					switch (commandID)
					{
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

				bool perform(const InvocationInfo& info) override//function相关功能点击
				{
					switch (info.commandID)
					{
					case CommandIDs::functionFilter:
						//filterButtonClicked();///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1111
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
				ApplicationCommandManager & commandManager;
			};

			ApplicationCommandManager& commandManager;
			FunctionCommandTarget functionCommandTarget;
		};

		ApplicationCommandManager& commandManager;
		EffectCommandTarget effectCommandTarget;
	};
	

	SettingsCommandTarget settingsCommandTarget{ commandManager };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};