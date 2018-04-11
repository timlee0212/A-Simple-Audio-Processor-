#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

using namespace dsp;

//==============================================================================
struct DSPParameterBase : public ChangeBroadcaster
{
	DSPParameterBase(const String& labelName) : name(labelName) {}
	virtual ~DSPParameterBase() {}

	virtual Component* getComponent() = 0;

	virtual int getPreferredHeight() = 0;
	virtual int getPreferredWidth() = 0;

	String name;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DSPParameterBase)
};

//==============================================================================
struct SliderParameter : public DSPParameterBase
{
	SliderParameter(Range<double> range, double skew, double initialValue,
		const String& labelName, const String& suffix = {})
		: DSPParameterBase(labelName)
	{
		slider.setRange(range.getStart(), range.getEnd(), 0.01);
		slider.setSkewFactor(skew);
		slider.setValue(initialValue);

		if (suffix.isNotEmpty())
			slider.setTextValueSuffix(suffix);

		slider.onValueChange = [this] { sendChangeMessage(); };
	}

	Component* getComponent() override { return &slider; }

	int getPreferredHeight() override { return 40; }
	int getPreferredWidth()  override { return 500; }

	double getCurrentValue() const { return slider.getValue(); }

private:
	Slider slider;
};

//==============================================================================
struct ChoiceParameter : public DSPParameterBase
{
	ChoiceParameter(const StringArray& options, int initialId, const String& labelName)
		: DSPParameterBase(labelName)
	{
		parameterBox.addItemList(options, 1);
		parameterBox.onChange = [this] { sendChangeMessage(); };

		parameterBox.setSelectedId(initialId);
	}

	Component* getComponent() override { return &parameterBox; }

	int getPreferredHeight() override { return 25; }
	int getPreferredWidth()  override { return 250; }

	int getCurrentSelectedID() const { return parameterBox.getSelectedId(); }

private:
	ComboBox parameterBox;
};


//==============================================================================
class DSPParametersComponent : public Component
{
public:
	DSPParametersComponent(const std::vector<DSPParameterBase*>& Params)
	{
		parameters = Params;

		for (auto demoParameter : parameters)
		{
			addAndMakeVisible(demoParameter->getComponent());

			auto* paramLabel = new Label({}, demoParameter->name);

			paramLabel->attachToComponent(demoParameter->getComponent(), true);
			paramLabel->setJustificationType(Justification::centredLeft);
			addAndMakeVisible(paramLabel);
			labels.add(paramLabel);
		}
	}

	void resized() override
	{
		auto bounds = getLocalBounds();
		bounds.removeFromLeft(30);

		for (auto* p : parameters)
		{
			auto* comp = p->getComponent();

			comp->setSize(jmin(bounds.getWidth(), p->getPreferredWidth()), p->getPreferredHeight());

			auto compBounds = bounds.removeFromTop(p->getPreferredHeight());
			comp->setCentrePosition(compBounds.getCentre());
		}
	}

	int getHeightNeeded()
	{
		auto height = 0;

		for (auto* p : parameters)
			height += p->getPreferredHeight();

		return height + 10;
	}

private:
	std::vector<DSPParameterBase*> parameters;
	OwnedArray<Label> labels;
};

//==============================================================================

template <class DSPType>
struct DSPProcessor : public AudioSource,
	public ProcessorWrapper<DSPType>,
	private ChangeListener
{
	DSPProcessor(AudioSource& input)
		: inputSource(&input)
	{
		for (auto* p : getParameters())
			p->addChangeListener(this);
	}

	void prepareToPlay(int blockSize, double sampleRate) override
	{
		inputSource->prepareToPlay(blockSize, sampleRate);
		this->prepare({ sampleRate, (uint32)blockSize, 2 });
	}

	void releaseResources() override
	{
		inputSource->releaseResources();
	}

	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		jassert(bufferToFill.buffer != nullptr);

		inputSource->getNextAudioBlock(bufferToFill);

		dsp::AudioBlock<float> block(*bufferToFill.buffer,
			(size_t)bufferToFill.startSample);

		ScopedLock audioLock(audioCallbackLock);
		this->process(ProcessContextReplacing<float>(block));
	}


	const std::vector<DSPParameterBase*>& getParameters()
	{
		return this->processor.parameters;
	}

	void changeListenerCallback(ChangeBroadcaster*) override
	{
		ScopedLock audioLock(audioCallbackLock);
		static_cast<DSPType&> (this->processor).updateParameters();
	}



	CriticalSection audioCallbackLock;

	AudioSource* inputSource;
};

//Few Implementation Of DSP Modules
//=========================================
struct FIRFilter
{
	void prepare(const ProcessSpec& spec)
	{
		sampleRate = spec.sampleRate;

		fir.state = FilterDesign<float>::designFIRLowpassWindowMethod(440.0f, sampleRate, 21,
			WindowingFunction<float>::blackman);
		fir.prepare(spec);
	}

	void process(const ProcessContextReplacing<float>& context)
	{
		fir.process(context);
	}

	void reset()
	{
		fir.reset();
	}

