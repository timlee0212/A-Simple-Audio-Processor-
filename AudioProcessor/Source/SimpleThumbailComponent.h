#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioSource\dRowAudio_AudioFilePlayerExt.h"
class SimpleThumbnailComponent : public Component,
	private ChangeListener
{
public:
	SimpleThumbnailComponent(int sourceSamplesPerThumbnailSample,
		AudioFormatManager& formatManager,
		AudioThumbnailCache &cache,
		AudioFilePlayerExt &transSource)
		:thumbnail(sourceSamplesPerThumbnailSample, formatManager, cache),
		transSource(transSource), displayFullThumbnail(true),
		waveColour(Colours::lightgrey)
	{
		thumbnail.addChangeListener(this);
	}

	void setFile(const File& file)
	{
		thumbnail.setSource(new FileInputSource(file));
	}

	void paint(Graphics &g) override
	{
		paintThumbnail(g);
	}

	void paintThumbnail(Graphics &g)
	{
		if (thumbnail.getNumChannels() == 0)
		{
			g.setColour(Colours::white);
			g.drawFittedText("No File Loaded", getLocalBounds(), Justification::centred, 1.0f);
		}
		else
		{
			//g.setColour(Colours::lightgrey);
			//g.fillRect(getLocalBounds());
			g.setColour(waveColour);
			auto audioLength(thumbnail.getTotalLength());
			if (audioLength > 0.0)
			{
				auto endTime = displayFullThumbnail ?
					audioLength : jmax(30.0, audioLength);

				thumbnail.drawChannels(g, getLocalBounds(), 0.0, endTime, 1.0f);
				g.setColour(Colours::green);
				auto audioPosition(transSource.getCurrentPosition());
				auto drawPosition((audioPosition / audioLength) *  getLocalBounds().getWidth() + getLocalBounds().getX());
				g.drawLine(drawPosition, getLocalBounds().getY(), drawPosition, getLocalBounds().getBottom(), 2.0f);
			}
		}
	}

	void setDisplayFullThumbnail(bool displayFull)
	{
		this->displayFullThumbnail = displayFull;
		repaint();
	}

	AudioThumbnail &getThumbnail() { return thumbnail; }
	void mouseDown(const MouseEvent &event) override
	{
		auto duration = transSource.getLengthInSeconds();
		if (duration > 0.0)
		{
			auto clickPosition = event.position.x;
			auto audioPosition = (clickPosition / getWidth()) * duration;
			transSource.setPosition(audioPosition);
		}
	}

	void setWaveColour(Colour newColour) { waveColour = newColour; }

	void changeListenerCallback(ChangeBroadcaster *source) override
	{
		if (source == &thumbnail)
			repaint();
	}
private:
	AudioThumbnail thumbnail;
	AudioFilePlayerExt &transSource;
	Colour waveColour;

	bool displayFullThumbnail;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleThumbnailComponent)
};