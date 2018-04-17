#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioSource\dRowAudio_AudioFilePlayerExt.h"
class SimpleThumbnailComponent : public Component,
	private ChangeListener,
	public FileDragAndDropTarget,
	public ChangeBroadcaster,
	private ScrollBar::Listener,
	private Timer
{
public:
	SimpleThumbnailComponent(int sourceSamplesPerThumbnailSample,
		AudioFormatManager& formatManager,
		AudioThumbnailCache &cache,
		AudioTransportSource &transSource,
		Slider* slider,
		AudioFilePlayerExt* player)
		:thumbnail(sourceSamplesPerThumbnailSample, formatManager, cache),
		transSource(transSource), displayFullThumbnail(true),
		zoomSlider(slider),
		waveColour(Colours::lightgrey),
		player(player)
	{
		ThumbnailSample = sourceSamplesPerThumbnailSample;
		thumbnail.addChangeListener(this);

		addAndMakeVisible(scrollbar);
		scrollbar.setRangeLimits(visibleRange);
		scrollbar.setAutoHide(false);
		scrollbar.addListener(this);

		currentPositionMarker.setFill(Colours::white.withAlpha(0.85f));
		addAndMakeVisible(currentPositionMarker);
	}
	SimpleThumbnailComponent(int sourceSamplesPerThumbnailSample,
		AudioFormatManager& formatManager,
		AudioThumbnailCache &cache,
		AudioTransportSource &transSource)
		:thumbnail(sourceSamplesPerThumbnailSample, formatManager, cache),
		transSource(transSource), displayFullThumbnail(true),
		waveColour(Colours::lightgrey),
		zoomSlider(nullptr), player(nullptr)
	{
		ThumbnailSample = sourceSamplesPerThumbnailSample;
		thumbnail.addChangeListener(this);

		addAndMakeVisible(scrollbar);
		scrollbar.setRangeLimits(visibleRange);
		scrollbar.setAutoHide(false);
		scrollbar.addListener(this);

		currentPositionMarker.setFill(Colours::white.withAlpha(0.85f));
		addAndMakeVisible(currentPositionMarker);
	}

	void setURL(const URL& url)
	{
		InputSource* inputSource = nullptr;

#if ! JUCE_IOS
		if (url.isLocalFile())
		{
			inputSource = new FileInputSource(url.getLocalFile());
		}
		else
#endif
		{
			if (inputSource == nullptr)
				inputSource = new URLInputSource(url);
		}

		if (inputSource != nullptr)
		{
			thumbnail.setSource(inputSource);

			Range<double> newRange(0.0, thumbnail.getTotalLength());
			scrollbar.setRangeLimits(newRange);
			setRange(newRange);

			startTimerHz(40);
		}
	}

	~SimpleThumbnailComponent()
	{
		scrollbar.removeListener(this);
		thumbnail.removeChangeListener(this);
	}

	URL getLastDroppedFile() const noexcept { return lastFileDropped; }

	void setFile(const File& file)
	{
		thumbnail.setSource(new FileInputSource(file));

		Range<double> newRange(0.0, thumbnail.getTotalLength());
		scrollbar.setRangeLimits(newRange);
		setRange(newRange);

		startTimerHz(40);
	}

	void setZoomFactor(double amount)
	{
		if (thumbnail.getTotalLength() > 0)
		{
			auto newScale = jmax(0.001, thumbnail.getTotalLength() * (1.0 - jlimit(0.0, 0.99, amount)));
			auto timeAtCentre = xToTime(getWidth() / 2.0f);

			setRange({ timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5 });
		}
	}

	void setRange(Range<double> newRange)
	{
		visibleRange = newRange;
		scrollbar.setCurrentRange(visibleRange);
		updateCursorPosition();
		repaint();
	}

	void setFollowsTransport(bool shouldFollow)
	{
		isFollowingTransport = shouldFollow;
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
				auto thumbArea = getLocalBounds();
				if (ThumbnailSample == 512)
				{
					thumbArea.setTop(thumbArea.getY() + 50);
				}
				thumbArea.removeFromBottom(4);
				thumbnail.drawChannels(g, thumbArea.reduced(2),
					visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
				//auto endTime = displayFullThumbnail ?
				//audioLength : jmax(30.0, audioLength);

				//thumbnail.drawChannels(g, getLocalBounds(), 0.0, endTime, 1.0f);
				//g.setColour(Colours::green);
				//auto audioPosition(transSource.getCurrentPosition());
				//auto drawPosition((audioPosition / audioLength) *  getLocalBounds().getWidth() + getLocalBounds().getX());
				//g.drawLine(drawPosition, getLocalBounds().getY(), drawPosition, getLocalBounds().getBottom(), 2.0f);
			}
		}
	}

	void resized() override
	{
		if (ThumbnailSample == 512) {
			scrollbar.setBounds(getLocalBounds().getX(), getLocalBounds().getY() + 5, getLocalBounds().getWidth(), 10);
		}

	}

	bool isInterestedInFileDrag(const StringArray& /*files*/) override
	{
		return true;
	}

	void setDisplayFullThumbnail(bool displayFull)
	{
		this->displayFullThumbnail = displayFull;
		repaint();
	}

	void disableClick() { clickDisabled = true; }

	AudioThumbnail &getThumbnail() { return thumbnail; }
	/*void mouseDown(const MouseEvent &event) override
	{
	if (!clickDisabled)
	{
	auto duration = transSource.getLengthInSeconds();
	if (duration > 0.0)
	{
	auto clickPosition = event.position.x;
	auto audioPosition = (clickPosition / getWidth()) * duration;
	transSource.setPosition(audioPosition);
	}
	}
	}*/

	void filesDropped(const StringArray& files, int /*x*/, int /*y*/) override
	{
		lastFileDropped = URL(File(files[0]));
		sendChangeMessage();
	}


	void mouseDown(const MouseEvent& e) override
	{
		mouseDrag(e);
	}

	void mouseDrag(const MouseEvent& e) override
	{
		if (canMoveTransport())
			transSource.setPosition(jmax(0.0, xToTime((float)e.x)));
	}

	void mouseUp(const MouseEvent&) override
	{
		transSource.start();
	}

	void mouseWheelMove(const MouseEvent&, const MouseWheelDetails& wheel) override
	{
		if (zoomSlider != nullptr) 
		{
			if (thumbnail.getTotalLength() > 0.0)
			{
				auto newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
				newStart = jlimit(0.0, jmax(0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

				if (canMoveTransport())
					setRange({ newStart, newStart + visibleRange.getLength() });

				if (wheel.deltaY != 0.0f)
					zoomSlider->setValue(zoomSlider->getValue() - wheel.deltaY);

				repaint();
			}
		}
	}

	void setWaveColour(Colour newColour) { waveColour = newColour; }

	void changeListenerCallback(ChangeBroadcaster *) override
	{
		//if (source == thumbnail.get())
		repaint();
	}
private:
	AudioThumbnail thumbnail;
	AudioTransportSource &transSource;
	Colour waveColour;
	Slider* zoomSlider;
	ScrollBar scrollbar{ false };
	AudioFilePlayerExt* player;

	AudioThumbnailCache thumbnailCache{ 5 };
	Range<double> visibleRange;
	bool isFollowingTransport = false;
	URL lastFileDropped;

	DrawableRectangle currentPositionMarker;
	int ThumbnailSample;

	bool clickDisabled = false;
	bool displayFullThumbnail;

	float timeToX(const double time) const
	{
		if (visibleRange.getLength() <= 0)
			return 0;

		return getWidth() * (float)((time - visibleRange.getStart()) / visibleRange.getLength());
	}

	double xToTime(const float x) const
	{
		return (x / getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
	}

	bool canMoveTransport() const noexcept
	{
		return !(isFollowingTransport && transSource.isPlaying());
	}

	void scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
	{
		if (scrollBarThatHasMoved == &scrollbar)
			if (!(isFollowingTransport && transSource.isPlaying()))
				setRange(visibleRange.movedToStartAt(newRangeStart));
	}

	void timerCallback() override
	{
		if (canMoveTransport())
			updateCursorPosition();
		else
		{
			double currentPosition;
			if (player->getAudioFormatReaderSource() != nullptr)
				currentPosition = (player->getAudioFormatReaderSource()->getNextReadPosition() / static_cast<double>(transSource.getTotalLength())) * transSource.getLengthInSeconds();
			else
				currentPosition = transSource.getCurrentPosition();
			setRange(visibleRange.movedToStartAt(currentPosition - (visibleRange.getLength() / 2.0)));
		}
	}

	void updateCursorPosition()
	{
		currentPositionMarker.setVisible(transSource.isPlaying() || isMouseButtonDown());

		double currentTime;

		if (player->getAudioFormatReaderSource() != nullptr)
			currentTime = (player->getAudioFormatReaderSource()->getNextReadPosition() / static_cast<double>(transSource.getTotalLength())) * transSource.getLengthInSeconds();
		else
			currentTime = transSource.getCurrentPosition();

		currentPositionMarker.setRectangle(Rectangle<float>(timeToX(currentTime) - 0.75f, 0,
			1.5f, (float)(getHeight() - scrollbar.getHeight())));
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleThumbnailComponent)
};