	void updateParameters()
	{
		if (sampleRate != 0.0)
		{
			auto cutoff = static_cast<float> (cutoffParam.getCurrentValue());
			auto windowingMethod = static_cast<WindowingFunction<float>::WindowingMethod> (typeParam.getCurrentSelectedID() - 1);

			*fir.state = *FilterDesign<float>::designFIRLowpassWindowMethod(cutoff, sampleRate, 21, windowingMethod);
		}
	}

	//==============================================================================
	ProcessorDuplicator<FIR::Filter<float>, FIR::Coefficients<float>> fir;

	double sampleRate = 0.0;

	SliderParameter cutoffParam{ { 20.0, 20000.0 }, 0.4, 440.0f, "Cutoff", "Hz" };
	ChoiceParameter typeParam{ { "Rectangular", "Triangular", "Hann", "Hamming", "Blackman", "Blackman-Harris", "Flat Top", "Kaiser" },
		5, "Windowing Function" };

	std::vector<DSPParameterBase*> parameters{ &cutoffParam, &typeParam };
};

struct I2RFilter
{
	void prepare(const ProcessSpec& spec)
	{
		sampleRate = spec.sampleRate;

		iir.state = IIR::Coefficients<float>::makeLowPass(sampleRate, 440.0);
		iir.prepare(spec);
	}

	void process(const ProcessContextReplacing<float>& context)
	{
		iir.process(context);
	}

	void reset()
	{
		iir.reset();
	}

	void updateParameters()
	{
		if (sampleRate != 0.0)
		{
			auto cutoff = static_cast<float> (cutoffParam.getCurrentValue());
			auto qVal = static_cast<float> (qParam.getCurrentValue());

			switch (typeParam.getCurrentSelectedID())
			{
			case 1:     *iir.state = *IIR::Coefficients<float>::makeLowPass(sampleRate, cutoff, qVal); break;
			case 2:     *iir.state = *IIR::Coefficients<float>::makeHighPass(sampleRate, cutoff, qVal); break;
			case 3:     *iir.state = *IIR::Coefficients<float>::makeBandPass(sampleRate, cutoff, qVal); break;
			default:    break;
			}
		}
	}

	//==============================================================================
	ProcessorDuplicator<IIR::Filter<float>, IIR::Coefficients<float>> iir;

	ChoiceParameter typeParam{ { "Low-pass", "High-pass", "Band-pass" }, 1, "Type" };
	SliderParameter cutoffParam{ { 20.0, 20000.0 }, 0.5, 440.0f, "Cutoff", "Hz" };
	SliderParameter qParam{ { 0.3, 20.0 }, 0.5, 1.0 / std::sqrt(2.0), "Q" };

	std::vector<DSPParameterBase*> parameters{ &typeParam, &cutoffParam, &qParam };
	double sampleRate = 0.0;
};

struct ReverbDSP
{
	void prepare(const ProcessSpec& spec)
	{
		sampleRate = spec.sampleRate;
		juce::Reverb::Parameters param;
		param.roomSize = static_cast<float>(0.1);
		param.damping = static_cast<float>(0.5);
		param.wetLevel = static_cast<float>(0.8);
		param.dryLevel = static_cast<float>(0.2);
		param.width = static_cast<float>(0.2);
		param.freezeMode = static_cast<float>(0.2);
		rev.setParameters(param);

		rev.prepare(spec);
	}

	void process(const ProcessContextReplacing<float>& context)
	{
		rev.process(context);
	}

	void reset()
	{
		rev.reset();
	}

	void updateParameters()
	{
		if (sampleRate != 0.0)
		{
			juce::Reverb::Parameters param;
			param.roomSize = static_cast<float>(RoomSize.getCurrentValue()/1000.0f);
			param.damping= static_cast<float>(damping.getCurrentValue());
			param.wetLevel = static_cast<float>(wetLevel.getCurrentValue()/100.0f);
			param.dryLevel = static_cast<float>(dryLevel.getCurrentValue()/100.0f);
			param.width = static_cast<float>(width.getCurrentValue()/100.0f);
			param.freezeMode= static_cast<float>(feedback.getCurrentValue());
			rev.setParameters(param);

		}
	}

	//==============================================================================
	dsp::Reverb rev;

	SliderParameter wetLevel{ { 0.0, 100.0 }, 1, 80, "Wet", "%" };
	SliderParameter dryLevel{ { 0.0, 100.0 }, 1, 20, "Dry", "%" };
	SliderParameter RoomSize{ { 0.0, 1000.0 }, 1, 100, "RoomSize", "m2" };
	SliderParameter damping{ { 0.0, 1.0 }, 1, 0.5, "Damp", "" };
	SliderParameter width{ { 0.0, 100.0 }, 1, 20, "Width", "" };
	SliderParameter feedback{ { 0.0, 1.0 }, 1, 0.2, "Feedback", "" };

	std::vector<DSPParameterBase*> parameters{ &wetLevel, &dryLevel, &RoomSize, &damping, &width, &feedback };
	double sampleRate = 0.0;
};