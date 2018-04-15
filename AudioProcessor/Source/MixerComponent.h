#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "TrackComponent.h"

class MixerComponent : 
	public AudioIODeviceCallback,
	public Component
{
public:
	MixerComponent(	AudioDeviceManager& deviceManager):
		deviceManager(deviceManager)
	{
		player.setSource(&mixerSource);

		addAndMakeVisible(&addButton);
		addButton.setButtonText("+");
		addButton.onClick = [this] {addTrack(); };

		addAndMakeVisible(&removeButton);
		removeButton.setButtonText("-");
		removeButton.onClick = [this] {removeTrack(tracks.indexOf(tracks.getLast())); };

		addAndMakeVisible(&saveButton);
		saveButton.setButtonText("Save");
		saveButton.onClick = [this] {saveMixer(); };
		
		addAndMakeVisible(&playButton);
		playButton.setButtonText("Play");
		playButton.onClick = [this] {
			for (auto track : tracks)
			{
				if(!track->getTransportSource()->isPlaying())
					track->getTransportSource()->start();
				else
					track->getTransportSource()->stop();
			}

			if (playButton.getButtonText() == "Play")
				playButton.setButtonText("Pause");
			else
				playButton.setButtonText("Play");
		};

		addAndMakeVisible(&stopButton);
		stopButton.setButtonText("Stop");
		stopButton.onClick = [this] {
			playButton.setButtonText("Play");
			for (auto track : tracks)
			{
				track->getTransportSource()->stop();
				track->getTransportSource()->setPosition(0.0f);
			}
		};

		setSize(800, 600);
	}
	~MixerComponent()
	{}

	void resized() override
	{
		addButton.setBounds(10, 10, 40, 40);
		removeButton.setBounds(60, 10, 40, 40);
		saveButton.setBounds(110, 10, 80, 40);
		playButton.setBounds(200, 10, 80, 40);
		stopButton.setBounds(290, 10, 80, 40);

		int i = 0;
		for (auto track : tracks)
		{
			track->setBounds(10, 80 + i * 110, getWidth() - 20, 100);
			i++;
		}
	}

	//============================================================================
	//Implementation Of AudioDeviceCallback

	void audioDeviceIOCallback(const float** inputChannelData,
		int numInputChannels,
		float** outputChannelData,
		int numOutputChannels,
		int numSamples) override
	{
		player.audioDeviceIOCallback(inputChannelData, numInputChannels, outputChannelData, numOutputChannels, numSamples);
	}

	void audioDeviceAboutToStart(AudioIODevice* device) override
	{
		player.audioDeviceAboutToStart(device);
	}

	void audioDeviceStopped() override
	{
		player.audioDeviceStopped();
	}


	//===================================================================================

private:
	void addTrack()
	{
		for (auto track : tracks)
		{
			track->getTransportSource()->stop();
			track->getTransportSource()->setPosition(0.0f);
		}
		playButton.setButtonText("Play");

		tracks.add(new TrackComponent());
		mixerSource.addInputSource(tracks.getLast(), false);
		addAndMakeVisible(tracks.getLast());
		resized();
	}

	void removeTrack(int index)
	{
		auto *trackToRemove = tracks.removeAndReturn(index);
		removeChildComponent(trackToRemove);
		mixerSource.removeInputSource(trackToRemove);
		resized();
	}

	void saveMixer()
	{
		//Find the total Number of Samples of the mixer Audio
		int64 totalNumSamples = 0;

		for (auto track : tracks)
		{
			totalNumSamples = jmax(totalNumSamples, track->getTotalLength());
		}

		FileChooser chooser("Select the place to save...",
			File::nonexistent,
			"*.wav");
		if (chooser.browseForFileToSave(true))
		{
			auto file = chooser.getResult();

			AudioDeviceManager::AudioDeviceSetup setup;
			deviceManager.getAudioDeviceSetup(setup);

			ScopedPointer<OutputStream> outStream(file.createOutputStream());
			if (outStream != nullptr)
			{
				WavAudioFormat wav;
				ScopedPointer<AudioFormatWriter> writer(wav.createWriterFor(outStream,
					setup.sampleRate, setup.outputChannels.toInteger(), 16, {} , 0));
				if (writer != nullptr)
				{
					for (auto track : tracks)
					{
						track->getTransportSource()->stop();
						track->getTransportSource()->setPosition(0.0f);
						track->getTransportSource()->start();
					}
					outStream.release();

					deviceManager.closeAudioDevice(); //Avoid Artifacts While Writing
					writer->writeFromAudioSource(mixerSource, totalNumSamples);
					deviceManager.restartLastAudioDevice();
					for (auto track : tracks)
					{
						track->getTransportSource()->setPosition(0.0f);
					}
					writer = nullptr;
				}
			}

		}
	}

	MixerAudioSource mixerSource;
	OwnedArray<TrackComponent> tracks;
	AudioDeviceManager& deviceManager;


	AudioSourcePlayer player;

	TextButton addButton, removeButton, saveButton, playButton, stopButton;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent);
};