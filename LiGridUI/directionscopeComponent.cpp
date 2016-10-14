/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  7 Jul 2009 10:24:00 pm

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

#include "directionscopeComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
directionscopeComponent::directionscopeComponent ()
    : Component (T("directionScope"))
{

    //[UserPreSize]
    //[/UserPreSize]

    setSize (200, 200);

    //[Constructor] You can add your own custom stuff here..
	SampleData=NULL;
	SampleCount=0;
	avalue=bvalue=0.0;
	threshold=0.0;
	needsrepainting=1;
    //[/Constructor]
}

directionscopeComponent::~directionscopeComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void directionscopeComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
	needsrepainting=0;
	g.setColour(Colours::grey);
	g.setOrigin(0,0);
	g.drawLine(0,100,199,100);
	g.drawLine(100,0,100,199);
	g.setOrigin(100,100);
	g.setColour(Colours::blue);
	double maxval=0;
	for (int i=0;i<SampleCount;i++) {
		if (abs(SampleData[i*2])>maxval) maxval=SampleData[i*2];
		if (abs(SampleData[i*2+1])>maxval) maxval=SampleData[i*2+1];
	}
	if (maxval) {
		for (int i=0;i<SampleCount;i++) {
			double p=sqrt(SampleData[i*2]*SampleData[i*2]+SampleData[i*2+1]*SampleData[i*2+1]);
			if (p) {
				p=20.0*log10(p/32768.0);
				if (p>threshold) {
					double x=SampleData[i*2]*98.0/maxval;
					double y=SampleData[i*2+1]*98.0/maxval;
					g.drawEllipse(x,y,2,2,2);
				}
			}
		}
	}
	g.setColour(Colours::red);
	if (SampleCount)
		g.drawLine(-200,-200.0*avalue+bvalue,200,200.0*avalue+bvalue);
    //[/UserPaint]
}

void directionscopeComponent::resized()
{
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void directionscopeComponent::SetData(double level,int nsamples,int *data,double a,double b)
{
	int *newsamples=new int[nsamples*2],*osamples,i;
	for (i=0;i<nsamples*2;i++) {
		newsamples[i]=data[i];
	}
	SampleCount=nsamples;
	osamples=SampleData;
	SampleData=newsamples;
	avalue=a;
	bvalue=b;
	threshold=level;
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

<JUCER_COMPONENT documentType="Component" className="directionscopeComponent"
                 componentName="directionScope" parentClasses="public Component"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330000013" fixedSize="1" initialWidth="200"
                 initialHeight="200">
  <BACKGROUND backgroundColour="ffffffff"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
