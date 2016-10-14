/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  7 Jul 2009 10:23:59 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.11

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "scopeComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
scopeComponent::scopeComponent ()
    : Component (T("signalScope"))
{

    //[UserPreSize]
    //[/UserPreSize]

    setSize (200, 200);

    //[Constructor] You can add your own custom stuff here..
	SampleData=NULL;
	SampleCount=0;
	needsrepainting=1;
    //[/Constructor]
}

scopeComponent::~scopeComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void scopeComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
	needsrepainting=0;
	g.setColour(Colours::grey);
	g.drawLine(0,100,199,100);
	if (SampleData && SampleCount) {
		int i;
		g.setColour(Colours::blue);
		g.setPixel(0,SampleData[0]);
		for (i=1;i<SampleCount;i++) {
			g.drawLine((i-1),SampleData[(i-1)*2],i,SampleData[i*2]);
		}
		g.setColour(Colours::red);
		g.setPixel(0,SampleData[1]);
		for (i=1;i<SampleCount;i++)
			g.drawLine((i-1),SampleData[(i-1)*2+1],i,SampleData[i*2+1]);
	}
    //[/UserPaint]
}

void scopeComponent::resized()
{
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void scopeComponent::SetData(int nsamples,int *data)
{
	int *newsamples=new int[nsamples*2],*osamples,i,db;
	for (i=0;i<nsamples*2;i++) {
		if (!data[i])
			db=100;
		else {
			if (data[i]>0)
				db=0-20.0*log10(((double)data[i])/32768.0)+2;
			else {
				db=196+20.0*(log10(((double)abs(data[i]))/32768.0))+2;
			}
		}
		newsamples[i]=db;
	}
	SampleCount=nsamples;
	osamples=SampleData;
	SampleData=newsamples;
	if (osamples)
		delete osamples;
	needsrepainting++;
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="scopeComponent" componentName="signalScope"
                 parentClasses="public Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="200" initialHeight="200">
  <BACKGROUND backgroundColour="ffffffff"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
