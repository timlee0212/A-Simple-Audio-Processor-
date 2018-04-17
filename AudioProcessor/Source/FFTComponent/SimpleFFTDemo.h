#pragma once

#include "../../JuceLibraryCode/JuceHeader.h"

//==============================================================================
class SimpleFFTDemo   : public Component,
						public AudioSource,
                        private Timer
{
public:
    SimpleFFTDemo(AudioDeviceManager& deviceManager) :
			forwardFFT (fftOrder),
			spectrogramImage (Image::RGB, 512, 512, true),
			deviceManager(deviceManager)
    {
        setOpaque (true);

        auto audioDevice = deviceManager.getCurrentAudioDevice();
        auto numInputChannels  = jmax (audioDevice != nullptr ? audioDevice->getActiveInputChannels() .countNumberOfSetBits() : 1, 1);
        auto numOutputChannels = audioDevice != nullptr ? audioDevice->getActiveOutputChannels().countNumberOfSetBits() : 2;

		addAndMakeVisible(MaxFreq);

        startTimerHz (60);
    }

    ~SimpleFFTDemo()
    {

    }

    //==============================================================================
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        // (nothing to do here)
		this->sampleRate = sampleRate;
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

        g.setOpacity (1.0f);
        g.drawImage (spectrogramImage, getLocalBounds().toFloat());
    }
	void resized() override
	{
		auto b = getLocalBounds();
		MaxFreq.setBounds(10, b.getHeight() - 18, b.getWidth() - 10, 18);
	}
    void timerCallback() override
    {
        if (nextFFTBlockReady)
        {
			// then render our FFT data..
			forwardFFT.performFrequencyOnlyForwardTransform(fftData);
            drawNextLineOfSpectrogram();
			auto max = findMaximum(fftData, fftSize);
			double maxFreq = 0.0;
			for (auto i = 0; i < fftSize; i++)
			{
				if (abs(fftData[i] - max) < 0.001)
				{
					//Order Slide Average
					if (i != 0)
					{
						auto newFreq = (sampleRate * i) / fftSize;
						if (newFreq < 100) continue;
						if (maxFreq == 0 ||  slideCount >= slideWin)
							maxFreq = newFreq;
						else
							maxFreq = maxFreq * 0.85 + 0.15 * newFreq;
						slideCount++;
					}
					else
					{
						maxFreq = 0;
						slideCount = 0;
					}
					break;
				}
			}

			nextFFTBlockReady = false;
			String Tip = String::formatted("Max Freq is: %.2f Hz ", maxFreq);
			Tip += (maxFreq >= 360 ? "Female" : "Male");
			MaxFreq.setText(Tip, dontSendNotification);
            repaint();
        }
    }

    void pushNextSampleIntoFifo (float sample) noexcept
    {
        // if the fifo contains enough data, set a flag to say
        // that the next line should now be rendered..
        if (fifoIndex == fftSize)
        {
            if (! nextFFTBlockReady)
            {
                zeromem (fftData, sizeof (fftData));
                memcpy (fftData, fifo, sizeof (fifo));
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
        spectrogramImage.moveImageSection (0, 0, 1, 0, rightHandEdge, imageHeight);

        // find the range of values produced, so we can scale our rendering to
        // show up the detail clearly
        auto maxLevel = FloatVectorOperations::findMinAndMax (fftData, fftSize / 2);

        for (auto y = 1; y < imageHeight; ++y)
        {
            auto skewedProportionY = 1.0f - std::exp (std::log (y / (float) imageHeight) * 0.2f);
            auto fftDataIndex = jlimit (0, fftSize / 2, (int) (skewedProportionY * fftSize / 2));
            auto level = jmap (fftData[fftDataIndex], 0.0f, jmax (maxLevel.getEnd(), 1e-5f), -0.25f, 0.167f);
			if (level < 0) level = 1.0f + level;
			auto brightness = jmap(fftData[fftDataIndex], 0.0f, jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);
            spectrogramImage.setPixelAt (rightHandEdge, y, Colour::fromHSV (level, 1.0f, brightness, 1.0f));
        }
    }

    enum
    {
        fftOrder = 10,
        fftSize  = 1 << fftOrder,
		slideWin = 20
    };

private:
    dsp::FFT forwardFFT;
    Image spectrogramImage;
	Label MaxFreq;

	AudioDeviceManager& deviceManager;
	double sampleRate = 0.0;

    float fifo [fftSize];
	short slideCount = 0;
    float fftData [2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleFFTDemo)
};
