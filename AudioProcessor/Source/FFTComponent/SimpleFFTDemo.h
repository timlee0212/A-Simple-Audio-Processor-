#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"
#include "PitchYIN.h"
#include "vad.h"

//==============================================================================
class SimpleFFTDemo   : public Component,
						public AudioSource,
                        private Timer
{
public:
    SimpleFFTDemo(AudioDeviceManager& deviceManager) :
			forwardFFT (fftOrder),
			spectrogramImage (Image::RGB, 512, 512, true),
			deviceManager(deviceManager),
			vad(1.5, 3, 10)
    {
        setOpaque (true);

        auto audioDevice = deviceManager.getCurrentAudioDevice();
        auto numInputChannels  = jmax (audioDevice != nullptr ? audioDevice->getActiveInputChannels() .countNumberOfSetBits() : 1, 1);
        auto numOutputChannels = audioDevice != nullptr ? audioDevice->getActiveOutputChannels().countNumberOfSetBits() : 2;

		addAndMakeVisible(&MaxFreq);
		addAndMakeVisible(&useAvgButton);
		useAvgButton.setButtonText("Avg.");
		useAvgButton.onClick = [this] {
			useAvg = useAvgButton.getToggleState();
			avgWinSlider.setEnabled(useAvgButton.getToggleState());
			avgWinLabel.setEnabled(useAvgButton.getToggleState());
		};

		addAndMakeVisible(&avgWinLabel);
		avgWinLabel.setText("AvgWin",dontSendNotification);
		avgWinLabel.setEnabled(false);

		addAndMakeVisible(&avgWinSlider);
		avgWinSlider.setRange(1, 100, 1);
		avgWinSlider.onValueChange = [this] {avgWin = avgWinSlider.getValue(); };
		avgWinSlider.setEnabled(false);

        startTimerHz (60);
    }

    ~SimpleFFTDemo()
    {

    }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        // (nothing to do here)
		this->sampleRate = sampleRate;
		pitchDetector.reset(new PitchYIN(sampleRate, samplesPerBlockExpected));
    }

    void releaseResources() override
    {
        // (nothing to do here)
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (bufferToFill.buffer->getNumChannels() > 0)
        {
            const auto* channelData = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);

            for (auto i = 0; i < bufferToFill.numSamples; ++i)
                pushNextSampleIntoFifo (channelData[i]);

            //bufferToFill.clearActiveBufferRegion();
        }
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
		auto b = getLocalBounds().toFloat();
		b.removeFromBottom(25);
        g.setOpacity (1.0f);

        g.drawImage (spectrogramImage, b);
    }
	void resized() override
	{
		auto b = getLocalBounds();
		MaxFreq.setBounds(10, b.getHeight() - 20, b.getWidth() - 10, 18);
		useAvgButton.setBounds(0, 0, b.getWidth() - 10, 20);
		avgWinLabel.setBounds(55, 0, b.getWidth() - 160, 20);
		avgWinSlider.setBounds(110, 0, b.getWidth() - 120, 20);
	}
    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
			// then render our FFT data..
			pitchDetection();

            drawNextLineOfSpectrogram();	
			nextFFTBlockReady = false;
			repaint();

        }
    }

	

private:

	enum
	{
		fftOrder = 11,
		fftSize = 1 << fftOrder
	};

    dsp::FFT forwardFFT;
    Image spectrogramImage;
	Label MaxFreq, avgWinLabel;
	ToggleButton useAvgButton;
	Slider avgWinSlider;

	AudioDeviceManager& deviceManager;
	double sampleRate = 0.0;

	ScopedPointer<PitchYIN> pitchDetector;
	Vad vad;

    float fifo [fftSize];
	float fftData[2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

	bool useAvg = false;
	int avgWin = 1;

	int64 sumMaxFreq = 0;
	int countMaxFreq = 0;


	void pitchDetection()
	{
		if (vad.IsSpeech(fftData, fftSize))
		{
			auto pitch = pitchDetector->getPitchInHz(fftData);
			double maxFreq;
			if (pitch > 20 && pitch < 1000)
			{
				if (useAvg)
				{
					sumMaxFreq += pitch; 
					countMaxFreq++;
					if (countMaxFreq == avgWin)
					{
						maxFreq = sumMaxFreq / static_cast<double>(countMaxFreq);
						countMaxFreq = sumMaxFreq = 0;
					}
					else
						return;
				}
				else
				{
					maxFreq = pitch;
				}
				String Tip = String::formatted("Max Freq is: %.2f Hz ", maxFreq);
				Tip += (maxFreq >= 200 ? "Female" : "Male");
				MaxFreq.setText(Tip, dontSendNotification);
			}
		}
	}

	void pushNextSampleIntoFifo(float sample) noexcept
	{
		// if the fifo contains enough data, set a flag to say
		// that the next line should now be rendered..
		if (fifoIndex == fftSize)
		{
			if (!nextFFTBlockReady)
			{
				zeromem(fftData, sizeof(fftData));
				memcpy(fftData, fifo, sizeof(fifo));
				nextFFTBlockReady = true;
			}

			fifoIndex = 0;
		}

		fifo[fifoIndex++] = sample;
	}

	void drawNextLineOfSpectrogram()
	{
		auto rightHandEdge = spectrogramImage.getWidth() - 1;
		auto imageHeight = spectrogramImage.getHeight() - 20;

		// first, shuffle our image leftwards by 1 pixel..
		spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);

		forwardFFT.performFrequencyOnlyForwardTransform(fftData);

		// find the range of values produced, so we can scale our rendering to
		// show up the detail clearly
		auto maxLevel = FloatVectorOperations::findMinAndMax(fftData, fftSize / 2);

		for (auto y = 1; y < imageHeight; ++y)
		{
			auto skewedProportionY = 1.0f - std::exp(std::log(y / (float)imageHeight) * 0.2f);
			auto fftDataIndex = jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2));
			auto level = jmap(fftData[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), -0.25f, 0.167f);
			if (level < 0) level = 1.0f + level;
			auto brightness = jmap(fftData[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
			spectrogramImage.setPixelAt(rightHandEdge, y, Colour::fromHSV(level, 1.0f, brightness, 1.0f));
		}
	}


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleFFTDemo)
};
