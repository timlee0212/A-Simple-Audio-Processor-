#pragma once

#include "../JuceLibraryCode/JuceHeader.h"


//Class for recording voice from input channel
//And save the recording to file
class Recorder : public AudioIODeviceCallback
{
public:
	Recorder(AudioThumbnail &thumbnail)
		:thumbnail(thumbnail)
	{
		backgroundThread.startThread();
	}

	~Recorder()
	{
		stop();
	}

	void startRecord(const File &file)
	{
		stop();

		if (sampleRate > 0)
		{
			//Delete The File if is Existed
			file.deleteFile();
			ScopedPointer<FileOutputStream> fileStream(file.createOutputStream());

			if (fileStream.get() != nullptr)
			{
				//Create Output Formater
				//TODO:Support for more Format
				WavAudioFormat wavFormat;
				auto *writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, 1, 16, {}, 0);
				if (writer != nullptr)
				{
					fileStream.release();

					//Create FIFO Buffer for data Writing
					threadedWriter.reset(new AudioFormatWriter::ThreadedWriter(writer, backgroundThread, 32678));

					//Reset Thumbnail
					thumbnail.reset(writer->getNumChannels(), writer->getSampleRate());
					nextSampleNum = 0;

					//Swap to active writer pointer
					const ScopedLock sl(writerLock);
					activeWriter = threadedWriter.get();
				}
			}
		}
	}

	void stop()
	{
		//ClearPointer To Stop The Callback from the Writer
		{
			const ScopedLock s1(writerLock);
			activeWriter = nullptr;
		}
		threadedWriter.reset();
	}

	bool isRecording() const
	{
		return activeWriter != nullptr;
	}

	void audioDeviceAboutToStart(AudioIODevice* device) override
	{
		sampleRate = device->getCurrentSampleRate();
	}

	void audioDeviceStopped() override
	{
		sampleRate = 0;
	}

	void audioDeviceIOCallback(const float** inputChannelData, int numInputChannels,
		float** outputChannelData, int numOutputChannels, int numSamples) override
	{
		const ScopedLock sl(writerLock);

		if (activeWriter != nullptr && numInputChannels >= thumbnail.getNumChannels())
		{
			activeWriter->write(inputChannelData, numSamples);

			//Use Audio Buffer to Wrap input data
			AudioBuffer<float> buffer(const_cast<float**> (inputChannelData), thumbnail.getNumChannels(), numSamples);
			thumbnail.addBlock(nextSampleNum, buffer, 0, numSamples);
			nextSampleNum += numSamples;
		}
		//TODO:Monitor Input
		for (int i = 0; i < numOutputChannels; i++)
		{
			if (outputChannelData[i] != nullptr)
				FloatVectorOperations::clear(outputChannelData[i], numSamples);
		}
		
	}

private:
	AudioThumbnail & thumbnail;
	TimeSliceThread backgroundThread{ "Audio Recorder Thread" };
	ScopedPointer<AudioFormatWriter::ThreadedWriter> threadedWriter;
	double sampleRate = 0.0;
	int nextSampleNum = 0;

	CriticalSection writerLock;
	AudioFormatWriter::ThreadedWriter* volatile activeWriter = nullptr;
};