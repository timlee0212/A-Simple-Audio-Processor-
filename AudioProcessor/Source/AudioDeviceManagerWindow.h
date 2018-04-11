#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AudioDeviceManagerWindow : public AudioAppComponent,
	public ChangeListener, public Timer
{
public:
	AudioDeviceManagerWindow(AudioDeviceManager&deviceManager) :
		audioSetupComp(deviceManager,
			0,			//Min Input Channels
			5,			//Max Input Channels
			0,			//Min Output Channels
			5,			//Max Output Channels
			false,		//Show MIDI Inputs
			false,		//Show MIDI Outputs
			false,		//Treat Channels as stereo pairs
			false)		//Hide Advanced Options
	{
		//Initialize GUI Components
		addAndMakeVisible(audioSetupComp);
		addAndMakeVisible(diagnosticsBox);

		diagnosticsBox.setMultiLine(true);
		diagnosticsBox.setReturnKeyStartsNewLine(true);
		diagnosticsBox.setReadOnly(true);
		diagnosticsBox.setScrollbarsShown(true);
		diagnosticsBox.setCaretVisible(false);
		diagnosticsBox.setColour(TextEditor::backgroundColourId, Colour(0x32ffffff));
		diagnosticsBox.setColour(TextEditor::outlineColourId, Colour(0x1c000000));
		diagnosticsBox.setColour(TextEditor::shadowColourId, Colour(0x16000000));

		cpuUsageLabel.setText("CPU Usage", dontSendNotification);
		cpuUsageText.setJustificationType(Justification::right);
		addAndMakeVisible(&cpuUsageLabel);
		addAndMakeVisible(&cpuUsageText);

		setSize(760, 360);

		setAudioChannels(2, 2);
		deviceManager.addChangeListener(this);

		startTimer(50);
	}
	~AudioDeviceManagerWindow()
	{
		deviceManager.removeChangeListener(this);
		shutdownAudio();
	}

	void prepareToPlay(int, double) override {}

	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		auto *device = deviceManager.getCurrentAudioDevice();

		auto activeInputChannels = device->getActiveInputChannels();
		auto activeOutputChannles = device->getActiveOutputChannels();
		
		auto maxInputChannels = activeInputChannels.getHighestBit() + 1;
		auto maxOutputChannels = activeOutputChannles.getHighestBit() + 1;

		for (auto channel = 0; channel < maxOutputChannels; channel++)
		{
			if ((!activeInputChannels[channel]) || maxInputChannels == 0)
			{
				bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
			}
			else
			{
				auto actualInputChannel = channel % maxInputChannels;
				if (!activeInputChannels[channel])
				{
					bufferToFill.buffer->clear(channel, bufferToFill.startSample, bufferToFill.numSamples);
				}
				else
				{
					auto *inBuffer = bufferToFill.buffer->getReadPointer(actualInputChannel,
						bufferToFill.startSample);
					auto *outBuffer = bufferToFill.buffer->getWritePointer(channel, 
						bufferToFill.startSample);
					for (auto sample = 0; sample < bufferToFill.numSamples; sample++)
					{
						outBuffer[sample] = inBuffer[sample] * random.nextFloat() * 0.25f;
					}
				}
			}
		}
	}

	void releaseResources() override {}

	void paint(Graphics &g) override
	{
		g.setColour(Colours::grey);
		g.fillRect(getLocalBounds().removeFromRight(proportionOfWidth(0.4f)));
	}

	void resized() override
	{
		auto rect = getLocalBounds();

		audioSetupComp.setBounds(rect.removeFromLeft(proportionOfWidth(0.6f)));
		rect.reduce(10, 10);
		auto topLine(rect.removeFromTop(20));
		cpuUsageLabel.setBounds(topLine.removeFromLeft(topLine.getWidth() / 2));
		cpuUsageText.setBounds(topLine);

		rect.removeFromTop(20);

		diagnosticsBox.setBounds(rect);
	}
private:

	void changeListenerCallback(ChangeBroadcaster*) override
	{
		dumpDeviceInfo();
	}

	static String getListOfActiveBits(const BigInteger&b)
	{
		StringArray bits;
		for (auto i = 0; i <= b.getHighestBit(); i++)
		{
			if (b[i])
				bits.add(String(i));
		}
		return bits.joinIntoString(",");
	}
	void timerCallback() override
	{
		auto cpu = deviceManager.getCpuUsage() * 100;
		cpuUsageText.setText(String(cpu, 6) + "%", dontSendNotification);
	}
	void dumpDeviceInfo()
	{
		logMessage("--------------------------------------");
		logMessage("Current audio device type: " + (deviceManager.getCurrentDeviceTypeObject() != nullptr
			? deviceManager.getCurrentDeviceTypeObject()->getTypeName()
			: "<none>"));

		if (auto* device = deviceManager.getCurrentAudioDevice())
		{
			logMessage("Current audio device: " + device->getName().quoted());
			logMessage("Sample rate: " + String(device->getCurrentSampleRate()) + " Hz");
			logMessage("Block size: " + String(device->getCurrentBufferSizeSamples()) + " samples");
			logMessage("Bit depth: " + String(device->getCurrentBitDepth()));
			logMessage("Input channel names: " + device->getInputChannelNames().joinIntoString(", "));
			logMessage("Active input channels: " + getListOfActiveBits(device->getActiveInputChannels()));
			logMessage("Output channel names: " + device->getOutputChannelNames().joinIntoString(", "));
			logMessage("Active output channels: " + getListOfActiveBits(device->getActiveOutputChannels()));
		}
		else
		{
			logMessage("No audio device open");
		}
	}

	void logMessage(const String &m)
	{
		diagnosticsBox.moveCaretToEnd();
		diagnosticsBox.insertTextAtCaret(m + newLine);
	}
	Random random;
	AudioDeviceSelectorComponent audioSetupComp;
	Label cpuUsageLabel;
	Label cpuUsageText;
	TextEditor diagnosticsBox;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioDeviceManagerWindow);
};